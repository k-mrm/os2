#include <types.h>
#include <compiler.h>
#include <kalloc.h>
#include <panic.h>
#include <mm.h>
#include <sysmem.h>
#include <string.h>
#include <x86/mm.h>

#define KPREFIX		"mm:"

#include <printk.h>

/*
 *  Kernel address space
 */
static Vas kernvas;

static PTE *
pagewalk (Vas *vas, ulong va, bool allocpgt)
{
  PageTable pgt    = vas->pgdir;
  uint      level;
  uint      vlevel = vas->level;
  uint      vll    = vas->lowestlevel;
  Phys      pgtpa;
  PTE       *pte;

  for (level = vlevel; level > vll; level--) {
    pte = &pgt[PIDX (level, va)];
    if (*pte & ppresent ()) {
      pgtpa = PTE_PA (*pte);
      pgt = (PageTable)P2V (pgtpa);
    } else if (allocpgt) {
      pgt = zalloc ();
      if (!pgt)
        return NULL;
      pgtpa = V2P (pgt);
      *pte = nexttablepte (pgtpa);
    }
    else {
      // unmapped
      return NULL;
    }
  }
  return &pgt[PIDX (level, va)];
}

static void
mappages (Vas *vas, ulong va, Phys pa, ulong size, ulong flags, bool remap)
{
  PTE *pte;

  for (ulong p = 0; p < size; p += PAGESIZE, va += PAGESIZE, pa += PAGESIZE) {
    pte = pagewalk (vas, va, true);
    if (!pte)
      panic ("null pte %p", va);
    if (!remap && (*pte & ppresent ()))
      panic ("this entry has been used: va %p", va);

    *pte = pteleaf (pa, flags);
  }
}

static Phys
addrwalk (Vas *vas, ulong va)
{
  PTE *pte = pagewalk (vas, va, false);

  if (pte && (*pte & ppresent ()))
    return PTE_PA(*pte);
  else
    return 0;
}

static Phys
kaddrwalk (ulong va)
{
  return addrwalk (&kernvas, va);
}

void
kmappage (void *va, Phys pa, ulong flags)
{
  mappages (&kernvas, (ulong)va, pa, PAGESIZE, flags, false);
}

static void
kremap (void *va, Phys pa, ulong flags)
{
  mappages (&kernvas, (ulong)va, pa, PAGESIZE, flags, true);
}

void *
iomap (Phys pa, ulong nbytes)
{
  void *va;
  ulong flags = pdevice () | pwritable () | pnocache ();

  // TODO: virtualalloc
  va = alloc ();
  if (!va)
    return NULL;
  kremap (va, pa, flags);
  return va;
}

static void INIT
initkvas (void)
{
  x86initkvas (&kernvas);
  kernvas.user = false;
  if (!kernvas.pgdir)
    panic ("NULL pgdir");
  if (!PAGEALIGNED (kernvas.pgdir))
    panic ("pgdir must be page-aligned");
  memset (kernvas.pgdir, 0, PAGESIZE);
}

void INIT
kernelmap (void)
{
  Phys  pstart, pend;
  void  *va;
  ulong flags;

  initkvas ();

  pstart = memstart ();
  pend = memend ();
  for (Phys addr = pstart; addr < pend; addr += PAGESIZE) {
    va = P2V (addr);
    flags = pnormal ();
    if (IS_KERN_TEXT(va))
      flags |= preadonly () | pexecutable ();
    else if (IS_KERN_RODATA(va))
      flags |= preadonly ();
    else
      flags |= pwritable ();
    // KDBG("make map: %p to %p\n", va, addr);
    kmappage (va, addr, flags);
  }

  switchvas (&kernvas);
  KDBG("Switched to kernel virtual address space\n");
}

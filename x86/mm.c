#include <types.h>
#include <compiler.h>
#include <sysmem.h>
#include <mm.h>
#include <kalloc.h>
#include "arch.h"
#include "mm.h"

/* Kernel Page Directory */
static PTE kpml4[512] ALIGNED (PAGESIZE);

extern PTE __boot_pml4[];
extern PTE __boot_pdpt[];

bool x86nxe;

void
switchvas (Vas *vas)
{
  Phys pgtpa = V2P (vas->pgdir);
  if (vas->user) {
    ;
  }
  asm volatile ("mov %0, %%cr3" :: "r"(pgtpa));
}

void
x86initkvas (Vas *kvas)
{
  kvas->pgdir       = kpml4;
  kvas->level       = 4;
  kvas->lowestlevel = 1;
}

void INIT
x86mminit (void)
{
  u32 efer = rdmsr32 (IA32_EFER);
  x86nxe = !!(efer & IA32_EFER_NXE);
}

void INIT
killbootmap (void)
{
  __boot_pml4[PIDX (4, KERNLINK_PA)] = 0;
}

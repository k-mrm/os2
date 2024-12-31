#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <mm.h>
#include <string.h>
#include "sysmem.h"
#include "x86/mm.h"

#define KPREFIX     "system memory:"

#include <printk.h>

Sysmem sysmem = {
  .avail.name = "Available",
  .avail.nblock = 0,
  .rsrv.name = "Reserved",
  .rsrv.nblock = 0,
};

static void DEBUG sysmemdump (void);

void INIT
reservekernelarea (void)
{
  Phys kstartpa, kendpa;
  ulong ksize;

  kstartpa = V2P (__kstart);
  kendpa   = V2P (__kend);
  ksize = PAGEALIGN (kendpa - kstartpa);
  KDBG ("kernel image @%p-%p\n", kstartpa, kendpa);

  sysrsrvmem (kstartpa, ksize);
}

/*
 * __memblockoverlap
 * true: overlapped
 * false: ∅
 */
static bool
__memblockoverlap (Phys astart, Phys aend, Phys bstart, Phys bend,
                   Phys *pstart, Phys *pend)
{
  Phys s, e;

  s = MAX (astart, bstart);
  e = MIN (aend, bend);

  if (s < e) {
    if (pstart) {
      *pstart = s;
    }
    if (pend) {
      *pend = e;
    }
    return true;
  } else {
    return false;
  }
}

static bool
memblockoverlap (MemBlock *a, MemBlock *b, Phys *pstart, Phys *pend)
{
  return __memblockoverlap (a->base, a->base + a->size, b->base, b->base + b->size,
                            pstart, pend);
}

static bool
memblockmerge (MemBlock *a, MemBlock *b, Phys *pstart, Phys *pend)
{
  Phys s, e;

  if (!memblockoverlap (a, b, NULL, NULL))
    return false;

  s = MIN (a->base, b->base);
  e = MAX (a->base + a->size, b->base + b->size);

  if (pstart)
    *pstart = s;
  if (pend)
    *pend = e;
  return true;
}

/*
 * bootmemfind
 * true: found memory region, start physaddr is @pa
 * false: memory is not found
 */
static bool INIT
bootmemfind (uint nbytes, uint align, Phys *pa)
{
  MemBlock *ab, *rb;
  Phys start, end;      /* [start, end) */
  Phys rstart, rend;    /* [rstart, rend) */
  Phys ms, me;

  FOREACH_SYSMEM_AVAIL_BLOCK (ab) {
    start = ab->base;
    end = ab->base + ab->size;

    for (uint i = 0; i < sysmem.rsrv.nblock + 1; i++) {
      rb = sysmem.rsrv.block + i;

      if (i)
        rstart = rb[-1].base + rb[-1].size;
      else
        rstart = 0x0;

      if (i == sysmem.rsrv.nblock)
        rend = (Phys)-1ll;
      else
        rend = rb->base;

      if (__memblockoverlap (start, end, rstart, rend, &ms, &me)) {
        ms = ALIGN (ms, align);
        if (me - ms >= nbytes) {
          *pa = ms;
          return true;
        }
      }
    }
  }
  return false;
}

void * INIT
bootmemalloc (uint nbytes, uint align)
{
  Phys pa;
  void *va;

  if (nbytes == 0)
    return NULL;
  if (!bootmemfind (nbytes, align, &pa))
    return NULL;

  sysrsrvmem (pa, nbytes);

  va = P2V (pa);
  memset (va, 0, nbytes);
  KDBG ("alloc bootmem %p bytes: %p-%p %p\n", nbytes, pa, pa + nbytes - 1, va);
  return va;
}

static void DEBUG
memchunkdump (MemChunk *c)
{
  MemBlock *b;
  uint i;

  KDBG ("memchunk %s:\n", c->name);
  FOREACH_MEMCHUNK_BLOCK (c, i, b)
    KDBG ("\t[%p-%p]\n", b->base, b->base + b->size - 1);
}

static void DEBUG
sysmemdump (void)
{
  memchunkdump (&sysmem.avail);
  memchunkdump (&sysmem.rsrv);
}

static void
memremoveblock (MemChunk *c, uint idx)
{
  if (idx >= c->nblock)
    return;

  memmove (c->block + idx, c->block + idx + 1, (c->nblock - idx - 1) * sizeof (MemBlock));
  c->nblock--;
}

static void
meminsertblock (MemChunk *c, uint idx, Phys start, ulong size)
{
  MemBlock *block;

  if (c->nblock >= 32)
    panic ("nblock > 32");

  memmove (c->block + idx + 1, c->block + idx, (c->nblock - idx) * sizeof (MemBlock));
  block = &c->block[idx];
  block->base = start;
  block->size = size;
  c->nblock++;
}

static bool
memchunkin (MemChunk *c, Phys pa)
{
  MemBlock *block;
  uint idx;

  FOREACH_MEMCHUNK_BLOCK (c, idx, block) {
    if (pa < block->base)
      return false;
    else if (block->base <= pa && pa < block->base + block->size)
      return true;
  }
  return false;
}

static void
memchunkmerge (MemChunk *c)
{
  MemBlock *block, *next;
  Phys start, end;
  bool mergeable;

  for (int i = 0; i < c->nblock - 1; ) {
    block = c->block + i;
    next = c->block + i + 1;

    mergeable = memblockmerge (block, next, &start, &end);
    if (mergeable) {
      memremoveblock (c, i);
      block->base = start;
      block->size = end - start;
      continue;
    }
    i++;
  }

  // MemchunkDump(c);
}

static void
memnewblock (MemChunk *c, Phys start, ulong size)
{
  MemBlock *block;
  uint idx = 0;

  KLOG ("%s [%p-%p]\n", c->name, start, start + size - 1);

  FOREACH_MEMCHUNK_BLOCK (c, idx, block) {
    if (start <= block->base)
      break;
  }

  meminsertblock (c, idx, start, size);
  memchunkmerge (c);
}

bool
reservedaddr (Phys addr)
{
  return memchunkin (&sysmem.rsrv, addr);
}

void
sysavailmem (Phys base, ulong size)
{
  memnewblock (&sysmem.avail, base, size);
}

void
sysrsrvmem (Phys base, ulong size)
{
  memnewblock (&sysmem.rsrv, base, size);
}

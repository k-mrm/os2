#ifndef _MM_H
#define _MM_H

#include <types.h>

#define ALIGN(p, align)       (((ulong)(p) + (align)-1) & ~((align)-1))
#define ALIGNDOWN(p, align)   ((ulong)(p) & ~((align)-1))
#define PAGEALIGNED(p)        (((ulong)(p) & (PAGESIZE-1)) == 0)
#define PAGEALIGN(p)          ALIGN (p, PAGESIZE)
#define PAGEALIGNDOWN(p)      ALIGNDOWN (p, PAGESIZE)

typedef struct Vas  Vas;

/*
 *  Virtual Address Space
 */
struct Vas {
  PageTable pgdir;
  uint      level;
  uint      lowestlevel;
  bool      user;
};

void initkerneladdrspace (Vas *vas);
void *iomap (Phys pa, ulong nbytes);
Vas *kernelas (void);

#endif  // __MM_H


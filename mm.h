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
struct Vas
{
        PageTable pgdir;
        uint      level;
        uint      lowestlevel;
        bool      user;
};

void kernelmap (void) INIT;
void *devmmap (Phys pa, ulong nbytes);
Vas *kernelas (void);
Vas *allocvas (void);
void mappages (Vas *vas, ulong va, Phys pa, ulong size, ulong flags, bool remap);
Vas *uservas (void);

#endif  // __MM_H

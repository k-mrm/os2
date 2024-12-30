#ifndef _MM_H
#define _MM_H

#define ALIGN(p, align)		(((ulong)(p) + (align)-1) & ~((align)-1))
#define ALIGNDOWN(p, align)	((ulong)(p) & ~((align)-1))

#define PAGEALIGNED(p)		(((ulong)(p) & (PAGESIZE-1)) == 0)

#define PAGEALIGN(p)		ALIGN(p, PAGESIZE)
#define PAGEALIGNDOWN(p)	ALIGNDOWN(p, PAGESIZE)

#endif  // __MM_H

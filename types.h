#ifndef _TYPES_H
#define _TYPES_H

typedef unsigned long   u64;
typedef long            i64;
typedef unsigned int    u32;
typedef signed int      i32;
typedef unsigned short  u16;
typedef signed short    i16;
typedef unsigned char   u8;
typedef signed char     i8;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef unsigned char   uchar;

/* physical address */
typedef ulong   Phys;

#define NULL    ((void *)0)

typedef ulong   *PageTable;
typedef ulong   PTE;

typedef _Bool   bool;

#define true    1
#define false   0

#define offsetof(st, m)   ((ulong)((char *)&((st *)0)->m - (char *)0))

#define container_of(ptr, st, m)  \
  ({ const typeof(((st *)0)->m) *_mptr = (ptr); \
     (st *)((char *)_mptr - offsetof(st, m)); })

#define MAX(a, b) ((a) < (b) ? (b) : (a))
#define MIN(a, b) ((a) > (b) ? (b) : (a))

#define	KiB	(1024)
#define	MiB	(1024 * 1024)
#define	GiB	(1024 * 1024 * 1024)

#define USER

#endif    // _TYPES_H

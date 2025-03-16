#ifndef _BLOCK_H
#define _BLOCK_H

#include <types.h>
#include <compiler.h>
#include <device.h>
#include <irq.h>
#include <list.h>

typedef struct BlockDev BlockDev;
typedef struct BUF      BUF;

#define B_VALID         (1)

struct BUF
{
        LIST (BUF);

        BlockDev        *dev;

        int             bno;
        int             flags;
        char            data[1024];
        uint            refcount;
};

struct BlockDev
{
        Device  *device;

        void    *priv;

        int     (*read) (BlockDev *dev, BUF *b);
        int     (*write) (BlockDev *dev, BUF *b);
};

void binit (void) INIT;
int probeblock (BlockDev *dev);

BUF *bread (BlockDev *dev, int bno);
BUF *readbootblock (BlockDev *dev);
BUF *readsuperblock (BlockDev *dev);

BlockDev *getblkdev (char *name);

#endif  // _BLOCK_H

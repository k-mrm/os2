#include <types.h>
#include <compiler.h>
#include <block.h>
#include <kalloc.h>
#include <list.h>

typedef struct BCACHE   BCACHE;

static BlockDev *blockdev;

struct BCACHE
{
        // Lock
        List *cache;
};

BCACHE  bcache;

static BUF *
balloc (BlockDev *dev)
{
        BUF *buf;

        buf = zalloc ();
        if (!buf)
                return NULL;

        buf->dev = dev;
        buf->refcount = 1;
        PUSH (bcache.cache, buf);
        return buf;
}

static BUF *
bget (BlockDev *dev, int bno)
{
        BUF *buf;

        FOREACH (bcache.cache, buf)
        {
                if (buf->bno == bno && buf->dev == dev)
                {
                        buf->refcount++;
                        return buf;
                }
        }

        buf = balloc (dev);
        buf->bno = bno;
        return buf;
}

void INIT
binit (void)
{
        bcache.cache = newlist ();
}

BUF *
bread (BlockDev *dev, int bno)
{
        BUF *b = bget (dev, bno);
        if (!b)
                return NULL;

        if (!(b->flags & B_VALID))
        {
                dev->read (dev, b);
                b->flags |= B_VALID;
        }
        return b;
}

BUF *
readbootblock (BlockDev *dev)
{
        return bread (dev, 0);
}

BUF *
readsuperblock (BlockDev *dev)
{
        return bread (dev, 1);
}

int
probeblock (BlockDev *dev)
{
        blockdev = dev;
        return 0;
}

BlockDev *
getblkdev (char *name)
{
        // TODO: name?
        if (!blockdev)
        {
                panic ("no block device!");
        }

        return blockdev;
}

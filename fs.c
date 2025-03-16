#include <types.h>
#include <compiler.h>
#include <fs.h>
#include <list.h>
#include <printk.h>

List *filesystems;

INODE *
allocinode (FS *fs)
{
        return NULL;
}

void
regfsif (FS_IF *op)
{
        if (!filesystems)
        {
                filesystems = newlist ();
        }

        PUSH (filesystems, op);
        trace ("New Filesystem: %s\n", op->name);
}

void
deregfsif (FS_IF *op)
{
        listdelete (op);
}

static FS_IF *
identfs (BlockDev *dev)
{
        BUF *sb;
        FS_IF *fsif;
        
        sb = readsuperblock (dev);
        if (!sb)
                return NULL;

        FOREACH (filesystems, fsif)
        {
                if (fsif->fsisme (sb->data))
                {
                        return fsif;
                }
        }

        return NULL;
}

int
mountfs (char *path, char *name)
{
        BlockDev *dev;
        FS_IF *fsif;
        FS *fs;
        int ret;

        dev = getblkdev (name);
        if (!dev)
                return -1;

        fsif = identfs (dev);
        if (!fsif)
                return -1;

        fs = zalloc ();
        if (!fs)
                return -1;
        fs->block       = dev;
        fs->op          = fsif;

        if (!fs->op->probe)
                return -1;

        ret = fs->op->probe (fs);

        return ret;
}

static int
mountroot (void)
{
        return mountfs ("/", "sda");
}

void INIT
initfs (void)
{
        if (!filesystems || EMPTY (filesystems))
                goto nofs;

        binit ();
        if (mountroot () < 0)
                goto nofs;

        return;

nofs:
        panic ("No FileSystem");
}

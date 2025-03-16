#include <types.h>
#include <compiler.h>
#include <fs.h>
#include <list.h>
#include <string.h>

#define KPREFIX "fs:"

#include <printk.h>

List *filesystems;
FS *rootfs;

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
                        goto found;
        }
        fsif = NULL;

found:
        brelease (sb);
        return fsif;
}

static INODE *
ialloc (FS *fs, int inum)
{
        INODE *ino;

        ino = zalloc ();
        ino->fs = fs;
        ino->inum = inum;
        ino->refcount = 1;
        ino->new = true;

        PUSH (fs->itable, ino);

        return ino;
}

INODE *
iget (FS *fs, int inum)
{
        INODE *ino;

        FOREACH (fs->itable, ino)
        {
                if (ino->fs == fs && ino->inum == inum)
                {
                        ino->refcount++;
                        return ino;
                }
        }

        return ialloc (fs, inum);
}

void
iput (INODE *ino)
{
        if (ino->refcount == 0)
        {
                panic ("iput");
        }

        ino->refcount--;

        if (ino->refcount == 0)
        {
                brelease (ino->buf);
                listdelete (ino);
                free (ino);
        }
}

static char *
skippath (char *path, char *name, int *err)
{
        int len = 0;

        /* skip '/' ("////aaa/bbb" -> "aaa/bbb") */
        while (*path == '/')
                path++;

        /* get elem and inc path (get "aaa/" from "aaa/bbb", path = "bbb") */
        while ((*name = *path) && *path++ != '/')
        {
                name++;

                if (++len > 50)
                {
                        *err = 1;
                        return NULL;
                }
        }

        /* cut '/' from name ("aaa/" -> "aaa") */
        if (*name == '/')
                *name = 0;

        return path;
}

static INODE *
traverse (FS *fs, INODE *pi, char *path, char *name)
{
        int err = 0;

        path = skippath (path, name, &err);
        if (err)
                goto err;
        if (*path == 0 && *name == 0)
                goto ret;
        if (!S_ISDIR (pi->mode))
                goto ret;

        int inum = fs->op->search (pi, name);
        if (inum < 0)
                return NULL;

        iput (pi);

        pi = fs->op->iget (fs, inum);
        memset (name, 0, 128);

        return traverse (fs, pi, path, name);

ret:
        iput (pi);
        return pi;
err:
        iput (pi);
        return NULL;
}

INODE *
path2ino (char *path)
{
        INODE *ino;
        FS *fs;
        char name[128] = {0};

        if (*path == '/')
        {
                fs = rootfs;
                ino = fs->op->rootinode (fs);
        }
        else
        {
                panic ("unimpl");
        }

        return traverse (fs, ino, path, name);
}

static void
regmountpoint (char *path, FS *fs)
{
        if (strcmp (path, "/") == 0)
        {
                rootfs = fs;
        }
        else
        {
                panic ("unimpl");
        }
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
        fs->itable      = newlist ();

        if (!fs->op->probe)
                return -1;

        ret = fs->op->probe (fs);
        regmountpoint (path, fs);

        return ret;
}

static int
mountroot (void)
{
        return mountfs ("/", "sda");
}

void INIT
fsdbg (void)
{
        INODE *ino;

        ino = path2ino ("/README.md");
        if (ino)
        {
                FS *fs = ino->fs;
                unsigned char buf[128] = {0};
                trace ("found inum: %d\n", ino->inum);
                fs->op->readi (ino, buf, 0, 127);
                trace ("%s\n", buf);
                iput (ino);
        }
        else
        {
                trace ("not found\n");
        }
}

void INIT
initfs (void)
{
        if (!filesystems || EMPTY (filesystems))
                goto nofs;

        binit ();
        if (mountroot () < 0)
                goto nofs;

        fsdbg ();

        return;

nofs:
        panic ("No FileSystem");
}

#include <types.h>
#include <compiler.h>
#include <fs.h>
#include <block.h>
#include <module.h>
#include <list.h>
#include <string.h>

#define KPREFIX "ext2:"

#include <printk.h>
#include "ext2.h"

static BUF *
getsb (FS *fs)
{
        return bread (fs->block, 1);
}

static BUF *
getbg (FS *fs)
{
        return bread (fs->block, 2);
}

static int
findblock (EXT2_FS *ext2, BUF *bbmp)
{
        char chunk = 0xff;
        int bnum = 0;
        int bsize = 1024 << ext2->sb.s_log_block_size;

        for (int i = 0; i < bsize; i++)
        {
                if ((chunk = bbmp->data[i]) != 0xff)
                        break;
                bnum += 8;
        }

        if (chunk == 0xff)   /* no free inode */
                return -1;

        for(; chunk & 1; chunk >>= 1)
                bnum++;

        return bnum;
}

static int
bbmpwrite (EXT2_FS *ext2, BUF *bbmp, int bno, int bit)
{
        int bitn = bno;
        int chunkn;
        char c;
        int bsize = 1024 << ext2->sb.s_log_block_size;

        for (chunkn = 0; chunkn < bsize; chunkn++)
        {
                if (bitn < 8)
                        goto found;
                bitn -= 8;
        }

        /* invalid inode number */
        return -1;

found:
        c = bbmp->data[chunkn];
        if (bit)
                c |= 1 << bitn;
        else
                c &= ~(char)(1 << bitn);

        bbmp->data[chunkn] = c;
        bbmp->flags |= B_DIRTY;

        return 0;
}

static int
findino (EXT2_FS *ext2, BUF *ibmp)
{
        char chunk = 0xff;
        int inum = 1;
        int bsize = 1024 << ext2->sb.s_log_block_size;

        for (int i = 0; i < bsize; i++)
        {
                if ((chunk = ibmp->data[i]) != 0xff)
                        break;
                inum += 8;
        }

        if (chunk == 0xff)   /* no free inode */
                return -1;

        for(; chunk & 1; chunk >>= 1)
                inum++;

        return inum;
}

static int
ibmpwrite (EXT2_FS *ext2, BUF *ibmp, int ino, int bit)
{
        int bitn = ino - 1;
        int chunkn;
        char c;
        int bsize = 1024 << ext2->sb.s_log_block_size;

        for (chunkn = 0; chunkn < bsize; chunkn++)
        {
                if (bitn < 8)
                        goto found;
                bitn -= 8;
        }

        /* invalid inode number */
        return -1;

found:
        c = ibmp->data[chunkn];
        if (bit)
                c |= 1 << bitn;
        else
                c &= ~(char)(1 << bitn);

        ibmp->data[chunkn] = c;
        ibmp->flags |= B_DIRTY;

        return 0;
}

static int
ext2allocinum (FS *fs)
{
        EXT2_FS *ext2 = fs->priv;
        BUF *ibmp = bread (fs->block, ext2->bg.bg_inode_bitmap);
        int inum;

        inum = findino (ext2, ibmp);
        if (inum < 0)
                return -1;

        ibmpwrite (ext2, ibmp, inum, 1);

        brelease (ibmp);

        return inum;
}

static int
ext2allocblock (FS *fs)
{
        EXT2_FS *ext2 = fs->priv;
        BUF *bbmp = bread (fs->block, ext2->bg.bg_block_bitmap);
        int bno;

        bno = findblock (ext2, bbmp);
        if (bno < 0)
                return -1;

        bbmpwrite (ext2, bbmp, bno, 1);

        brelease (bbmp);

        return bno;
}

static int
ext2probe (FS *fs)
{
        BUF *sbb, *bgb;
        EXT2_FS *ext2;

        ext2 = alloc ();

        sbb = getsb (fs);
        bgb = getbg (fs);
        if (!sbb || !bgb)
                return -1;

        memcpy (&ext2->sb, sbb->data, sizeof (ext2->sb));
        memcpy (&ext2->bg, bgb->data, sizeof (ext2->bg));

        brelease (sbb);
        brelease (bgb);

        fs->priv = ext2;
        ext2->fs = fs;
        return 0;
}

static INODE *
ext2createi (FS *fs, char *name, INODE *dir, int mode, int dev)
{
        return NULL;
}

static void
ext2isync (INODE *ino)
{
        FS *fs = ino->fs;
        EXT2_INODE *ei = ino->priv;
        // TODO
}

static EXT2_INODE *
rawinode (EXT2_FS *ext2, int inum, BUF **bp)
{
        FS *fs = ext2->fs;
        int bsize = 1024 << ext2->sb.s_log_block_size;
        int inosize = ext2->sb.s_inode_size;
        u32 bgrp = (inum - 1) / (bsize / inosize);
        u64 offset = ((inum - 1) % (bsize / inosize)) * inosize;
        BUF *itable = bread (fs->block, ext2->bg.bg_inode_table + bgrp);

        if (bp)
                *bp = itable;

        return (EXT2_INODE *)(itable->data + offset);
}

static INODE *
ext2iget (FS *fs, int inum)
{
        EXT2_FS *ext2 = fs->priv;
        INODE *i = iget (fs, inum);
        BUF *b;
        EXT2_INODE *e = rawinode (ext2, inum, &b);

        if (!i)
                return NULL;
        if (!i->new)
                return i;

        i->mode = e->i_mode;
        i->size = e->i_size;
        i->atime = e->i_atime;
        i->ctime = e->i_ctime;
        i->mtime = e->i_mtime;
        i->dtime = e->i_dtime;
        i->links_count = e->i_links_count;
        i->blocks = e->i_blocks;
        memcpy (i->block, e->i_block, sizeof(u32) * 15);
        i->priv = e;
        i->buf = b;

        return i;
}

static INODE *
ext2rootinode (FS *fs)
{
        return ext2iget (fs, EXT2_ROOT_INO);
}

#define ext2inodenblock(_ei) ((_ei)->i_blocks / (2 << 0))

static BUF *
ext2readindirect (FS *fs, u32 *map, int bi)
{
        int idx = bi - 12;
        return bread (fs->block, map[idx]);
}

static BUF *
ext2inodeblock (INODE *ino, int bi)
{
        FS *fs = ino->fs; 

        if (bi < 12)
        {
                return bread (fs->block, ino->block[bi]);
        }
        else
        {
                BUF *map = bread (fs->block, ino->block[12]);
                BUF *b = ext2readindirect (fs, (u32 *)map->data, bi);
                brelease (map);
                return b;
        }

        return NULL;
}

static int
ext2searchdirent (EXT2_FS *ext2, unsigned char *blk, char *path)
{
        EXT2_DENTRY *d;
        char buf[EXT2_DIRENT_NAME_MAX];
        int bsize = 1024 << ext2->sb.s_log_block_size;

        for (u64 bpos = 0; bpos < bsize; bpos += d->rec_len)
        {
                d = (EXT2_DENTRY *)(blk + bpos);

                memset (buf, 0, EXT2_DIRENT_NAME_MAX);
                memcpy (buf, d->name, d->name_len);
                if (strcmp (buf, path) == 0)
                        return d->inode;
        }

        return -1;
}

static int
ext2search (INODE *pi, char *basename)
{
        FS *fs = pi->fs;
        EXT2_FS *ext2 = fs->priv;
        EXT2_INODE *ei = pi->priv;
        int inum;

        for (int i = 0; i < ext2inodenblock (ei); i++)
        {
                BUF *db = ext2inodeblock (pi, i);
                if (!db)
                        continue;

                if ((inum = ext2searchdirent (ext2, db->data, basename)) > 0)
                {
                        brelease (db);
                        return inum;
                }
                brelease (db);
        }

        return -1;
}

static int
ext2readi (INODE *ino, unsigned char *buf, u64 off, u64 size)
{
        FS *fs = ino->fs;
        EXT2_FS *ext2 = fs->priv;
        EXT2_INODE *ei = ino->priv;
        int bsize = 1024 << ext2->sb.s_log_block_size;
        unsigned char *base = buf;
        u32 offblk, lastblk, offblkoff;
        u64 cpsize;
        BUF *b;

        if (off > ino->size)
                return -1;
        if (off + size > ino->size)
                size = ino->size - off;

        offblk = off / bsize;
        lastblk = (size + off) / bsize;
        offblkoff = off % bsize;

        for (int i = offblk; i < ext2inodenblock (ei) && i <= lastblk; i++)
        {
                b = ext2inodeblock (ino, i);
                cpsize = MIN (size, bsize);
                
                if (offblkoff + cpsize > bsize)
                        cpsize = bsize - offblkoff;

                memcpy (buf, b->data + offblkoff, cpsize);

                buf += cpsize;
                size -= cpsize;
                offblkoff = 0;

                brelease (b);
        }

        return buf - base;
}

static int
ext2writei (INODE *ino, unsigned char *buf, u64 off, u64 size)
{
        FS *fs = ino->fs;
        EXT2_FS *ext2 = fs->priv;
        EXT2_INODE *ei = ino->priv;
        int bsize = 1024 << ext2->sb.s_log_block_size;
        u32 offblk, lastblk, offblkoff;
        u64 cpsize, sz = size;
        BUF *b;

        if (off > ino->size)
                return -1;
        if (off + size > ino->size)
                ino->size = off + size;

        offblk = off / bsize;
        lastblk = (size + off) / bsize;
        offblkoff = off % bsize;

        /*
        if (lastblk > ext2inodenblock (ei))
                ext2growinoblock (ino, lastblk - ext2inodenblock (ei));
                */

        for (int i = offblk; i < ext2inodenblock (ei) && i <= lastblk; i++)
        {
                b = ext2inodeblock (ino, i);
                cpsize = MIN (size, bsize);

                if (offblkoff + cpsize > bsize)
                        cpsize = bsize - offblkoff;

                memcpy (b->data + offblkoff, buf, cpsize);
                b->flags |= B_DIRTY;

                buf += cpsize;
                size = size > bsize ? size - bsize : 0;
                offblkoff = 0;

                brelease (b);
        }

        ext2isync (ino);

        return sz;
}

static bool
ext2fsisme (unsigned char *sbd)
{
        EXT2SUPERBLOCK *sb = (EXT2SUPERBLOCK *)sbd;

        return sb->s_magic == 0xef53;
}

static FS_IF ext2if = {
        .name           = "ext2",
        .probe          = ext2probe,
        .createi        = ext2createi,
        .iget           = ext2iget,
        .rootinode      = ext2rootinode,
        .sync           = ext2isync,
        .search         = ext2search,
        .readi          = ext2readi,
        .writei         = ext2writei,
        /*
        .mkdir          = ext2mkdir,
        */
        .fsisme         = ext2fsisme,
};

void
ext2init (void)
{
        regfsif (&ext2if);
}

MODULE_DECL ext2 = {
        .name           = "ext2",
        .description    = "ext2 file system",
        .init           = ext2init,
        .delete         = NULL,
};

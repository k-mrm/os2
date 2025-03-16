#include <types.h>
#include <compiler.h>
#include <fs.h>
#include <block.h>
#include <module.h>
#include <list.h>

#define KPREFIX "ext2:"

#include <printk.h>
#include "ext2.h"

static int
ext2probe (FS *fs)
{
        trace ("ext2 lets gooooooooooooooo\n");
        return -1;
}

static INODE *
ext2inode (FS *fs, int inum)
{
        ;
}

static bool
ext2fsisme (char *sbd)
{
        EXT2SUPERBLOCK *sb = (EXT2SUPERBLOCK *)sbd;

        return sb->s_magic == 0xef53;
}

static FS_IF ext2if = {
        .name           = "ext2",
        .probe          = ext2probe,
        /*
        .createi        = ext2createi,
        .inode          = ext2inode,
        .pathinode      = ext2pathinode,
        .readi          = ext2readi,
        .writei         = ext2writei,
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

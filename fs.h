#ifndef _FS_H
#define _FS_H

#include <types.h>
#include <compiler.h>
#include <block.h>
#include <list.h>

typedef struct FS_IF            FS_IF;
typedef struct FS               FS;
typedef struct INODE            INODE;
typedef struct SUPERBLOCK       SUPERBLOCK;
typedef struct DENTRY           DENTRY;

struct SUPERBLOCK
{
        
};

struct DENTRY
{
        SUPERBLOCK      *sb;
        DENTRY          *parent;
        char            name[50];
        INODE           *inode;
};

struct INODE
{
        SUPERBLOCK      *sb;

        u16     mode;
        u32     size;
        u32     atime;
        u32     ctime;
        u32     mtime;
        u32     dtime;
        u16     links_count;
        u32     blocks;
        u32     block[15];

        u32     inum;
        u32     major;
        u32     minor;
        u32     ref;
};

struct FS_IF
{
        LIST (FS_IF);

        char    *name;
        int     (*probe) (FS *fs);
        int     (*createi) ();
        INODE   *(*inode) (FS *fs, int inum);
        INODE   *(*pathinode) (FS *fs, char *path);
        int     (*readi) (FS *fs, INODE *ino, char *buf, u64 size);
        int     (*writei) (FS *fs, INODE *ino, char *buf, u64 size);
        int     (*mkdir) (FS *fs, char *path);

        bool    (*fsisme) (char *sb);
};

struct FS
{
        BlockDev         *block; 

        FS_IF            *op;
        SUPERBLOCK       *sb;
};

void initfs (void) INIT;
void regfsif (FS_IF *op);
INODE *allocinode (FS *fs);

#endif  // _FS_H

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

#define S_IFMT    0xf000

#define S_IFSOCK  0xc000
#define S_IFLNK   0xa000
#define S_IFREG   0x8000
#define S_IFBLK   0x6000
#define S_IFDIR   0x4000
#define S_IFCHR   0x2000
#define S_IFIFO   0x1000

#define S_ISUID   0x0800
#define S_ISGID   0x0400
#define S_ISVTX   0x0200

#define S_IRUSR   0x0100
#define S_IWUSR   0x0080
#define S_IXUSR   0x0040
#define S_IRGRP   0x0020
#define S_IWGRP   0x0010
#define S_IXGRP   0x0008
#define S_IROTH   0x0004
#define S_IWOTH   0x0002
#define S_IXOTH   0x0001

#define S_ISSOCK(_m)    (((_m) & S_IFMT) == S_IFSOCK)
#define S_ISLNK(_m)     (((_m) & S_IFMT) == S_IFLNK)
#define S_ISREG(_m)     (((_m) & S_IFMT) == S_IFREG)
#define S_ISBLK(_m)     (((_m) & S_IFMT) == S_IFBLK)
#define S_ISDIR(_m)     (((_m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(_m)     (((_m) & S_IFMT) == S_IFCHR)
#define S_ISFIFO(_m)    (((_m) & S_IFMT) == S_IFIFO)

struct SUPERBLOCK
{
        
};

struct DENTRY
{
        FS      *fs;
        DENTRY  *parent;
        char    name[50];
        INODE   *inode;
};

struct INODE
{
        LIST (INODE);

        FS      *fs;
        void    *priv;
        BUF     *buf;

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
        u32     refcount;
        bool    new;
};

struct FS_IF
{
        LIST (FS_IF);

        char    *name;
        int     (*probe) (FS *fs);
        INODE   *(*createi) (FS *fs, char *name, INODE *dir, int mode, int dev);
        INODE   *(*iget) (FS *fs, int inum);
        int     (*readi) (INODE *ino, unsigned char *buf, u64 off, u64 size);
        int     (*writei) (INODE *ino, unsigned char *buf, u64 off, u64 size);
        int     (*mkdir) (FS *fs, char *path);
        void    (*sync) (INODE *ino);
        int     (*search) (INODE *dir, char *basename);
        INODE   *(*rootinode) (FS *fs);

        bool    (*fsisme) (unsigned char *sb);
};

struct FS
{
        BlockDev        *block; 

        FS_IF           *op;
        SUPERBLOCK      sb;

        void            *priv;
        List            *itable;
        // Lock            lock;
};

void initfs (void) INIT;
void regfsif (FS_IF *op);

INODE *iget (FS *fs, int inum);
void iput (INODE *ino);

INODE *path2ino (char *path);

#endif  // _FS_H

#ifndef _FS_EXT2_H
#define _FS_EXT2_H

#include <types.h>
#include <compiler.h>

typedef struct EXT2SUPERBLOCK   EXT2SUPERBLOCK;
typedef struct EXT2_BG_DESC     EXT2_BG_DESC;
typedef struct EXT2_FS          EXT2_FS;
typedef struct EXT2_INODE       EXT2_INODE;
typedef struct EXT2_DENTRY      EXT2_DENTRY;

/* inode.i_mode */
#define EXT2_S_IFSOCK 0xc000
#define EXT2_S_IFLNK  0xa000
#define EXT2_S_IFREG  0x8000
#define EXT2_S_IFBLK  0x6000
#define EXT2_S_IFDIR  0x4000
#define EXT2_S_IFCHR  0x2000
#define EXT2_S_IFIFO  0x1000

#define EXT2_S_ISUID  0x0800
#define EXT2_S_ISGID  0x0400
#define EXT2_S_ISVTX  0x0200

#define EXT2_S_IRUSR  0x0100
#define EXT2_S_IWUSR  0x0080
#define EXT2_S_IXUSR  0x0040
#define EXT2_S_IRGRP  0x0020
#define EXT2_S_IWGRP  0x0010
#define EXT2_S_IXGRP  0x0008
#define EXT2_S_IROTH  0x0004
#define EXT2_S_IWOTH  0x0002
#define EXT2_S_IXOTH  0x0001

/* inode number */
#define EXT2_BAD_INO            1
#define EXT2_ROOT_INO           2
#define EXT2_ACL_IDX_INO        3
#define EXT2_ACL_DATA_INO       4
#define EXT2_BOOT_LOADER_INO    5
#define EXT2_UNDEL_DIR_INO      6

/* ext2 filetype(ext2_dirent.file_type) */
#define EXT2_FT_UNKNOWN         0
#define EXT2_FT_REG_FILE        1
#define EXT2_FT_DIR             2
#define EXT2_FT_CHRDEV          3
#define EXT2_FT_BLKDEV          4
#define EXT2_FT_FIFO            5
#define EXT2_FT_SOCK            6
#define EXT2_FT_SYMLINK         7

struct EXT2SUPERBLOCK
{
        u32     s_inodes_count;
        u32     s_blocks_count;
        u32     s_r_blocks_count;
        u32     s_free_blocks_count;
        u32     s_free_inodes_count;
        u32     s_first_data_block;
        u32     s_log_block_size;
        u32     s_log_frag_size;
        u32     s_blocks_per_group;
        u32     s_frags_per_group;
        u32     s_inodes_per_group;
        u32     s_mtime;
        u32     s_wtime;
        u16     s_mnt_count;
        u16     s_max_mnt_count;
        u16     s_magic;
        u16     s_state;
        u16     s_errors;
        u16     s_minor_rev_level;
        u32     s_lastcheck;
        u32     s_checkinterval;
        u32     s_creator_os;
        u32     s_rev_level;
        u16     s_def_resuid;
        u16     s_def_resgid;

        u32     s_first_ino;
        u16     s_inode_size;
        u16     s_block_group_nr;
        u32     s_feature_compat;
        u32     s_feature_incompat;
        u32     s_feature_ro_compat;
        u64     s_uuid[2];
        char    s_volume_name[16];
        char    s_last_mounted[64];
        u32     s_algo_bitmap;

        u8      s_prealloc_blocks;
        u8      s_prealloc_dir_blocks;
        u16     _align;

        u64     s_journal_uuid[2];
        u32     s_journal_inum;
        u32     s_journal_dev;
        u32     s_last_orphan;

        u32     s_hash_seed[4];
        u8      s_def_hash_version;
        u8      _padding[3];

        u32     s_default_mount_options;
        u32     s_first_meta_bg;
        char    _unused[760];
} PACKED;

struct EXT2_BG_DESC
{
        u32     bg_block_bitmap;
        u32     bg_inode_bitmap;
        u32     bg_inode_table;
        u16     bg_free_blocks_count;
        u16     bg_free_inodes_count;
        u16     bg_used_dirs_count;
        u16     bg_pad;
        char    bg_reserved[12];
} PACKED;

struct EXT2_INODE
{
        u16     i_mode;
        u16     i_uid;
        u32     i_size;
        u32     i_atime;
        u32     i_ctime;
        u32     i_mtime;
        u32     i_dtime;
        u16     i_gid;
        u16     i_links_count;
        u32     i_blocks;
        u32     i_flags;
        u32     i_osd1;
        u32     i_block[15];
        u32     i_generation;
        u32     i_file_acl;
        u32     i_dir_acl;
        u32     i_faddr;
        u32     i_osd2[3];
} PACKED;

struct EXT2_FS
{
        FS              *fs;
        EXT2SUPERBLOCK  sb;
        EXT2_BG_DESC    bg;
};

#define DT_UNKNOWN  0
#define DT_REG      1
#define DT_DIR      2
#define DT_CHR      3
#define DT_BLK      4
#define DT_FIFO     5
#define DT_SOCK     6
#define DT_LNK      7

struct EXT2_DENTRY
{
        u32     inode;
        u16     rec_len;
        u8      name_len;
        u8      file_type;
        char    name[];
} PACKED;

#define EXT2_DIRENT_NAME_MAX    255

#endif  // _FS_EXT2_H

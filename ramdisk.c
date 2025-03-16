#include <types.h>
#include <compiler.h>
#include <device.h>
#include <block.h>
#include <string.h>

extern char _binary_fs_img_end[];
extern char _binary_fs_img_size[];
extern char _binary_fs_img_start[];

typedef struct RAMDISK  RAMDISK;

static struct RAMDISK
{
        void    *base;
        int     size;
} ramdisk;

static int
ramdiskread (BlockDev *dev, BUF *buf)
{
        RAMDISK *disk = dev->priv;
        int offset;

        offset = buf->bno * 1024;

        memcpy (buf->data, disk->base + offset, 1024);

        return 0;
}

static int
ramdiskwrite (BlockDev *dev, BUF *buf)
{
        RAMDISK *disk = dev->priv;
        int offset;

        offset = buf->bno * 1024;

        memcpy (disk->base + offset, buf->data, 1024);

        return 0;
}

static BlockDev ramdiskdev = {
        .priv   = &ramdisk,
        .read   = ramdiskread,
        .write  = ramdiskwrite,
};

void
ramdiskinit (void)
{
        ramdisk.base = _binary_fs_img_start;
        probeblock (&ramdiskdev);
}

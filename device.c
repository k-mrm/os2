#include <types.h>
#include <compiler.h>
#include <device.h>
#include <timer.h>
#include <printk.h>
#include <mm.h>
#include <string.h>

#define FOREACH_DEVICE(_dev)  \
        for (_dev = devs; _dev < &devs[cdev]; _dev++)

#define FOREACH_MY_DEVICE(_dev)  \
        for (_dev = mydevs[0]; _dev < &mydevs[0][cmydev]; _dev++)

static Device devs[256];
static int    cdev = 0;
static Device mydevs[8 /*NCPU*/][128];
static int    cmydev = 0;

static Device *
newdevice (void)
{
        return &devs[cdev++];
}

static Device *
newdevicecpu (int cpu)
{
        return &mydevs[cpu][cmydev++];
}

Device *
regdevicemycpu (char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
                void *priv)
{
        return regdevicecpu (0, ty, name, bus, drv, parent, priv);
}

Device *
regdevicecpu (int cpu, char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
              void *priv)
{
        Device *dev;
        
        if (!drv)
                return NULL;

        dev = newdevicecpu (cpu);

        if (!dev)
                return NULL;

        dev->type     = ty;
        dev->parent   = parent;
        strcpy (dev->name, name);
        dev->driver   = drv;
        dev->priv     = priv;
        return dev;
}

Device *
regdevice (char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
           void *priv)
{
        Device *dev;

        if (!drv)
                return NULL;
        
        dev = newdevice ();

        if (!dev)
                return NULL;

        dev->type     = ty;
        dev->parent   = parent;
        strcpy (dev->name, name);
        dev->driver   = drv;
        dev->priv     = priv;
        return dev;
}

void
devprobe (char *type)
{
        Device *dev;

        FOREACH_DEVICE (dev)
        {
                if (strcmp (dev->type, type) == 0)
                {
                        if (dev->driver->probe)
                                dev->driver->probe (dev);
                }
        }
}

void
mydevprobe (char *type)
{
        Device *dev;

        FOREACH_MY_DEVICE (dev)
        {
                if (strcmp (dev->type, type) == 0)
                {
                        if (dev->driver->probe)
                                dev->driver->probe (dev);
                }
        }
}

void
lsdev (void)
{
        Device *dev;

        FOREACH_DEVICE (dev)
        {
                printk ("%s device: %s\n", dev->type, dev->name);

                if (dev->driver)
                {
                        printk ("  driver: %s\n", dev->driver->name);
                        printk ("    %s\n", dev->driver->description);
                }
        }

        FOREACH_MY_DEVICE (dev)
        {
                printk ("%s device: %s\n", dev->type, dev->name);

                if (dev->driver)
                {
                        printk ("  driver: %s\n", dev->driver->name);
                        printk ("    %s\n", dev->driver->description);
                }
        }
}

IOMEM *
iomap (Device *dev, Phys base, uint size)
{
        IOMEM   *iomem;

        if (!dev->iomem)
                dev->iomem = newlist ();

        iomem = alloc ();
        if (!iomem)
        {
                return NULL;
        }

        iomem->base     = devmmap (base, size);
        if (!iomem->base)
        {
                goto failed;
        }
        iomem->pbase    = base;
        iomem->size     = size;

        PUSH (dev->iomem, iomem);

        return iomem;
failed:
        free (iomem);
        return NULL;
}

int
deviceresume (Device *dev)
{
        return -1;
}

int
devicesuspend (Device *dev)
{
        return -1;
}

int
regbus (char *name, Bus *parent)
{
        return -1;
}

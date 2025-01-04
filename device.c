#include <types.h>
#include <compiler.h>
#include <device.h>
#include <timer.h>
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

int
regdevicemycpu (char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
                DeviceStruct *priv)
{
  return regdevicecpu (0, ty, name, bus, drv, parent, priv);
}

int
regdevicecpu (int cpu, char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
              DeviceStruct *priv)
{
  Device *dev = newdevicecpu (cpu);

  if (!dev)
    return -1;
  if (!drv)
    return -1;

  dev->type     = ty;
  dev->parent   = parent;
  dev->name     = name;
  dev->bus      = bus;
  dev->driver   = drv;
  dev->priv     = priv;
  priv->device  = dev;
  return 0;
}

int
regdevice (char *ty, char *name, Bus *bus, Driver *drv, Device *parent,
           DeviceStruct *priv)
{
  Device *dev = newdevice ();

  if (!dev)
    return -1;
  if (!drv)
    return -1;

  dev->type     = ty;
  dev->parent   = parent;
  dev->name     = name;
  dev->bus      = bus;
  dev->driver   = drv;
  dev->priv     = priv;
  priv->device  = dev;
  return 0;
}

void
devprobe (char *type)
{
  Device *dev;

  FOREACH_DEVICE (dev) {
    if (strcmp (dev->type, type) == 0) {
      if (dev->driver->probe)
        dev->driver->probe (dev);
    }
  }
}

void
mydevprobe (char *type)
{
  Device *dev;
  FOREACH_MY_DEVICE (dev) {
    if (strcmp (dev->type, type) == 0) {
      if (dev->driver->probe)
        dev->driver->probe (dev);
    }
  }
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

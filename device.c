#include <types.h>
#include <compiler.h>
#include <device.h>
#include <timer.h>

static Device devs[256];
static int cdevs = 0;

static Device *
newdevice (void)
{
  return &devs[cdevs++];
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

  if (dev->driver->probe)
    return dev->driver->probe (dev);
  else
    return -1;
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

#include <types.h>
#include <compiler.h>
#include <pci.h>
#include <device.h>

static int
pcidevprobe (Device *dev)
{
  PciDev *pci = (PciDev *)dev->priv;

  return pci->driver->probe (pci);
}

static void
pcidevsuspend (Device *dev)
{
  ;
}

static void
pcidevresume (Device *dev)
{
  ;
}

static Driver pcidriver = {
  .name         = "PCI",
  .description  = "PCI(e) generic driver",
  .probe        = pcidevprobe,
  .suspend      = pcidevsuspend,
  .resume       = pcidevresume,
  .param        = "disable",
};

static int
regpci ()
{
  ;
}

void INIT
pciinit (void)
{
  for (int i = 0; )
}

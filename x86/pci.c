#include <compiler.h>
#include <types.h>
#include "arch.h"
#include "pci.h"

#define PCI_CONFIG_ADDR_PORT  0x0CF8
#define PCI_CONFIG_DATA_PORT	0x0CFC

// static Lock   lock;

#define CONF(_bus, _devfn, _reg) \
  ((1ul << 31) | (((_reg) & 0xf00) << 16) | ((_bus) << 16) | ((_devfn) << 8 | ((_reg) & 0xfc)))

int
pcicfgread (int bus, int devfn, int reg, int size, u32 *v)
{
  u32 val;

  // acquire lock;
  outl (PCI_CONFIG_ADDR_PORT, CONF (bus, devfn, reg));
  val = inl (PCI_CONFIG_DATA_PORT);
  // release lock;

  switch (size) {
    case 1:   *v = (u8)val;
    case 2:   *v = (u16)val;
    case 4:   *v = val;
    default:  return -1;
  }
  return 0;
}

int
pcicfgwrite (int bus, int devfn, int reg, int size, u32 v)
{
  u32 val;

  switch (size) {
    case 1:   val = (u8)v;
    case 2:   val = (u16)v;
    case 4:   val = v;
    default:  return -1;
  }

  // acquire lock;
  outl (PCI_CONFIG_ADDR_PORT, CONF (bus, devfn, reg));
  outl (PCI_CONFIG_DATA_PORT, val);
  // release lock;
  return 0;
}

void
x86pciinit ()
{
  ;
}

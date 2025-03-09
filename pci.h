#ifndef _PCI_H
#define _PCI_H

#include <types.h>
#include <compiler.h>

typedef struct PciDev     PciDev;
typedef struct PciDriver  PciDriver;
typedef struct PciConfig  PciConfig;

struct PciConfig
{

};

struct PciDriver
{
  char    *name;
};

struct PciDev
{
  DEVICE_STRUCT;

  PciConfig     cfg;
  PciDriver     *driver;
};

void pciprobe (void);

#endif  // _PCI_H

#ifndef _PCI_H
#define _PCI_H

#include <types.h>
#include <compiler.h>

typedef struct PciDev     PciDev;
typedef struct PciDriver  PciDriver;

struct PciConfigSpace {

};

struct PciDriver {

};

struct PciDev {
  Device      *dev;
  PciDriver   *driver;
};

void pciprobe (void);

#endif  // _PCI_H

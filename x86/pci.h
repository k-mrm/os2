#ifndef _X86_PCI_H
#define _X86_PCI_H

#include <compiler.h>
#include <types.h>
#include "arch.h"

int pcicfgread (int bus, int devfn, int reg, int size, u32 *v);
int pcicfgwrite (int bus, int devfn, int reg, int size, u32 v);

#endif  // _X86_PCI_H

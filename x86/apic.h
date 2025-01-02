#ifndef _X86_APIC_H
#define _X86_APIC_H

#include <types.h>

typedef struct Apic       Apic;
typedef struct ApicOps    ApicOps;

struct ApicOps {
  int   (*probe) (Apic *apic);
  u32   (*read) (Apic *apic, u32 reg);
  void  (*write) (Apic *apic, u32 reg, u32 val);
  void  (*sendipi) (Apic *apic, int id);
};

struct Apic {
  ApicOps   *ops;
  void      *base;
  Phys      basepa;
  int       id;
};

void xapicinit (uint id);
void x2apicinit (uint id);
void apicinit (uint id, ApicOps *ops);

#endif    // _X86_APIC_H

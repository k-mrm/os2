#ifndef _CPU_H
#define _CPU_H

#include <types.h>
#include <compiler.h>
#include <list.h>
#include <device.h>
#include <x86/arch.h>

typedef struct Proc   Proc;
typedef struct Cpu    Cpu;

struct Cpu {
  DEVICE_STRUCT;
  
  int     cpuid;
  Proc    *currentproc;
  List    *runqueue;
  Proc    *kernel;
};

Cpu *mycpu (void);
Cpu *getcpu (int id);
void initcpu (int cpuid);

#endif  // _CPU_H

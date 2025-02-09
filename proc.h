#ifndef _PROC_H
#define _PROC_H

#include <types.h>
#include <compiler.h>
#include <cpu.h>
#include <mm.h>
#include <list.h>
#include <x86/context.h>
#include <x86/trap.h>

typedef enum ProcState  ProcState;
typedef struct Proc     Proc;

enum ProcState {
  NONE,
  READY,
  RUNNING,
  BLOCKING,
  ZOMBIE,
};

struct Proc {
  LIST (Proc);

  // Lock       lock;
  ProcState     state;
  char          pname[32];

  Proc          *parent;
  Vas           *as;    // address space
  uint          procid;
  Cpu           *currentcpu;
  Trapframe     *tf;
  Context       context;
  void          *kstack;
  void          *ksp;

  int           exitstatus;
  bool          paniconexit;

  bool          user;
  // for kernel process
  int           (*func) (void *arg);
  void          *arg;
};

void initprocess (void);
void initkernelproc (void);
int idleprocess (void *a);
int spawn (char *pname, Proc *parent, int (*proc) (void *arg), void *arg);
int killpid (uint pid);
int kill (char *pname);
void schedule (void);
void exit (int status);


#endif  // _PROC_H

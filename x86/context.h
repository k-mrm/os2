#ifndef _X86_CONTEXT_H
#define _X86_CONTEXT_H

#include <types.h>
#include <compiler.h>

typedef struct KStackFrame  KStackFrame;
typedef struct Context      Context;
typedef struct Proc         Proc;

struct KStackFrame {
  u64   r15;
  u64   r14;
  u64   r13;
  u64   r12;
  u64   rbx;
  u64   rbp;
  u64   rip;
} PACKED;

struct Context {
  u64   rsp;    // must be fast!
};

void forkret (void);
Proc *cswitch (Context *prev, Context *next, Proc *pprev);

#endif  // _X86_CONTEXT_H

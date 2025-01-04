#ifndef _X86_CONTEXT_H
#define _X86_CONTEXT_H

#include <types.h>
#include <compiler.h>

typedef struct KStackFrame  KStackFrame;
typedef struct Context      Context;

struct KStackFrame {
  u64   r15;
  u64   r14;
  u64   r13;
  u64   r12;
  u64   rbx;
  u64   rbp;
  u64   rip;
};

struct Context {
  u64   rsp;
};

void forkret (void);
void cswitch (Context *prev, Context *next);

#endif  // _X86_CONTEXT_H

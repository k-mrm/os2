#include <types.h>
#include <compiler.h>
#include <mm.h>
#include <cpu.h>
#include <kalloc.h>
#include <string.h>
#include <proc.h>
#include <x86/arch.h>
#include <x86/trap.h>

#define KPREFIX       "proc:"

#include <printk.h>

static Proc *proctable[256];
static int  np = 0;
static Proc *initproc;
static Proc *kidle;
static uint procidtable = 0;

static uint
procid (void)
{
  return procidtable++;
}

static void
initnewproctf (Proc *p, Trapframe *tf)
{
  if (p->user) {
    ;
  } else {
    u64 cs, rflags, ss;
    asm volatile ("mov  %%cs, %0" : "=r" (cs));
    asm volatile ("mov  %%ss, %0" : "=r" (ss));
    asm volatile (
      "pushfq\n"
      "pop  %0\n" : "=r" (rflags)
    );

    tf->rip     = (u64)p->func;
    tf->rdi     = (u64)p->arg;
    tf->cs      = cs;
    tf->rflags  = rflags | EFLAGS_IF;
    tf->ss      = ss;
    tf->rsp     = (u64)tf;
  }
}

static Proc *
newkproc (void)
{
  Proc  *p = zalloc ();
  void  *sp;

  if (!p)
    return NULL;

  strcpy (p->pname, "kernel");
  p->parent     = NULL;
  p->procid     = procid ();
  p->as         = kernelas ();
  p->currentcpu = mycpu ();

  return p;

err:
  free (p);
  return NULL;
}

static Proc *
newproc (char *name, Proc *parent, bool user, int (*pfunc) (void *arg), void *parg)
{
  // Proc *p = malloc (sizeof *p);
  Proc  *p = zalloc ();
  void  *sp;

  if (!p)
    return NULL;

  p->procid = procid ();

  p->user = user;
  if (!p->user) {
    p->func = pfunc;
    p->arg  = parg;
  }

  strcpy (p->pname, name);

  p->kstack = zalloc ();
  if (!p->kstack)
    goto err;
  sp = p->kstack + PAGESIZE;
  // init trapframe
  sp -= sizeof *p->tf;
  initnewproctf (p, sp);
  p->tf = sp;
  KLOG ("p->tf->sp:%p, ss:%p, rflags:%p, cs:%p, rip:%p\n",
        p->tf->rsp, p->tf->ss, p->tf->rflags, p->tf->cs, p->tf->rip);
  // init stackframe for context switch
  sp -= sizeof (KStackFrame);
  ((KStackFrame *)sp)->rip = (u64)forkret;

  p->context.rsp  = (u64)sp;
  p->state        = READY;
  proctable[np++] = p;
  return p;
  
err:
  free (p);
  return NULL;
}

void
initkernelproc (void)
{
  Cpu   *cpu  = mycpu ();
  Proc  *kp   = newkproc ();

  cpu->kernel       = kp;
  cpu->currentproc  = kp;

  kp->currentcpu = cpu;
}

void
initprocess (void)
{
  ;
}

int NORETURN
idleprocess (void *a)
{
  int i = 0;
  u64 rflags;

  for (;;) {
    asm volatile (
      "pushfq\n"
      "pop  %0\n" : "=r" (rflags)
    );
    printk ("%p %d\n", rflags, i++);
    for (int n = 0; n < 10000000; n++)
      ;
  }
}

int
spawn (char *pname, Proc *parent, int (*pfunc) (void *arg), void *parg)
{
  Proc    *p;

  if (!parent)
    parent = initproc;
  p = newproc (pname, parent, false, pfunc, parg);
  if (!p)
    return -1;

  if (strcmp (pname, "kidle") == 0)
    kidle = p;
  return 0;
}

static Proc *
nextproc (Cpu *cpu)
{
  return kidle;
  if (EMPTY (cpu->runqueue))
    return kidle;
}

void
schedtail (void)
{
  return;
}

static void
contextswitch (Cpu *cpu, Proc *prev, Proc *next)
{
  cpu->currentproc = next;
  next->currentcpu = cpu;

  KLOG ("cswitch: prev %p -> %p\n", prev, next);
  // switchas (prev->as, next->as);
  cswitch (&prev->context, &next->context);
}

void
schedule (void)
{
  Cpu   *cpu    = mycpu ();
  Proc  *nextp  = nextproc (cpu); 

  if (!cpu)
    panic ("cpu!?");
  if (!nextp)
    panic ("next process!?");
  contextswitch (cpu, cpu->currentproc, nextp);
}

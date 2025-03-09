#include <types.h>
#include <compiler.h>
#include <mm.h>
#include <cpu.h>
#include <kalloc.h>
#include <string.h>
#include <proc.h>
#include <list.h>
#include <x86/arch.h>
#include <x86/trap.h>

#define KPREFIX       "proc:"

#include <printk.h>

typedef struct RunQueue   RunQueue;
typedef struct PFreeList  PFreeList;

struct RunQueue
{
        // Lock  *lock;
        List  *queue;
        int   nqueue;
};

struct PFreeList
{
        // Lock   *lock;
        List  *freelist;
};

static Proc       *proctable[256];
static int        np = 0;
static Proc       *kidle;
static uint       procidtable = 0;
static RunQueue   rq;
static PFreeList  freelist;

static void
rqinit (RunQueue *r)
{
        // initlock (r->lock);
        r->queue  = newlist ();
        r->nqueue = 0;
}

static void
freelistinit (PFreeList *f)
{
        f->freelist = newlist ();
}

// Locked r
static void
rqreg (RunQueue *r, Proc *p)
{
        if (p->state != READY)
                return;

        PUSH (r->queue, p);
        r->nqueue++;
}

// Locked r
static Proc *
rqpop (RunQueue *r)
{
        Proc *p;

        if (EMPTY (r->queue))
                return NULL;
        p = FIRST (r->queue);
        listdelete (p);
        r->nqueue--;

        return p;
}

static uint
procid (void)
{
        return procidtable++;
}

static void
initnewproctf (Proc *p, Trapframe *tf)
{
        if (p->user)
        {
                ;
        }
        else
        {
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

static void
fire (Proc *p)
{
        List *flist;

        // acquire
        flist = freelist.freelist;

        PUSH (flist, p);

        // release
}

static void
dead (Proc *p)
{
        if (!p)
                return;
        p->state = ZOMBIE;
}

static void
ready (Proc *p)
{
        if (!p)
                return;
        // Lock rq
        p->state = READY;
        rqreg (&rq, p);
        // Unlock rq
}

// Locked rq
static void
running (Proc *p)
{
        if (!p)
                return;
        // assert (rq is locked)
        p->state = RUNNING;
}

static void
block (Proc *p)
{
        if (!p)
                return;
        p->state = BLOCKING;
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
        if (!p->user)
        {
                p->func = pfunc;
                p->arg  = parg;
        }

        p->parent = parent;

        strcpy (p->pname, name);

        p->kstack = zalloc ();
        if (!p->kstack)
                goto err;
        sp = p->kstack + PAGESIZE;
        // init trapframe
        sp -= sizeof *p->tf;
        initnewproctf (p, sp);
        p->tf = sp;
        log ("p->tf->sp:%p, ss:%p, rflags:%p, cs:%p, rip:%p\n",
             p->tf->rsp, p->tf->ss, p->tf->rflags, p->tf->cs, p->tf->rip);
        // init stackframe for context switch
        sp -= sizeof (KStackFrame);
        ((KStackFrame *)sp)->rip = (u64)forkret;

        p->context.rsp  = (u64)sp;
        return p;

err:
        free (p);
        return NULL;
}

static void
freeproc (Proc *p)
{
        log ("freed %s\n", p->pname);
        listdelete (p);

        free (p->kstack);
        memset (p, sizeof *p, 0);
        free (p);
}

void
initkernelproc (void)
{
        ;
}

int testtask1 (void *a);
int testtask2 (void *a);
int testtask3 (void *a);
int pfreethread (void *_) NORETURN;

void
initprocess (void)
{
        rqinit (&rq);
        freelistinit (&freelist);

        spawn ("kidle", NULL, idleprocess, NULL);
        spawn ("task1", NULL, testtask1, NULL);
        spawn ("task2", NULL, testtask2, NULL);
        spawn ("task3", NULL, testtask3, NULL);
        spawn ("procfree", NULL, pfreethread, NULL);
}

int NORETURN
idleprocess (void *a)
{
        for (;;)
        {
                HLT;
        }
}

int NORETURN
testtask1 (void *a)
{
        int i = 0;
        for (;;)
        {
                printk ("testtask1 %p %04d\n", a, i++);
                for (int n = 0; n < 10000000; n++)
                        ;
                if (i == 100)
                        exit (0);
        }
}

int NORETURN
testtask2 (void *a)
{
        int i = 0;
        for (;;)
        {
                printk ("testtask2 %p %04d\n", a, i++);
                for (int n = 0; n < 10000000; n++)
                        ;
                if (i == 30)
                        exit (0);
        }
}

int NORETURN
testtask3 (void *a)
{
        int i = 0;
        for (;;)
        {
                printk ("testtask3 %p %04d\n", a, i++);
                for (int n = 0; n < 10000000; n++)
                        ;
                if (i == 40)
                        exit (0);
        }
}

int NORETURN
pfreethread (void *_)
{
        Proc *p, *np;

        for (;;)
        {
                // acquire freelist.lock;
                FOREACH_SAFE (freelist.freelist, p, np)
                {
                        if (p->state != ZOMBIE)
                                panic ("wtf");

                        freeproc (p);
                }
                // sleep (myproc, &freelist.lock);
                // release freelist.lock;
        }
}

int
spawn (char *pname, Proc *parent, int (*pfunc) (void *arg), void *parg)
{
        Proc *p;

        p = newproc (pname, parent, false, pfunc, parg);
        if (!p)
                return -1;

        if (strcmp (pname, "kidle") == 0)
        {
                kidle = p;
                return 0;
        }

        ready (p);
        return 0;
}

void
exit (int status)
{
        Cpu  *cpu  = mycpu ();
        Proc *proc = cpu->currentproc;

        dead (proc);

        proc->exitstatus = status;

        if (proc->parent)
        {
                // TODO
        }
        else
        {
                fire (proc);
        }

        schedule ();
        // never return here
        panic ("dead proc!?");
}

static Proc *
nextproc (Proc *prev)
{
        Proc *p;

        if (prev && prev->state == RUNNING)
                ready (prev);

        p = rqpop (&rq);
        if (!p)
                return kidle;
        return p;
}

/*
void
sleep (void *chan, Lock *lock)
{
  ;
}

void
wakeup (void *chan)
{
  ;
}
*/

void
schedtail (void)
{
        return;
}

void
schedule (void)
{
        Cpu     *cpu  = mycpu ();
        Proc    *prev = cpu->currentproc;
        Proc    *next = nextproc (prev); 
        Context *c;

        if (!next)
                panic ("next process!?");

        cpu->currentproc = next;
        next->currentcpu = cpu;
        running (next);

        log ("cswitch: prev %p(%s) -> %p(%s)\n",
             prev, prev ? prev->pname : "NULL", next, next->pname);
        if (UNLIKELY (!prev))
        {
                // from kernel
                c = &cpu->context;
        }
        else
        { 
                // from process
                c = &prev->context;
        }

        // switchas (prev->as, next->as);
        prev = cswitch (c, &next->context, prev);

        log ("cswitch returned: last:%s c:%s %d\n",
             prev ? prev->pname : "kernel", cpu->currentproc->pname, interruptible ());
}

#include <types.h>
#include <compiler.h>
#include <mm.h>
#include <cpu.h>
#include <kalloc.h>
#include <string.h>
#include <proc.h>
#include <list.h>
#include <elf.h>
#include <syscall.h>
#include <timer.h>
#include <x86/arch.h>
#include <x86/mm.h>
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
                u64 cs, rflags, ss;
                asm volatile ("mov  %%cs, %0" : "=r" (cs));
                asm volatile ("mov  %%ss, %0" : "=r" (ss));
                asm volatile (
                        "pushfq\n"
                        "pop  %0\n" : "=r" (rflags)
                );

                tf->rip     = 0x1000;
                tf->cs      = cs;
                tf->rflags  = rflags | EFLAGS_IF;
                tf->ss      = ss;
                tf->rsp     = USTACKTOP;
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

        p->as = NULL;
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

static void
inituserproc (void)
{
        Proc *p;
        extern char _binary_initcode_start[];
        extern char _binary_initcode_end[];
        int isize = _binary_initcode_end - _binary_initcode_start;
        void *initcode;

        p = newproc ("init0", NULL, true, NULL, NULL);
        if (!p)
                return;

        p->as = uservas ();
        p->cwd = path2ino ("/");

        initcode = zalloc ();
        memcpy (initcode, _binary_initcode_start, isize);
        mappages (p->as, 0x1000, V2P (initcode), PAGESIZE,
                  pnormal () | preadonly () | pexecutable (), false);

        ready (p);
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

        inituserproc ();
}

static int
exec (const char *path, const char **argv)
{
        INODE *elf;
        FS *fs;
        EHDR ehdr;
        PHDR phdr;
        int status, size = 0;
        u64 phoff, flags;
        Cpu *cpu = mycpu ();
        Proc *proc = cpu->currentproc;
        void *p;
        int uargc = 0;
        Vas *as = uservas ();
        Vas *oldas = proc->as;
        void *top = as->ustack + PAGESIZE;
        void *sp = top;
        u64 args[10] = {0};

        elf = path2ino (path);
        if (!elf)
        {
                goto err;
        }

        fs = elf->fs;
        status = fs->op->readi (elf, (uchar *)&ehdr, 0, sizeof ehdr);
        if (status != sizeof (ehdr))
        {
                goto err;
        }
        if (!iself (&ehdr))
        {
                goto err;
        }
        if (ehdr.e_type != ET_EXEC)
        {
                goto err;
        }

        phoff = ehdr.e_phoff;
        for (int i = 0; i < ehdr.e_phnum; i++, phoff += sizeof phdr)
        {
                p = zalloc ();
                flags = 0;

                status = fs->op->readi (elf, (uchar *)&phdr, phoff, sizeof phdr);
                if (status != sizeof phdr)
                {
                        goto err;
                }
                if (phdr.p_type != PT_LOAD)
                {
                        continue;
                }

                if (!PAGEALIGNED (phdr.p_vaddr))
                        panic ("o");

                flags |= phdr.p_flags & PF_X ? pexecutable () | preadonly () : 0;
                flags |= phdr.p_flags & PF_W ? pwritable () : 0;

                size += fs->op->readi (elf, p, phdr.p_offset, phdr.p_filesz);
                mappages (as, phdr.p_vaddr, V2P (p), PAGESIZE, pnormal () | flags, false);
        }

        // setup arguments
        for (; argv && argv[uargc]; uargc++)
        {
                sp -= strlen (argv[uargc]) + 1;
                sp = (void *)((u64)sp & ~0xf);
                if (sp < as->ustack)
                {
                        goto err;
                }

                memcpy (sp, argv[uargc], strlen (argv[uargc]));
                args[uargc] = USTACKTOP - (top - sp);
        }

        sp -= sizeof (args[0]) * (uargc + 1);
        sp = (void *)((u64)sp & ~0xf);
        if (sp < as->ustack)
        {
                goto err;
        }
        memcpy (sp, args, sizeof (args[0]) * (uargc + 1));

        proc->tf->rip = ehdr.e_entry;
        proc->tf->rdi = uargc + 1;
        proc->tf->rsi = USTACKTOP - (top - sp);
        proc->tf->rsp = USTACKTOP - (top - sp);
        proc->as = as;

        trace ("exec %s rip %#x rdi %d rsp %#x\n", path, proc->tf->rip, proc->tf->rdi, proc->tf->rsp);

        freeuvas (oldas);
        switchvas (proc->as);

        return 0;
err:
        panic ("err");
        freeuvas (as);
        return -1;
}

static int
sysexec (const char * USER path, const char ** USER argv)
{
        // TODO: copyuser
        return exec (path, argv);
}

SYSCALL_DEFINE (SYS_EXEC, sysexec);

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
        {
                return -1;
        }

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

        if (next->as)
        {
                switchvas (next->as);
        }

        prev = cswitch (c, &next->context, prev);

        log ("cswitch returned: last:%s c:%s %d\n",
             prev ? prev->pname : "kernel", cpu->currentproc->pname, interruptible ());
}

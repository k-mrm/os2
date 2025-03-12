#include <types.h>
#include <compiler.h>
#include <module.h>

#define KPREFIX         "Module:"

#include <printk.h>

extern MODULE __module_s[];
extern MODULE __module_e[];

static int
modelfcheck (void *p)
{
        ;
}

static int
readmodelf (void *p, MODELF *elf)
{
        ;
}

int
moduleload (MODELF *elf)
{
        ;
}

void
delmodule (MODULE *m)
{
        if (m && m->delete)
                m->delete ();
}

void
initmodule (void)
{
        for (MODULE *m = __module_s; m < __module_e; m++)
        {
                log ("Module Loading: %s\n", m->name);
                if (m->init)
                        m->init ();
        }
}

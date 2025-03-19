#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <types.h>
#include <compiler.h>
#include <device.h>
#include <irq.h>

typedef struct EarlyConsole     EarlyConsole;
typedef struct Console  Console;

struct EarlyConsole
{
        char    name[40];
        Console *console;
};

struct Console
{
        Device  *device;

        void    *priv;

        int     (*write) (Console *cs, const char *buf, uint n);
        int     (*read) (Console *cs, char *buf, uint n);
        int     (*csirq) (Console *cs, Irq *irq);
};

extern Console *console;

int probeconsole (Device *dev);
void earlyconsole (EarlyConsole *ec);

int consoleirq (Irq *irq);

#endif  // _CONSOLE_H

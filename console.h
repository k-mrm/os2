#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <types.h>
#include <device.h>

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

        int     (*write) (Console *, const char *, uint);
        int     (*read) (Console *, char *, uint);
};

extern Console *console;

int probeconsole (Device *dev);
void earlyconsole (EarlyConsole *ec);

#endif  // _CONSOLE_H

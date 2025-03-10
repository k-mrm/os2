#include <types.h>
#include <compiler.h>
#include <console.h>
#include <printk.h>

Console *console = NULL;

int
probeconsole (Device *dev)
{
        return -1;
}

void
earlyconsole (EarlyConsole *ec)
{
        if (ec->console)
        {
                console = ec->console;
                printk ("using earlycon: %s\n", ec->name);
        }
}

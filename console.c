#include <types.h>
#include <console.h>
#include <printk.h>

Console *console = NULL;

void
kernelconsole (Console *cons)
{
  int rc = -1;

  if (cons && cons->init)
    rc = cons->init (cons);

  if (!rc) {
    console = cons;
    printk ("current console: %s\n", console->name);
  }
}

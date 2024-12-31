#include <types.h>
#include <compiler.h>
#include <printk.h>
#include <multiboot.h>
#include "arch.h"
#include "com.h"

void NORETURN INIT
x86main (MultiBootInfo *mb)
{
  serialportinit ();
  mb2parsebootinfo (mb);
  x86cpuinit ();

  for (;;)
    HLT;
}

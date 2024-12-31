#include <types.h>
#include <compiler.h>
#include <printk.h>
#include <multiboot.h>
#include <acpi.h>
#include "arch.h"
#include "com.h"
#include "mm.h"

void NORETURN INIT
x86main (MultiBootInfo *mb)
{
  killbootmap ();
  serialportinit ();
  mb2parsebootinfo (mb);
  // In x86-64, First 1MB is reserved
  sysrsrvmem (0x0, 0x100000);
  reservekernelarea ();
  x86cpuinit ();
  x86mminit ();
  initpic8259a ();
  acpiinit ();

  kernelmain ();
}

#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <printk.h>
#include <kalloc.h>
#include <timer.h>
#include <device.h>
#include <x86/arch.h>

void NORETURN
kernelmain (void)
{
  // Kernel early mapping is 0-1GiB
  kallocinitearly (0x0, 1 * GiB);
  kernelmap ();
  kallocinit ();

  devprobe ("timer");

  KDBG ("sleeptest\n");
  msleep (1000);
  KDBG ("1 ");
  msleep (1000);
  KDBG ("2 ");
  msleep (1000);
  KDBG ("3\n");

  for (;;)
    HLT;
  panic ("kernelmain exits");
}

#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <printk.h>
#include <kalloc.h>
#include <timer.h>
#include <device.h>
#include <proc.h>
#include <cpu.h>
#include <x86/arch.h>

void NORETURN
kernelmain (void)
{
  // Kernel early mapping is 0-1GiB
  kallocinitearly (0x0, 1 * GiB);
  kernelmap ();
  kallocinit ();
  regcpu (0);     // FIXME
  devprobe ("irqchip");
  devprobe ("timer");
  devprobe ("eventtimer");
  devprobe ("network");
  initprocess ();
  apmain ();
}

void NORETURN
apmain (void)
{
  mydevprobe ("cpu");
  mydevprobe ("irqchip");
  mydevprobe ("timer");
  mydevprobe ("eventtimer");
  initkernelproc ();

  INTR_ENABLE;

  for (;;)
    HLT;
  panic ("kernelmain exits");
}

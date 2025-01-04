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

int testtask1 (void *a);
int testtask2 (void *a);
int testtask3 (void *a);

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
  spawn ("kidle", NULL, idleprocess, NULL);
  spawn ("task1", NULL, testtask1, NULL);
  spawn ("task2", NULL, testtask2, NULL);
  spawn ("task3", NULL, testtask3, NULL);
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

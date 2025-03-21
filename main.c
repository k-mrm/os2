#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <printk.h>
#include <kalloc.h>
#include <timer.h>
#include <device.h>
#include <proc.h>
#include <cpu.h>
#include <pci.h>
#include <fs.h>
#include <syscall.h>
#include <x86/arch.h>
#include <symbol.h>
#include <module.h>

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
        ramdiskinit ();
        initmodule ();
        initfs ();
        initprocess ();
        syscallinit ();
        apmain ();
}

void NORETURN
apmain (void)
{
        mydevprobe ("cpu");
        mydevprobe ("irqchip");
        mydevprobe ("timer");
        mydevprobe ("eventtimer");

        lsdev ();

        int (*symprintk) (const char *fmt, ...);
        symprintk = S ("printk");
        symprintk ("hello from symbol\n");

        INTR_ENABLE;

        for (;;)
                HLT;
        panic ("kernelmain exits");
}

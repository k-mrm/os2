#include <types.h>
#include <cpu.h>
#include <device.h>

#define KPREFIX   "cpu:"

#include <printk.h>

static Cpu *_mycpu;

Cpu *
mycpu (void)
{
        return _mycpu;
}

static void
resumecpu (Device *dev)
{
        return;
}

static void
suspendcpu (Device *dev)
{
        return;
}

static int
probecpu (Device *dev)
{
        Cpu *cpu = (Cpu *)dev->priv;

        log ("%s: cpu %d probed!\n", dev->name, cpu->cpuid);

        if (cpu->cpuid != 0)
                panic ("mp not supported");
        _mycpu = cpu;
}

static Driver cpudrv = {
        .name         = "cpu",
        .description  = "cpu",
        .probe        = probecpu,
        .disconnect   = NULL,
        .reconnect    = NULL,
        .suspend      = suspendcpu,
        .resume       = resumecpu,
        .param        = "disable",
};

void
regcpu (int cpuid)
{
        char *a    = zalloc ();   // malloc
        Cpu  *cpu  = zalloc ();

        cpu->cpuid = cpuid;
        sprintf (a, "cpu%d", cpuid);
        regdevicecpu (cpuid, "cpu", a, NULL, &cpudrv, NULL, (DeviceStruct *)cpu);
}

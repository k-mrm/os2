#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <kalloc.h>
#include <mm.h>
#include "arch.h"
#include "apic.h"

#define KPREFIX     "xapic:"

#include <printk.h>

#define XAPIC_ID        0x020
#define XAPIC_VER       0x030
#define XAPIC_TPR       0x080
#define XAPIC_EOI       0x0b0
#define XAPIC_ESR       0x280
#define XAPIC_ICR_LOW   0x300
#define XAPIC_ICR_HIGH  0x310

static inline u32
xapicread32 (void *base, u32 off)
{
        return *(volatile u32 *)(base + off);
}

static inline void
xapicwrite32 (void *base, u32 off, u32 val)
{
        *(volatile u32 *)(base + off) = val;
}

static u32
xapicrd (Apic *apic, u32 reg)
{
        u32 offset = reg;
        return xapicread32 (apic->base, offset);
}

static void
xapicwr (Apic *apic, u32 reg, u32 val)
{
        u32 offset = reg;
        xapicwrite32 (apic->base, offset, val);
        // wait for completion
        xapicread32 (apic->base, XAPIC_ID);
}

static void
enxapic (void)
{
        ulong apicbase = rdmsr64 (IA32_APIC_BASE);

        if (apicbase & IA32_APIC_BASE_APIC_GLOBAL_ENABLE)
                return;

        apicbase |= IA32_APIC_BASE_APIC_GLOBAL_ENABLE;
        wrmsr64 (IA32_APIC_BASE, apicbase);
}

static void
xapicsendipi (Apic *apic, int id)
{
        ;
}

static u32
apicbaseaddr (void)
{
        ulong apicbase = rdmsr64 (IA32_APIC_BASE);
        return (apicbase & IA32_APIC_BASE_APIC_BASE_MASK);
}

static int
xapicprobe (Apic *apic)
{
        void *xapic;

        enxapic ();
        xapic = iomap (apicbaseaddr (), PAGESIZE);
        if (!xapic)
                return -1;

        apic->basepa  = apicbaseaddr ();
        apic->base    = xapic;
        return 0;
}

static ApicOps xapicops = {
        .probe    = xapicprobe,
        .read     = xapicrd,
        .write    = xapicwr,
        .sendipi  = xapicsendipi,
};

static bool
xapicsupported (void)
{
        u32 a, b, c, d;
        cpuid (CPUID_1, &a, &b, &c, &d);
        return !!(d & CPUID_1_EDX_APIC);
}

void INIT
xapicinit (uint id)
{
        if (!xapicsupported ())
                return;
        log ("Kernel use xapic\n");
        apicinit (id, &xapicops);
}

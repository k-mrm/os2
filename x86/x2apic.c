#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <kalloc.h>
#include <mm.h>
#include "arch.h"
#include "apic.h"

#define KPREFIX     "x2apic:"

#include <printk.h>

static void
x2apicsendipi (Apic *apic, int id)
{
  ;
}

static ApicOps x2apicops = {
  .probe    = NULL,
  .read     = NULL,
  .write    = NULL,
  .sendipi  = x2apicsendipi,
};

static bool
x2apicsupported (void)
{
  u32 a, b, c, d;
  cpuid (CPUID_1, &a, &b, &c, &d);
  return !!(c & CPUID_1_ECX_X2APIC);
}

void INIT
x2apicinit (uint id)
{
  if (!x2apicsupported ())
    return;
  KLOG ("Kernel use x2apic\n");
  apicinit (id, &x2apicops);
}

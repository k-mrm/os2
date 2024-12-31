#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <string.h>
#include <acpi.h>
#include <hpet.h>
#include <x86/mm.h>

#define KPREFIX     "acpi:"

#include <printk.h>

void *mb2xsdp (void) INIT;
void *mb2rsdp (void) INIT;

static int rsdpver;
static Rsdt *rsdt;
static Xsdt *xsdt;

#define FOREACH_RSDT(_rsdt, _e, _i)   \
  for (_e = rsdtnext((_rsdt), &(_i)); \
       _e;                            \
       _e = rsdtnext((_rsdt), &(_i)))

#define FOREACH_XSDT(_xsdt, _e, _i)   \
  for (_e = xsdtnext((_xsdt), &(_i)); \
       _e;                            \
       _e = xsdtnext((_xsdt), &(_i)))

static void *
rsdtnext (Rsdt *rsdt, uint *itr)
{
  SdtHeader *header = (SdtHeader *)rsdt;
  uint nentry;
  uint i = *itr;

  nentry = (header->length - sizeof (*header)) / sizeof (u32);
  if (i < nentry) {
    (*itr)++;
    return P2V (rsdt->entry[i]);
  } else {
    return NULL;
  }
}

static void *
xsdtnext (Xsdt *xsdt, uint *itr)
{
  SdtHeader *header = (SdtHeader *)xsdt;
  uint nentry;
  uint i = *itr;

  nentry = (header->length - sizeof (*header)) / sizeof (u64);
  if (i < nentry) {
    (*itr)++;
    return P2V (xsdt->entry[i]);
  } else {
    return NULL;
  }
}

static bool
rsdpcheck (Rsdp *rsdp)
{
  KDBG ("%s\n", rsdp->signature);
  return !strncmp (rsdp->signature, "RSD PTR ", 8);
}

static void *
findrsdt (char *sig)
{
  SdtHeader *ent;
  uint i = 0;

  FOREACH_RSDT (rsdt, ent, i) {
    if (!strncmp (ent->signature, sig, 4))
      return ent;
  }
  return NULL;
}

static void *
findxsdt (char *sig)
{
  SdtHeader *ent;
  uint i = 0;

  FOREACH_XSDT (xsdt, ent, i) {
    if (!strncmp (ent->signature, sig, 4))
      return ent;
  }
  return NULL;
}

static void
rsdtdump (void)
{
  SdtHeader *ent;
  uint i = 0;

  KLOG ("ACPI RSDT:\n");
  FOREACH_RSDT (rsdt, ent, i) {
    KLOG("\t%s (%s %s)\n", ent->signature,
         ent->oemid,
         ent->oemtableid);
  }
}

static void
xsdtdump (void)
{
  SdtHeader *ent;
  uint i = 0;

  KLOG ("ACPI XSDT:\n");
  FOREACH_XSDT (xsdt, ent, i) {
    KDBG("\t%s (%s %s)\n", ent->signature,
         ent->oemid,
         ent->oemtableid);
  }
}

static void
acpidump (void)
{
  if (rsdpver == 1)
    return rsdtdump ();
  else if (rsdpver == 2)
    return xsdtdump ();
}

static void *
acpifind (char *sig)
{
  if (rsdpver == 1)
    return findrsdt (sig);
  else if (rsdpver == 2)
    return findxsdt (sig);
  else
    return NULL;
}

static void INIT
acpihpetinit (void)
{
  HpetAcpi *hpet = acpifind ("HPET");

  if (!hpet)
    return;

  if (hpet->address.space != ACPI_SPACE_SYSTEM_MEMORY) {
    KLOG ("HPET address space is not memory\n");
    return;
  }
  if (!hpet->address.address) {
    KLOG ("HPET address is invalid\n");
    return;
  }
  KLOG ("HPET%d register @%p\n", hpet->number, hpet->address.address);
  hpetinit (hpet->address.address, hpet->number);
}

static void INIT
apicparseioapic (MadtIoapic *ioapic)
{
  KLOG ("IOAPIC%d at %p\n", ioapic->ioapicid, ioapic->ioapicaddr);
}

static void INIT
apicparselocalx2apic (MadtLocalx2apic *x2apic)
{
  KLOG ("x2apic Processor %d found %d %d\n",
        x2apic->x2apicid, x2apic->acpiid, x2apic->flags);
}

static void INIT
apicparselocalapic (MadtLocalapic *apic)
{
  if (!(apic->flags & 1))
    return;
  KLOG ("Processor %d(%d) found\n", apic->procid, apic->apicid);
}

static void INIT
acpimadtinit (void)
{
  Madt      *madt;
  MadtEntry *ent;
  ulong     len;

  madt = acpifind ("APIC");
  if (!madt)
    return;

  len = ((SdtHeader *)madt)->length;
  for (ent = madt->table;
       (ulong)ent < (ulong)madt + len;
       ent = (MadtEntry *)((ulong)ent + ent->length)) {
    switch (ent->type) {
      case APIC_TYPE_LOCALAPIC:
        apicparselocalapic ((MadtLocalapic *)ent);
        break;
      case APIC_TYPE_IOAPIC:
        apicparseioapic ((MadtIoapic *)ent);
        break;
      case APIC_TYPE_LOCAL_X2APIC:
        apicparselocalx2apic ((MadtLocalx2apic *)ent);
        break;
    }
  }
}

void INIT
acpiinit (void)
{
  Rsdp *rsdp;
  Xsdp *xsdp;

  rsdp = mb2xsdp ();
  if (rsdp && rsdpcheck (rsdp)) {
    rsdpver = 2;
    goto found;
  }
  rsdp = mb2rsdp ();
  if (rsdp && rsdpcheck (rsdp)) {
    rsdpver = 1;
    goto found;
  }
  panic ("no acpi table");

found:
  KLOG ("found acpi table version%d\n", rsdpver);
  KLOG ("acpi (%s %s) rsdt %p\n", rsdp->signature, rsdp->oemid, rsdp->rsdtaddress);

  if (rsdpver == 1) {
    rsdt = P2V (rsdp->rsdtaddress);
  } else if (rsdpver == 2) {
    xsdp = (Xsdp *)rsdp;
    xsdt = P2V (xsdp->xsdtaddress);
  }
  acpidump ();
  acpimadtinit ();
  acpihpetinit ();
}

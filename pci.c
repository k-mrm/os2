#include <types.h>
#include <compiler.h>
#include <pci.h>
#include <device.h>
#include <printk.h>
#include <x86/pci.h>

static int
pcidevprobe (Device *dev)
{
        /*
        PciDev *pci = (PciDev *)dev->priv;

        return pci->driver->probe (pci);
        */
}

static void
pcidevsuspend (Device *dev)
{
        ;
}

static void
pcidevresume (Device *dev)
{
        ;
}

static Driver pcidriver = {
        .name         = "PCI",
        .description  = "PCI(e) generic driver",
        .probe        = pcidevprobe,
        .suspend      = pcidevsuspend,
        .resume       = pcidevresume,
        .param        = "disable",
};

static int
regpci ()
{
        ;
}

void INIT
initpci (void)
{
        int bn, dn, fn;
        u16 v = 0, d = 0;
        u8 headertype;

	for (bn = 0; bn < 256; bn++)
	for (dn = 0; dn < 32; dn++)
        for (fn = 0; fn < 8; fn++)
        {
                pcicfgread (bn, DEVFN (dn, fn), PCI_CONFIG_VENDOR_ID, sizeof v, &v);
                if (v == 0xffff)
                {
                        continue;
                }

                pcicfgread (bn, DEVFN (dn, fn), PCI_CONFIG_DEVICE_ID, sizeof d, &d);
                trace ("PCI Device: %x.%x:%x %x:%x\n", bn, dn, fn, v, d);

                pcicfgread (bn, DEVFN (dn, fn), PCI_CONFIG_HEADER_TYPE,
                            sizeof headertype, &headertype);

                if (fn == 0 && !(headertype & PCI_CONFIG_HEADER_TYPE_MULTIFUNCTION))
                {
                        break;
                }
        }
}

#include <types.h>
#include <compiler.h>
#include <pci.h>
#include <device.h>
#include <kalloc.h>
#include <x86/pci.h>

#define KPREFIX         "PCI:"

#include <printk.h>

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

static PciDev *
pcifind (u16 vendor, u16 device)
{
        ;
}

static int
pciloaddriver (PciDev *pci, PciDriver *drv)
{
        return -1;
}

static PciDriver *
pcigetdriver (PciDev *pci)
{
        ;
}

static char *
pciname (PciDev *pci, char *name)
{
        sprintf (name, "%02x:%02x.%02x", pci->bus, DEVNO (pci->devfn), FUNCNO (pci->devfn));
        return name;
}

static int
newpci (int bus, int devfn)
{
        PciDev *pci;
        Device *dev;
        char name[40] = {0};
        
        pci = alloc ();
        if (!pci)
                return -1;

        pci->bus        = bus;
        pci->devfn      = devfn;
        pci->vendorid   = pciread (pci, PCI_CONFIG_VENDOR_ID, 2);
        pci->deviceid   = pciread (pci, PCI_CONFIG_DEVICE_ID, 2);
        pci->hdrtype    = pciread (pci, PCI_CONFIG_HEADER_TYPE, 1);

        dev = regdevice ("PCI", pciname (pci, name), NULL, &pcidriver, NULL, pci);
        if (!dev)
        {
                return -1;
        }

        pci->device = dev;
        return 0;
}

void INIT
initpci (void)
{
        int bn, dn, fn;
        u16 v = 0;
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

                if (newpci (bn, DEVFN (dn, fn)) < 0)
                {
                        warn ("PCI? %02x:%02x:%02x\n", bn, dn, fn);
                }

                pcicfgread (bn, DEVFN (dn, fn), PCI_CONFIG_HEADER_TYPE,
                            sizeof headertype, &headertype);

                if (fn == 0 && !(headertype & PCI_CONFIG_HEADER_TYPE_MULTIFUNCTION))
                {
                        break;
                }
        }
}

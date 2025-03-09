#include <types.h>
#include <compiler.h>
#include <pci.h>
#include <device.h>
#include <printk.h>
#include <kalloc.h>
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

static PciDev *
pcifind (u16 vendor, u16 device)
{
        ;
}

static char *
pciname (PciDev *pci, char *name)
{
        sprintf (name, "%x:%x.%x", pci->bus, DEVNO (pci->devfn), FUNCNO (pci->devfn));
        return name;
}

static int
newpci (int bus, int devfn)
{
        PciDev *pci;
        char name[40] = {0};
        
        pci = alloc ();
        if (!pci)
                return -1;

        pci->bus        = bus;
        pci->devfn      = devfn;
        pci->vendorid   = pciread (pci, PCI_CONFIG_VENDOR_ID, 2);
        pci->deviceid   = pciread (pci, PCI_CONFIG_DEVICE_ID, 2);
        pci->hdrtype    = pciread (pci, PCI_CONFIG_HEADER_TYPE, 1);

        return regdevice ("PCI", pciname (pci, name), NULL, &pcidriver, NULL, (DeviceStruct*)pci);
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

                newpci (bn, DEVFN (dn, fn));

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

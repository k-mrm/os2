#include <types.h>
#include <compiler.h>
#include <pci.h>
#include <device.h>
#include <kalloc.h>
#include <module.h>
#include <x86/pci.h>

#define KPREFIX         "PCI:"

#include <printk.h>

List *pcidriver;
List *pcidevs;

static int
pcidevprobe (Device *dev)
{
        PciDev *pci = (PciDev *)dev->priv;

        if (pci->driver)
                return pci->driver->probe (pci);
        else
                return -1;
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

static Driver driver = {
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

static void
pcifinddriver (PciDev *pci)
{
        PciDriver *driver;
        PCI_ID *id;

        FOREACH (pcidriver, driver)
        {
                for (id = driver->id; id->vendor != 0; id++)
                {
                        if (pci->vendorid == id->vendor &&
                            pci->deviceid == id->device)
                        {
                                pci->driver = driver;
                                log ("found driver: %s\n", driver->name);
                                return;
                        }
                }
        }
}

int
newpcidriver (PciDriver *drv)
{
        PciDev *dev;
        PCI_ID *id;

        if (!pcidriver)
        {
                pcidriver = newlist ();
        }

        PUSH (pcidriver, drv);

        for (id = drv->id; id->vendor != 0; id++)
        {
                FOREACH (pcidevs, dev)
                {
                        if (dev->vendorid == id->vendor &&
                            dev->deviceid == id->device)
                        {
                                dev->driver = drv;
                                if (dev->driver->probe)
                                        dev->driver->probe (dev);
                        }
                }
        }

        return 0;
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
        
        pci = zalloc ();
        if (!pci)
                return -1;

        pci->bus        = bus;
        pci->devfn      = devfn;
        pci->vendorid   = pciread (pci, PCI_CONFIG_VENDOR_ID, 2);
        pci->deviceid   = pciread (pci, PCI_CONFIG_DEVICE_ID, 2);
        pci->hdrtype    = pciread (pci, PCI_CONFIG_HEADER_TYPE, 1);

        dev = regdevice ("PCI", pciname (pci, name), NULL, &driver, NULL, pci);
        if (!dev)
        {
                return -1;
        }
        pci->device = dev;

        PUSH (pcidevs, pci);

        pcifinddriver (pci);

        return 0;
}

static void
initpci (void)
{
        int bn, dn, fn;
        u16 v = 0;
        u8 headertype;

        if (!pcidriver)
        {
                pcidriver = newlist ();
        }
        pcidevs = newlist ();

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

        devprobe ("PCI");
}

MODULE_DECL pci = {
        .name           = "pci",
        .description    = "PCI(e) driver",
        .init           = initpci,
        .delete         = NULL,
};

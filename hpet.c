#include <types.h>
#include <compiler.h>
#include <timer.h>
#include <sysmem.h>
#include <mm.h>
#include "hpet.h"

#define KPREFIX   "HPET:"

#include <printk.h>

#define HPET_MMIO_SIZE    1024

#define HPET_ID           0x0
#define HPET_CLK_PERIOD   0x4
#define HPET_GCR          0x10
#define HPET_MCR          0xf0

typedef struct Hpet Hpet;
struct Hpet {
  void *base;
  Phys basepa; 

  bool cnt64;
  uint nchannel;
  uint periodfs;   // 10(^-15) s
};

static inline void
hpetwr32 (Hpet *hpet, ulong offset, u32 val)
{
  *(volatile u32 *)(hpet->base + offset) = val;
}

static inline u32
hpetrd32 (Hpet *hpet, ulong offset)
{
  return *(volatile u32 *)(hpet->base + offset);
}

static inline u64
hpetrd64 (Hpet *hpet, ulong offset)
{
  return *(volatile u64 *)(hpet->base + offset);
}

static void
hpetctrl (Hpet *hpet, bool en)
{
  u32 gcr = hpetrd32 (hpet, HPET_GCR);

  if (!!(gcr & 1) == en)
    return;

  gcr |= en ? 1 : 0;
  hpetwr32 (hpet, HPET_GCR, gcr);
}

static void
hpeton (Device *dev)
{
  Timer *tm = (Timer *)dev->priv;
  Hpet *hpet = tm->priv;
  hpetctrl (hpet, true);
}

static void
hpetoff (Device *dev)
{
  Timer *tm = (Timer *)dev->priv;
  Hpet *hpet = tm->priv;
  hpetctrl (hpet, false);
}

#define USEC2FSEC   1000000000
static ulong
hpetusec2period (Timer *tm, uint usec)
{
  Hpet *hpet = tm->priv;
  ulong fsec = (ulong)usec * USEC2FSEC;
  ulong period = fsec / hpet->periodfs;
  return period;
}

static ulong
hpetgetcnt (Hpet *hpet)
{
  return hpetrd64 (hpet, HPET_MCR);
}

static ulong
hpetgetcntraw (Timer *tm)
{
  return hpetgetcnt (tm->priv);
}

static bool
hpetdead (Hpet *hpet)
{
  u64 now, after;

  now = hpetgetcnt (hpet);
  for (int i = 0; i < 1000; i++)  // busy loop
    ;
  after = hpetgetcnt (hpet);
  return now >= after;
}

static int INIT
hpetprobe (Device *dev)
{
  Timer *tm = (Timer *)dev->priv;
  Hpet *hpet = tm->priv;
  u32 id, clkperiod;
  void *base;

  base = iomap (hpet->basepa, HPET_MMIO_SIZE);
  if (!base)
    return -1;
  hpet->base = base;

  hpetctrl (hpet, false);

  id        = hpetrd32 (hpet, HPET_ID);
  clkperiod = hpetrd32 (hpet, HPET_CLK_PERIOD);

  hpet->cnt64    = !!(id & (1 << 13));
  hpet->nchannel = (id >> 8) & 0x1f;
  hpet->periodfs = clkperiod;

  hpetctrl (hpet, true);
  KLOG ("%s: %d channel(s) clock period: %d ns %d bit counter\n",
        dev->name, hpet->nchannel, clkperiod / 1000000, hpet->cnt64 ? 64 : 32);
  KDBG ("HPET %p\n", hpetgetcnt (hpet));

  if (hpetdead (hpet)) {
    KLOG("%s: Dead HPET\n", dev->name);
    goto err;
  }
  return probetimer (dev);

err:
  hpetctrl (hpet, false);
  return -1;
}

static Driver hpetdriver = {
  .name         = "HPET",
  .description  = "HPET Timer Device Driver",
  .probe        = hpetprobe,
  .suspend      = hpetoff,
  .resume       = hpeton,
  .param        = "disable",
};

static Timer tmhpet = {
  .global       = true,
  .read         = hpetgetcntraw,
  .usec2period  = hpetusec2period,
};

int INIT
hpetinit (Phys baseaddr, int n)
{
  Hpet *hpet;

  sysrsrvmem (baseaddr, HPET_MMIO_SIZE);
  hpet = bootmemalloc (sizeof *hpet, _Alignof (*hpet));
  if (!hpet)
    return -1;

  hpet->basepa = baseaddr;
  tmhpet.priv  = hpet;

  return regdevice ("timer", "HPET", NULL, &hpetdriver, NULL, (DeviceStruct*)&tmhpet);
}

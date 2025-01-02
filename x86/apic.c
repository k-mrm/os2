#include <types.h>
#include <compiler.h>
#include <panic.h>
#include <timer.h>
#include <sysmem.h>
#include <irq.h>
#include <device.h>
#include <kalloc.h>
#include "arch.h"
#include "apic.h"

#define KPREFIX   "apic:"

#include <printk.h>

#define ID        0x020
#define VER       0x030
#define TPR       0x080
#define EOI       0x0b0
#define SPIV      0x0f0
#define SPIV_APIC_ENABLED (1 << 8)
#define ESR       0x280
#define ICR_LOW   0x300
#define ICR_HIGH  0x310
#define LVT_TIMER 0x320
#define LVT_TIMER_INT_MASK      (1 << 16)
#define LVT_TIMER_ONE_TIME      (0 << 17)
#define LVT_TIMER_PERIODIC      (1 << 17)
#define LVT_TIMER_TSC_DEADLINE  (2 << 17)

#define TM_INIT     0x380
#define TM_CURRENT  0x390
#define TM_DIV      0x3e0

typedef struct LapicTimer   LapicTimer;
struct LapicTimer {
  Apic *apic;
  uint freq;
  uint periodms;
};

static int
apictmfreq (LapicTimer *lt)
{
  Apic  *apic = lt->apic;
  u32   cnt, cnt2;

  cnt = apic->ops->read (apic, TM_CURRENT);
  msleep (1000);
  cnt2 = apic->ops->read (apic, TM_CURRENT);

  if (cnt <= cnt2) {
    KWARN ("lapic timer?");
    return -1;
  }

  lt->freq = cnt - cnt2;
  KDBG ("lapic timer %x -> %x freq:%d\n", cnt, cnt2, lt->freq);
  return 0;
}

static int
apictimerprobe (Device *device)
{
  EventTimer  *et   = device->priv; 
  LapicTimer  *lt   = et->priv;
  Apic        *apic = lt->apic;
  int         err;
  u32         lvt = 0;

  lvt |= LVT_TIMER_INT_MASK;
  lvt |= 0x40;

  apic->ops->write (apic, LVT_TIMER, lvt);
  // Enable Timer
  apic->ops->write (apic, TM_INIT, 0xffffffff);

  err = apictmfreq (lt);
  if (err)
    return -1;

  lvt = LVT_TIMER_PERIODIC;
  // lvt |= LVT_TIMER_INT_MASK;
  lvt |= 0x40;    // periodic mode, vector is 0x40
  
  KLOG ("new apictimer!! %d\n", apic->id);
  apic->ops->write (apic, LVT_TIMER, lvt);
  // test
  apic->ops->write (apic, TM_INIT, lt->freq * 3);

  err = newirq (device, 0x40, true, eventtimerirq);
  if (err) {
    KDBG ("no irq\n");
    return -1;
  }
  return 0;
}

static void
apictimeron (Device *device)
{
  EventTimer  *et   = device->priv;
  LapicTimer  *lt   = et->priv;
  Apic        *apic = lt->apic;
  u32 lvt;

  lvt = apic->ops->read (apic, LVT_TIMER);
  lvt &= ~LVT_TIMER_INT_MASK;   // MASK bit
  apic->ops->write (apic, LVT_TIMER, lvt);
}

static void
apictimeroff (Device *device)
{
  EventTimer  *et   = device->priv;
  LapicTimer  *lt   = et->priv;
  Apic        *apic = lt->apic;
  u32 lvt;

  lvt = apic->ops->read (apic, LVT_TIMER);
  lvt |= LVT_TIMER_INT_MASK;    // MASK bit
  apic->ops->write (apic, LVT_TIMER, lvt);
}

static int
apicack (Irq *irq)
{
  return -1;
}

static void
apiceoi (Irq *irq)
{
  ;
}

static uint
apictimergetperiod (EventTimer *et)
{
  LapicTimer *lt = et->priv;
  return lt->periodms;
}

static void
apictimersetperiod (EventTimer *et, uint ms)
{
  LapicTimer *lt = et->priv;
  lt->periodms = ms;
}

static int
apictimerirq (EventTimer *et, Irq *irq)
{
  KLOG ("APIC TIMER IRQ\n");
  return -1;
}

static EventTimer lapictimer = {
  .global     = true,
  .getperiod  = apictimergetperiod,
  .setperiod  = apictimersetperiod,
  .irqhandler = apictimerirq,
} PERCPU;

static Driver apictimerdriver = {
  .name         = "apictimer",
  .description  = "x86 LAPIC Timer Device Driver",
  .probe        = apictimerprobe,
  .resume       = apictimeron,
  .suspend      = apictimeroff,
  .param        = "disable",
};

static void
enableapic (Device *dev)
{
  IrqChip     *irqchip  = (IrqChip *)dev->priv;
  Apic        *apic     = (Apic *)irqchip->priv;
  u32         spiv;

  spiv = apic->ops->read (apic, SPIV);
  spiv |= SPIV_APIC_ENABLED;
  apic->ops->write (apic, SPIV, spiv);
}

static void
disableapic (Device *dev)
{
  IrqChip     *irqchip  = (IrqChip *)dev->priv;
  Apic        *apic     = (Apic *)irqchip->priv;
  u32         spiv;

  spiv = apic->ops->read (apic, SPIV);
  spiv &= ~SPIV_APIC_ENABLED;
  apic->ops->write (apic, SPIV, spiv);
}

static void INIT
setspiv (Apic *apic)
{
  u32 spiv;
  spiv = 0x20;    // sprious interrupt vector is 0x20
  apic->ops->write (apic, SPIV, spiv);
}

static int INIT
apicprobe (Device *dev)
{
  IrqChip     *irqchip  = (IrqChip *)dev->priv;
  Apic        *apic     = (Apic *)irqchip->priv;
  LapicTimer  *timer;

  if (apic->ops->probe (apic) < 0)
    return -1;

  // Set Sprious Interrupt Vector
  setspiv (apic);
  // Clear Error Status
  apic->ops->write (apic, ESR, 0);
  apic->ops->write (apic, TPR, 0);

  enableapic (dev);

  // timer = malloc (sizeof *timer);
  timer = alloc ();
  if (!timer) {
    KWARN("cannot initialize lapic timer\n");
    return -1;
  }

  timer->apic = apic;

  lapictimer.priv = timer;

  regdevicemycpu ("eventtimer", "LAPICTimer", NULL, &apictimerdriver, NULL, (DeviceStruct *)&lapictimer);
  return 0;
}

static Driver apicdrv = {
  .name           = "apic",
  .description    = "apic driver",
  .probe          = apicprobe,
  .suspend        = disableapic,
  .resume         = enableapic,
  .param          = "xapic,x2apic",
};

static IrqChip apicirqchip = {
  .ack        = apicack,
  .eoi        = apiceoi,
};

void INIT
apicinit (u32 id, ApicOps *ops)
{
  Apic *apic;
  if (id != 0)
    panic ("unimpl");

  apic = bootmemalloc (sizeof *apic, _Alignof (*apic));
  if (!apic)
    return;
  apic->ops = ops;
  apic->id  = id;
  apicirqchip.priv = apic;

  regdevicecpu (id, "irqchip", "localapic", NULL, &apicdrv, NULL, (DeviceStruct *)&apicirqchip);
}

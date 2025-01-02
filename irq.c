#include <types.h>
#include <compiler.h>
#include <irq.h>
#include <kalloc.h>

#define KPREFIX     "irq:"

#include <printk.h>

static IrqChip  *myirqchip;

static Irq irqs[256];
static int nirq = 0;

static Irq *
allocirq (void)
{
  return &irqs[nirq++];
}

static Irq *
getirq (int irqno)
{
  Irq *i;
  for (i = irqs; i < &irqs[nirq]; i++) {
    if (i->irqno == irqno)
      return i;
  }
  return NULL;
}

int
newirq (Device *dev, int irqno, bool priv, int (*handler) (Irq *irq))
{
  Irq *irq = allocirq ();
  if (!irq)
    return -1;
  if (irqno < 0)
    // TODO: Allocate new irqno
    return -1;

  if (priv)
    irq->chip = myirqchip;
  else
    irq->chip = myirqchip;

  irq->irqno    = irqno;
  irq->device   = dev;
  irq->handler  = handler;
  return 0;
}

int
handleirq (int irqno)
{
  Irq *irq = getirq (irqno);
  int ret;

  if (!irq)
    return -1;

  irq->chip->ack (irq);
  ret = irq->handler (irq);
  irq->chip->eoi (irq);

  return ret;
}

int
probeirqchip (Device *dev)
{
  IrqChip *irqchip = (IrqChip *)dev->priv;
  myirqchip = irqchip;
  KLOG ("using irqchip: %s\n", dev->name);
  return 0;
}

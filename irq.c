#include <types.h>
#include <compiler.h>
#include <irq.h>
#include <kalloc.h>

#define KPREFIX     "irq:"

#include <printk.h>

static Irq *
getirq (int irqno)
{
  // TODO
  return NULL;
}

int
newirq (Device *dev, int irqno, bool priv, int (*handler) (Irq *irq))
{
  Irq *irq = alloc ();   // TODO: malloc
  if (!irq)
    return -1;
  if (irqno < 0)
    // TODO: Allocate new irqno
    return -1;

  if (priv)
    irq->chip = NULL;
  else
    irq->chip = NULL;

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

void INIT
IrqInit(void)
{
}

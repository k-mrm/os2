#ifndef _IRQ_H
#define _IRQ_H

#include <types.h>
#include <compiler.h>
#include <device.h>

typedef struct Irq        Irq;
typedef struct IrqChip    IrqChip;

struct IrqChip {
  DEVICE_STRUCT;

  void    (*eoi) (Irq *irq);
  int     (*ack) (Irq *irq);
  void    *priv;
};

struct Irq {
  int     irqno;
  IrqChip *chip;
  Device  *device;
  int     (*handler) (Irq *irq);
  int     (*enable) (Irq *irq);
  int     (*disable) (Irq *irq);
};

int newirq (Device *dev, int irqno, bool priv, int (*handler) (Irq *irq));
int handleirq (int irqno);
int probeirqchip (Device *dev);

#endif  // _IRQ_H

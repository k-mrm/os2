#ifndef _BLOCK_H
#define _BLOCK_H

#include <types.h>
#include <compiler.h>
#include <device.h>
#include <irq.h>

typedef struct BlockDev   BlockDev;

struct BlockDev {
  DEVICE_STRUCT;
  void    *priv;

  int     (*read) (BlockDev *dev, void *buf, ulong bno, int size);
  int     (*write) (BlockDev *dev, void *buf, ulong bno, int size);
};

int probeblock (Device *dev);

#endif  // _BLOCK_H

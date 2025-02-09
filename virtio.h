#ifndef _VIRTIO_H
#define _VIRTIO_H

#include <types.h>
#include <compiler.h>
#include <pci.h>

typedef struct VirtioDevice   VirtioDevice;
typedef struct VirtioDeviceOp VirtioDeviceOp;

struct VirtioDeviceOp {
  void    (*notify) (VirtioDevice *dev, int qsel);
  void    (*setvq) (VirtioDevice *dev, VirtQ *vq);
};

struct VirtioDevice {
  PciDev    *pci;
  // mmio?

  void      *priv;

  VirtioDeviceOp  *op;
};

#endif  // _VIRTIO_H

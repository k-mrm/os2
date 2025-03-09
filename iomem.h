#ifndef _IOMEM_H
#define _IOMEM_H

#include <compiler.h>
#include <types.h>
#include <list.h>

typedef struct IOMEM    IOMEM;
typedef struct Device   Device;

struct IOMEM
{
        LIST (IOMEM);

        volatile void *base;
        Phys    pbase;
        uint    size;
};

IOMEM *iomap (Device *dev, Phys base, uint size);

#endif  // _IOMEM_H

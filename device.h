#ifndef _DEVICE_H
#define _DEVICE_H

#include <types.h>
#include <compiler.h>
#include <iomem.h>

typedef enum DeviceType     DeviceType;
typedef struct Driver       Driver;
typedef struct Device       Device;
typedef struct Bus          Bus;

struct Driver
{
        char    *name;
        char    *description;

        int     (*probe) (Device *);
        void    (*disconnect) (Device *);
        int     (*reconnect) (Device *);
        void    (*suspend) (Device *);
        void    (*resume) (Device *);

        char    *param;
};

struct Device
{
        Device  *parent;
        char    *type;
        char    name[40];

        List    *iomem;

        Driver  *driver;
        void    *priv;     // private device information
};

struct Bus
{
        ;
};

void devprobe (char *type);
void mydevprobe (char *type);
Device *regdevicemycpu (char *ty, char *name, Bus *bus, Driver *drv, Device *parent, void *priv);
Device *regdevicecpu (int id, char *ty, char *name, Bus *bus, Driver *drv, Device *parent, void *priv);
Device *regdevice (char *ty, char *name, Bus *bus, Driver *drv, Device *parent, void *priv);
int regbus (char *name, Bus *parent);

#endif  // _DEVICE_H

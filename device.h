#ifndef _DEVICE_H
#define _DEVICE_H

#include <types.h>
#include <compiler.h>

typedef enum DeviceType     DeviceType;
typedef struct Driver       Driver;
typedef struct Device       Device;
typedef struct Bus          Bus;
typedef struct DeviceStruct DeviceStruct;

struct Driver {
  char    *name;
  char    *description;

  int     (*probe)(Device *);
  void    (*disconnect)(Device *);
  int     (*reconnect)(Device *);
  void    (*suspend)(Device *);
  void    (*resume)(Device *);

  char    *param;
};

struct Device {
  Device      *parent;
  char        *type;
  char        *name;

  Bus         *bus;
  Driver      *driver;
  void        *priv;     // private device information

  // Log         *logger;
};

struct DeviceStruct {
  Device *device;
};

#define DEVICE_STRUCT       DeviceStruct device;

struct Bus {
  DEVICE_STRUCT;
};

int regdevice (char *ty, char *name, Bus *bus, Driver *drv, Device *parent, DeviceStruct *priv);
int regbus (char *name, Bus *parent);

#endif  // _DEVICE_H
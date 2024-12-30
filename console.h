#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <types.h>

typedef struct Console    Console;

struct Console {
  const char name[16];
  void *priv;

  int (*init) (Console *);
  void (*write) (Console *, const char *, uint);
  void (*read) (Console *, char *, uint);
};

extern Console *console;

void kernelconsole (Console *cs);

#endif  // _CONSOLE_H

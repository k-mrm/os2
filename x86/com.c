#include <types.h>
#include <console.h>
#include "arch.h"
#include "com.h"

#define COM1    0x3f8
#define COM2    0x2f8

#define DATA  0x0
#define IER   0x1
#define DLL   0x0
#define DLH   0x1
#define IIR   0x2
#define FCR   0x2
#define LCR   0x3
#define MCR   0x4
#define LSR   0x5
#define MSR   0x6

#define DLAB  0x80

typedef struct Com  Com;
struct Com {
  u16 port;
  u32 baud;
};

static Com com1 = {
  .port = COM1,
  .baud = 115200,
};

static int
cominit (Console *cs)
{
  Com *com = cs->priv;
  u16 port = com->port;
  u16 div = 115200 / com->baud;

  // Disable Interrupt
  outb (port + IER, 0x0);
  // Disable FIFO
  outb (port + FCR, 0x0);
  // Baudrate
  outb (port + LCR, 0x80);
  outb (port + DLL, div & 0xff);
  outb (port + DLH, (div >> 8) & 0xff);
  // RTS | DTR
  outb (port + MCR, 0x3);

  // 8n1
  outb (port + LCR, 0x3);

  return 0;
}

static bool
comempty(Com *com)
{
  return inb (com->port + LSR) & 0x20;
}

static void
comsend(Com *com, char c)
{
  while (comempty (com) == 0)
    ;

  outb(com->port + DATA, c);
}

static void
computc (Console *cs, char c)
{
  Com *com = cs->priv;
  if (c == '\n')
    comsend (com, '\r');
  comsend (com, c);
}

static void
comwrite (Console *cs, const char *buf, uint n)
{
  for (uint i = 0; i < n && buf[i]; i++) {
    computc (cs, buf[i]);
  }
}

static void
comread (Console *cs, char *buf, uint n)
{
  ;
}

static Console cons = {
  .name = "COM",
  .priv = &com1,
  .init = cominit,
  .write = comwrite,
  .read = comread,
};

void
serialportinit (void)
{
  kernelconsole (&cons);
}

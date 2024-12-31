#include <types.h>
#include <printk.h>
#include <panic.h>
#include "x86/arch.h"

void NORETURN
panic (char *msg, ...)
{
  va_list ap;
  char buf[256] = {0};
  int n;

  INTR_DISABLE;

  va_start (ap, msg);
  n = vsprintf (buf, msg, ap);
  va_end (ap);

  printk ("kernel panic: %s\n", buf);

  for (;;)
    HLT;
}

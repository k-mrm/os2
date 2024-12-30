#ifndef _PRINTK_H
#define _PRINTK_H

#include "compiler.h"

int printk (const char *fmt, ...);
int sprintf (char *buf, const char *fmt, ...);
int vsprintf (char *buf, const char *fmt, va_list ap);

#ifndef KPREFIX
#define KPREFIX
#endif  // KPREFIX

#define KDBG(fmt, ...)  printk (KPREFIX " [debug] " fmt, ##__VA_ARGS__)
#define KWARN(fmt, ...) printk (KPREFIX " [warning] " fmt, ##__VA_ARGS__)
#define KLOG(fmt, ...)  printk (KPREFIX " " fmt, ##__VA_ARGS__)

#endif  // _PRINTK_H

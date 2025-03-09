#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <compiler.h>
#include <types.h>

void *sym2a (const char *name);
const char *a2sym (u64 addr, u64 *off);

#define S(_a)    sym2a(_a)

#endif  // _SYMBOL_H

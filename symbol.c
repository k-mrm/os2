#include <compiler.h>
#include <types.h>
#include <string.h>
#include <symbol.h>

typedef struct Symbol     Symbol;

struct Symbol {
  u64         addr;
  const char  *name;
};

static Symbol symbols[] = {
#include "symbol.inc.h"
  {0, NULL},
};

void *
sym2a (const char *name)
{
  for (Symbol *sym = symbols; sym->name; sym++) {
    if (strcmp (name, sym->name) == 0)
      return (void *)sym->addr;
  }
  return NULL;
}

const char *
a2sym (u64 addr, u64 *off)
{
  // TODO
  return NULL;
}

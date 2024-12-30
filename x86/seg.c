#include "types.h"
#include "compiler.h"
#include "arch.h"

const ulong gdt[] = {
  0x0,                // NULL
  0x00cf9b000000ffff, // KCODE32
  0x00cf93000000ffff, // KDATA32
  0x00af9b000000ffff, // KCODE64
  0x00af93000000ffff, // KDATA64
};

static inline void
loadgdt (const ulong *gdt, ulong gdtsize)
{
  volatile u16 t[5];

  t[0] = (u16)gdtsize - 1;
  t[1] = (u16)(ulong)gdt;
  t[2] = (u16)((ulong)gdt >> 16);
  t[3] = (u16)((ulong)gdt >> 32);
  t[4] = (u16)((ulong)gdt >> 48);

  asm volatile ("lgdt (%0)" :: "r"(t));
}

void INIT
gdtinit (void)
{
  // configgdt ();

  loadgdt (gdt, sizeof gdt);
}

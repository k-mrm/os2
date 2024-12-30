#include <types.h>
#include <compiler.h>
#include <printk.h>
#include <multiboot.h>

static void *xsdp = NULL;
static void *rsdp = NULL;

void * INIT
mb2xsdp(void)
{
  return xsdp;
}

void * INIT
mb2rsdp(void)
{
  return rsdp;
}

void INIT
mb2parsebootinfo(MultiBootInfo *mb)
{
  MultiBootTag *tag;

  if (!mb)
    return;
  for (tag = (MultiBootTag *)((char *)mb + 8);
       tag->type != MULTIBOOT_TAG_TYPE_END;
       tag = (MultiBootTag *)((char *)tag + ((tag->size + 7) & ~7))) {
    switch (tag->type) {
      case MULTIBOOT_TAG_TYPE_CMDLINE: {
        MultiBootTagString *cmd = (MultiBootTagString *)tag;
        KLOG ("Kernel Parameter: %s\n", cmd->string);
        break;
      }
      case MULTIBOOT_TAG_TYPE_BOOTDEV: {
        MultiBootTagBootdev *dev = (MultiBootTagBootdev *)tag;
        KDBG ("Boot dev: 0x%x\n", dev->biosdev);
        break;
      }
      case MULTIBOOT_TAG_TYPE_MMAP: {
        MultiBootMmapEntry *e;
        MultiBootTagMmap *mmap = (MultiBootTagMmap *)tag;

        for (e = mmap->entries;
            (char *)e < (char *)mmap + mmap->size;
            e = (MultiBootMmapEntry *)((char *)e + mmap->entrysize)) {
          /*
          if (e->type == MULTIBOOT_MEMORY_AVAILABLE)
            NewMem(e->addr, e->len);
          else if (e->type == MULTIBOOT_MEMORY_RESERVED)
            ReserveMem(e->addr, e->len);
            */
        }
        break;
      }
      case MULTIBOOT_TAG_TYPE_ACPI_OLD: {
        MultiBootTagAcpiOld *acpi = (MultiBootTagAcpiOld *)tag;
        rsdp = &acpi->rsdp;
        break;
      }
      case MULTIBOOT_TAG_TYPE_ACPI_NEW: {
        MultiBootTagAcpiNew *acpi = (MultiBootTagAcpiNew *)tag;
        xsdp = &acpi->xsdp;
        break;
      }
      default:
        break;
    }
  }
}


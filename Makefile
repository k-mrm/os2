CONFIG ?= .config

include $(CONFIG)

QEMUPREFIX ?=
QEMU = $(QEMUPREFIX)qemu-system-x86_64

PREFIX = 
CC = $(PREFIX)gcc
AS = $(PREFIX)as
LD = $(PREFIX)ld
OBJCOPY = $(PREFIX)objcopy

CFLAGS := -Wall -Og -g -MD -ffreestanding -nostdinc -nostdlib -nostartfiles
CFLAGS += -I ./
LDFLAGS := -g -nostdlib -nostartfiles -Wl,--build-id=none -static --no-relax

NAME = kernel
elf = $(NAME).elf
fakeelf = __$(NAME).elf
fakeelf2 = __$(NAME)2.elf
img = $(NAME).img
iso = $(NAME).iso
map = $(NAME).map

NCPU ?= 1
MEMSZ ?= 512

ARCH ?= x86

# Kernel objects (arch-specific)
objs-x86 += x86/entry.o
objs-x86 += x86/com.o x86/main.o x86/seg.o
objs-x86 += x86/cpu.o x86/pic-8259a.o x86/mm.o
objs-x86 += x86/trap-handler.o x86/trap.o
objs-x86 += x86/apic.o x86/xapic.o x86/x2apic.o
objs-x86 += x86/cswitch.o

# Kernel objects
objs-1 += printk.o string.o main.o console.o
objs-1 += hpet.o sysmem.o panic.o
objs-1 += multiboot.o acpi.o device.o timer.o
objs-1 += mm.o kalloc.o irq.o proc.o cpu.o symbol.o

.SUFFIXES : .c .S .o

symbol.inc.h:
	@touch symbol.inc.h

%.o: %.c
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(fakeelf): symbol.inc.h $(ARCH)/link.ld $(objs-1) $(objs-$(ARCH)) $(CONFIG)
	@echo LD $@
	@$(LD) -n -Map $(map) --no-relax -T $(ARCH)/link.ld -o $@ $(objs-1) $(objs-$(ARCH))

symbol: $(fakeelf)
	@echo Generate Symbols...
	@nm -a $(fakeelf) | grep ' T ' | sort | awk '{printf("{0x%s,\"%s\"},\n", $$1, $$3)}' > symbol.inc.h

$(elf): symbol $(ARCH)/link.ld $(objs-1) $(objs-$(ARCH)) $(CONFIG)
	@echo LD $@
	@$(LD) -n -Map $(map) --no-relax -T $(ARCH)/link.ld -o $@ $(objs-1) $(objs-$(ARCH))

elf: $(elf)
	@touch symbol.c
	@make $(elf)

iso: $(iso)

$(iso): elf grub.cfg
	@mkdir -p iso/boot/grub
	@cp grub.cfg iso/boot/grub
	@cp $(elf) iso/boot/
	@grub-mkrescue -o $@ iso/

clean:
	$(RM) $(objs-1) $(objs-$(ARCH)) $(elf) $(fakeelf) $(iso) $(img) $(map) symbol.inc.h
	$(RM) -rf iso/

#qemu-img: $(img)
#	$(QEMU) -nographic -drive file=$(img),index=0,media=disk,format=raw -smp $(NCPU) -m $(MEMSZ)

qemu-iso: $(iso)
	$(QEMU) -nographic -drive file=$(iso),format=raw -serial mon:stdio -smp $(NCPU) -m $(MEMSZ)

qemu-gdb: $(iso)
	$(QEMU) -nographic -drive file=$(iso),format=raw -serial mon:stdio -smp $(NCPU) -m $(MEMSZ) -S -gdb tcp::1234

.PHONY: symbol clean elf iso qemu-iso qemu-gdb

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
LDFLAGS := -g -nostdlib -nostartfiles -Wl,--build-id=none -static

NAME = kernel
elf = $(NAME).elf
img = $(NAME).img
iso = $(NAME).iso
map = $(NAME).map

NCPU ?= 4
MEMSZ ?= 512

ARCH ?= x86

# Kernel objects (arch-specific)
objs-1 += $(ARCH)/entry.o
objs-1 += $(ARCH)/com.o $(ARCH)/main.o $(ARCH)/seg.o

# Kernel objects
objs-1 += printk.o string.o main.o console.o
objs-1 += hpet.o
objs-1 += multiboot.o

.SUFFIXES : .c .S .o

%.o: %.c
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	@echo CC $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(elf): $(ARCH)/link.ld $(objs-1) $(CONFIG)
	@$(LD) -n -Map $(map) -T $(ARCH)/link.ld -o $@ $(objs-1)

iso: $(iso)

$(iso): $(elf) grub.cfg
	@mkdir -p iso/boot/grub
	@cp grub.cfg iso/boot/grub
	@cp $(elf) iso/boot/
	@grub-mkrescue -o $@ iso/

clean:
	$(RM) $(objs-1) $(elf) $(iso) $(img) $(map)
	$(RM) -rf iso/

#qemu-img: $(img)
#	$(QEMU) -nographic -drive file=$(img),index=0,media=disk,format=raw -smp $(NCPU) -m $(MEMSZ)

qemu-iso: $(iso)
	$(QEMU) -nographic -drive file=$(iso),format=raw -serial mon:stdio -smp $(NCPU) -m $(MEMSZ)

qemu-gdb: $(iso)
	$(QEMU) -nographic -drive file=$(iso),format=raw -serial mon:stdio -smp $(NCPU) -m $(MEMSZ) -S -gdb tcp::1234

.PHONY: clean iso qemu-iso qemu-gdb

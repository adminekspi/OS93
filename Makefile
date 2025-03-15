HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

OUTPUT_KERNEL = kernel.bin
OUTPUT_IMG = os93.img

CC = i686-linux-gnu-gcc
CFLAGS = -std=gnu17 -ffreestanding -Wall -Wextra -Werror -nostdinc -nostdlib -fno-exceptions -fno-pie

LD = i686-linux-gnu-ld
LDFLAGS = -T linker.ld -o $(OUTPUT_KERNEL) --allow-multiple-definition

OBJECT_KERNEL = kernel.o
OBJECTS = $(SOURCES:.c=.o)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	nasm -f elf32 kernel.asm -o $(OBJECT_KERNEL)
	$(LD) $(LDFLAGS) $(OBJECT_KERNEL) $(OBJECTS)

	dd if=/dev/zero of=$(OUTPUT_IMG) bs=1M count=128
	parted $(OUTPUT_IMG) --script mklabel msdos
	parted $(OUTPUT_IMG) --script mkpart primary 0% 100%
	parted $(OUTPUT_IMG) --script set 1 boot on

	LOOP=$$(losetup -f); \
	losetup -P $$LOOP $(OUTPUT_IMG); \
	mkfs.msdos $${LOOP}p1; \
	mkdir -p mnt; \
	mount $${LOOP}p1 mnt; \
	cp $(OUTPUT_KERNEL) mnt/; \
	cp syslinux.cfg mnt/; \
	cp /usr/lib/syslinux/modules/bios/ldlinux.c32 mnt/; \
	cp /usr/lib/syslinux/modules/bios/libcom32.c32 mnt/; \
	cp /usr/lib/syslinux/modules/bios/libutil.c32 mnt/; \
	cp /usr/lib/syslinux/modules/bios/mboot.c32 mnt/; \
	umount mnt; \
	syslinux --install $${LOOP}p1; \
	losetup -d $$LOOP

run: all
	qemu-system-i386 -m 128M -drive file=$(OUTPUT_IMG),format=raw -vga std

clean:
	rm -rf *.bin *.o *.img mnt/

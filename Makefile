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
	mkfs.vfat -F 16 $(OUTPUT_IMG)

	mkdir -p mnt
	sudo mount -o loop $(OUTPUT_IMG) mnt
	sudo cp $(OUTPUT_KERNEL) syslinux.cfg mnt/
	sudo cp /usr/lib/syslinux/modules/bios/mboot.c32 mnt/
	sudo cp /usr/lib/syslinux/modules/bios/libcom32.c32 mnt/
	sudo cp /usr/lib/syslinux/modules/bios/libutil.c32 mnt/
	sudo umount mnt

	syslinux --install $(OUTPUT_IMG)

run: all
	qemu-system-i386 -m 128M -drive file=$(OUTPUT_IMG),format=raw -vga std

clean:
	sudo rm -rf *.bin *.o *.img mnt/

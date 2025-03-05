HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

CC = i686-linux-gnu-gcc
CFLAGS = -std=gnu17 -ffreestanding -Wall -Wextra -Werror -nostdinc -nostdlib -fno-exceptions -fno-pie

LD = i686-linux-gnu-ld
LDFLAGS = -T linker.ld -o kernel.bin --oformat binary --allow-multiple-definition

OBJECT_KERNEL = kernel.o
OBJECTS = $(SOURCES:.c=.o)

OUTPUT_BOOT = boot.bin
OUTPUT_KERNEL = kernel.bin
OUTPUT_IMG = os93.img

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	nasm -f bin boot.asm -o $(OUTPUT_BOOT)
	nasm -f elf kernel.asm -o $(OBJECT_KERNEL)
	$(LD) $(LDFLAGS) $(OBJECT_KERNEL) $(OBJECTS)

	# Create and format a 128 MB FAT16 image:
	# Using the -C option to create the file with 131072 blocks (131072 * 1024 = 134217728 bytes).
	# -F 16 : Select FAT16 format
	# -s 4 : 4 sectors per cluster (cluster size 2048 bytes)
	# -r 224 : 224 entries for the root directory
	# -n OS93 : Set the volume label to OS93
	# -I : Disable safety checks (for formatting on a file)
	mkfs.vfat -F 16 -s 4 -r 224 -n OS93 -I -C $(OUTPUT_IMG) 131072

	dd if=$(OUTPUT_BOOT) of=$(OUTPUT_IMG) bs=1 count=448 seek=62 conv=notrunc
	mcopy -i $(OUTPUT_IMG) $(OUTPUT_KERNEL) ::kernel.bin

run: all
	qemu-system-i386 -drive file=$(OUTPUT_IMG),format=raw -vga std

clean:
	rm -f *.bin *.o $(OUTPUT_IMG)

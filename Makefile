HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)

OUTPUT_BOOT = boot.bin
OUTPUT_KERNEL = kernel.bin
OUTPUT_IMG = os93.img

CC = i686-linux-gnu-gcc
CFLAGS = -std=gnu17 -ffreestanding -Wall -Wextra -Werror -nostdinc -nostdlib -fno-exceptions -fno-pie

LD = i686-linux-gnu-ld
LDFLAGS = -T linker.ld -o $(OUTPUT_KERNEL) --oformat binary --allow-multiple-definition

OBJECT_KERNEL = kernel.o
OBJECTS = $(SOURCES:.c=.o)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(OBJECTS)
	nasm -f bin boot.asm -o $(OUTPUT_BOOT)
	nasm -f elf kernel.asm -o $(OBJECT_KERNEL)
	$(LD) $(LDFLAGS) $(OBJECT_KERNEL) $(OBJECTS)

	mkfs.fat -F 16 -R 4 -s 4 -S 512 -r 512 -n OS93 -I -v -C $(OUTPUT_IMG) 131072

	dd if=$(OUTPUT_BOOT) of=$(OUTPUT_IMG) bs=512 count=1 conv=notrunc
	mcopy -v -i $(OUTPUT_IMG) $(OUTPUT_KERNEL) "::KERNEL.BIN"

run: all
	qemu-system-i386 -drive file=$(OUTPUT_IMG),format=raw -vga std

clean:
	rm -f *.bin *.o $(OUTPUT_IMG)

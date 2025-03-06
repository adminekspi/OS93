#include "ports.h"
#include "vga.h"

void write_regs(unsigned char *regs)
{
    unsigned int i;

    // Write Miscellaneous Register
    outb(VGA_MISC_WRITE, *regs);
    regs++;

    // Unlock CRTC registers
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);

    // Write CRTC Registers
    for (i = 0; i < 25; i++)
    {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, *regs++);
    }

    // Write Graphics Controller Registers
    for (i = 0; i < 9; i++)
    {
        outb(VGA_GRAPHICS_INDEX, i);
        outb(VGA_GRAPHICS_DATA, *regs++);
    }

    // Write Attribute Controller Registers
    for (i = 0; i < 21; i++)
    {
        (void)inb(VGA_ATTR_RESET);
        outb(VGA_ATTR_WRITE, i);
        outb(VGA_ATTR_WRITE, *regs++);
    }

    // Enable palette
    (void)inb(VGA_ATTR_RESET);
    outb(VGA_ATTR_WRITE, 0x20);
}

void clear_screen_vga(void)
{
    unsigned int i = 0;

    // this loop clears the screen
    for (i = 1; i < 640 * 480; i++)
    {
        VGA_MODE_PTR[i] = 0;
    }
}

void putpixel(int x, int y, unsigned char color)
{
    unsigned char mask = 0x80 >> (x & 7);
    unsigned int offset = (y * 80) + (x / 8);

    for (int plane = 0; plane < 4; plane++) {
        outb(VGA_GRAPHICS_INDEX, 4);  // select plane register
        outb(VGA_GRAPHICS_DATA, plane);
        if (color & (1 << plane))
            VGA_MODE_PTR[offset] |= mask;
        else
            VGA_MODE_PTR[offset] &= ~mask;
    }
}

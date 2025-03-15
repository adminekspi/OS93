#include "ports.h"
#include "vga.h"

void switch_to_vga_12h_mode(unsigned char * regs_values)
{
    int index = 0;
    int i;

    // Write miscellaneous output register
    outb(VGA_MISC_WRITE, regs_values[index++]);

    // Write sequencer registers
    for (i = 0; i < NUM_SEQ; i++)
    {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, regs_values[index++]);
    }

    // Unlock CRTC registers 0x03 and 0x11
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);

    regs_values[0x03] |= 0x80;
    regs_values[0x11] &= ~0x80;

    // Write CRTC registers
    for (i = 0; i < NUM_CRTC; i++)
    {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, regs_values[index++]);
    }

    // Write graphics controller registers
    for (i = 0; i < NUM_GC; i++)
    {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, regs_values[index++]);
    }

    // Write attribute controller registers
    for (i = 0; i < NUM_AC; i++)
    {
        (void)inb(VGA_INSTAT_READ); // Reset flip-flop before writing each register
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_INDEX, regs_values[index++]); // Same port!
    }
    
    // Finalize attribute controller write
    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

void clear_screen_vga_12h_mode(void)
{
    volatile unsigned char * vga = (volatile unsigned char *)VGA_MODE_PTR;
    int plane, i;
    int plane_size = 80 * 480; // Each plane: 80 bytes per scanline (640/8) * 480 scanlines

    for (plane = 0; plane < 4; plane++)
    {
        // Select the current plane by writing to the Sequencer Map Mask Register (index 2)
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, 1 << plane);
        
        // Clear the current plane by setting all bytes to 0
        for (i = 0; i < plane_size; i++)
        {
            vga[i] = 0;
        }
    }
    
    // Restore writing to all planes
    outb(VGA_SEQ_INDEX, 2);
    outb(VGA_SEQ_DATA, 0x0F);
}

void put_pixel_vga_12h_mode(int x, int y, unsigned char color)
{
    volatile unsigned char * vga = (volatile unsigned char *)VGA_MODE_PTR;
    int plane;
    int index = y * 80 + (x >> 3);
    unsigned char bitmask = 0x80 >> (x & 7);
    unsigned char current;

    for (plane = 0; plane < 4; plane++)
    {
        // Select the current plane (bit in map mask)
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, 1 << plane);

        // Read current byte and update the specific bit for this plane
        current = vga[index];
        if ((color >> plane) & 1)
        {
            current |= bitmask;
        }
        else
        {
            current &= ~bitmask;
        }
        vga[index] = current;
    }
}

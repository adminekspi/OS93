#include "vga.h"
#include "ports.h"

void switch_to_vga_12h_mode(unsigned char * regs_values)
{
    int index = 0;
    int i;

    /* Write Miscellaneous Output Register */
    outb(VGA_MISC_WRITE, regs_values[index++]);

    /* Write Sequencer Registers */
    for (i = 0; i < NUM_SEQ; i++) {
        outb(VGA_SEQ_INDEX, i);
        outb(VGA_SEQ_DATA, regs_values[index++]);
    }

    /* Unlock CRTC registers 0x03 and 0x11 */
    outb(VGA_CRTC_INDEX, 0x03);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) | 0x80);
    outb(VGA_CRTC_INDEX, 0x11);
    outb(VGA_CRTC_DATA, inb(VGA_CRTC_DATA) & ~0x80);

    regs_values[0x03] |= 0x80;
    regs_values[0x11] &= ~0x80;

    /* Write CRT Controller Registers */
    /* Some registers (like 0x03 and 0x11) might require unlocking. */
    for (i = 0; i < NUM_CRTC; i++) {
        outb(VGA_CRTC_INDEX, i);
        outb(VGA_CRTC_DATA, regs_values[index++]);
    }

    /* Write Graphics Controller Registers */
    for (i = 0; i < NUM_GC; i++) {
        outb(VGA_GC_INDEX, i);
        outb(VGA_GC_DATA, regs_values[index++]);
    }

    /* Write Attribute Controller Registers */
    for (i = 0; i < NUM_AC; i++) {
        (void)inb(VGA_INSTAT_READ); // Reset flip-flop before writing each register
        outb(VGA_AC_INDEX, i);
        outb(VGA_AC_INDEX, regs_values[index++]); // same port!
    }
    
    /* Finalize Attribute Controller write */
    (void)inb(VGA_INSTAT_READ);
    outb(VGA_AC_INDEX, 0x20);
}

void clear_screen_vga_12h_mode(void)
{
    volatile unsigned char * vga = (volatile unsigned char *)VGA_MODE_PTR;
    int plane, i;
    int plane_size = 80 * 480; // Each plane: 80 bytes per scanline (640/8) * 480 scanlines

    for (plane = 0; plane < 4; plane++) {
        // Select the current plane by writing to the Sequencer Map Mask Register (index 2)
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, 1 << plane);
        
        // Clear the current plane by setting all bytes to 0
        for (i = 0; i < plane_size; i++) {
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

    for (plane = 0; plane < 4; plane++) {
        // Select the current plane (bit in map mask)
        outb(VGA_SEQ_INDEX, 2);
        outb(VGA_SEQ_DATA, 1 << plane);

        // Read current byte and update the specific bit for this plane
        current = vga[index];
        if ((color >> plane) & 1)
            current |= bitmask;
        else
            current &= ~bitmask;
        vga[index] = current;
    }
}

void get_font_vga_12h_mode(void)
{
    unsigned int vga_ptr = (unsigned int)VGA_MODE_PTR;

    // Read font data from VGA memory
    outw(VGA_GC_INDEX, 0x0005);
    outw(VGA_GC_INDEX, 0x0406);
    outw(VGA_SEQ_INDEX, 0x0402);
    outw(VGA_SEQ_INDEX, 0x0604);
    outd(VGA_SEQ_INDEX, 0x000A0000);

    // Read and copy font data
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 16; j++)
        {
            vga_font[i][j] = inb(vga_ptr);
            vga_ptr++;
        }

        // Skip the 16-byte gap between characters
        for (int j = 0; j < 16; j++)
        {
            vga_ptr++;
        }
    }

    // Restore default VGA registers
    outw(VGA_SEQ_INDEX, 0x0302);
    outw(VGA_SEQ_INDEX, 0x0204);
    outw(VGA_GC_INDEX, 0x1005);
    outw(VGA_GC_INDEX, 0x0E06);
}

void draw_char_vga_12h_mode(unsigned char c, int x, int y, int color_fg, int color_bg)
{
    int mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    unsigned char * glyph = (unsigned char *)(vga_font + (short)(c * 16));

    for(int cy=0; cy<16; cy++)
    {
        for(int cx = 0; cx < 8; cx++){
            put_pixel_vga_12h_mode(glyph[cy] & mask[cx] ? color_fg : color_bg , x + cx, y + cy - 12);
        }
    }
}

#include "math.h"
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

void draw_line_vga_12h_mode(int x0, int y0, int x1, int y1, unsigned char color)
{
    // Draw a line using Bresenham's line algorithm

    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    int e2 = 0;

    while (1)
    {
        if (x0 >= 0 && x0 < VGA_12H_WIDTH && y0 >= 0 && y0 < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0, y0, color);
        }

        if (x0 == x1 && y0 == y1)
        {
            break;
        }

        e2 = 2 * err;
        
        if (e2 > -dy)
        {
            err = err - dy;
            x0 = x0 + sx;
        }

        if (e2 < dx)
        {
            err = err + dx;
            y0 = y0 + sy;
        }
    }
}

// Draw a rectangle outline
void draw_rectangle_vga_12h_mode(int x, int y, int width, int height, unsigned char color)
{
    // Check bounds
    if (x < 0 || y < 0 || x + width > VGA_12H_WIDTH || y + height > VGA_12H_HEIGHT)
    {
        return;
    }

    // Draw the four lines
    draw_line_vga_12h_mode(x, y, x + width - 1, y, color);                           // Top
    draw_line_vga_12h_mode(x, y + height - 1, x + width - 1, y + height - 1, color); // Bottom
    draw_line_vga_12h_mode(x, y, x, y + height - 1, color);                          // Left
    draw_line_vga_12h_mode(x + width - 1, y, x + width - 1, y + height - 1, color);  // Right
}

// Draw a filled rectangle
void draw_filled_rectangle_vga_12h_mode(int x, int y, int width, int height, unsigned char color)
{
    int i = 0;
    int j = 0;

    // Check bounds
    if (x < 0 || y < 0 || x + width > VGA_12H_WIDTH || y + height > VGA_12H_HEIGHT)
    {
        return;
    }

    for (j = y; j < y + height; j++)
    {
        for (i = x; i < x + width; i++)
        {
            put_pixel_vga_12h_mode(i, j, color);
        }
    }
}

// Draw a circle using the midpoint circle algorithm
void draw_circle_vga_12h_mode(int x0, int y0, int radius, unsigned char color)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        // Draw 8 octants
        if (x0 + x >= 0 && x0 + x < VGA_12H_WIDTH && y0 + y >= 0 && y0 + y < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 + x, y0 + y, color);
        }
            
        if (x0 + y >= 0 && x0 + y < VGA_12H_WIDTH && y0 + x >= 0 && y0 + x < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 + y, y0 + x, color);
        }
            
        if (x0 - y >= 0 && x0 - y < VGA_12H_WIDTH && y0 + x >= 0 && y0 + x < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 - y, y0 + x, color);
        }
            
        if (x0 - x >= 0 && x0 - x < VGA_12H_WIDTH && y0 + y >= 0 && y0 + y < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 - x, y0 + y, color);
        }
            
        if (x0 - x >= 0 && x0 - x < VGA_12H_WIDTH && y0 - y >= 0 && y0 - y < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 - x, y0 - y, color);
        }
            
        if (x0 - y >= 0 && x0 - y < VGA_12H_WIDTH && y0 - x >= 0 && y0 - x < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 - y, y0 - x, color);
        }
            
        if (x0 + y >= 0 && x0 + y < VGA_12H_WIDTH && y0 - x >= 0 && y0 - x < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 + y, y0 - x, color);
        }
            
        if (x0 + x >= 0 && x0 + x < VGA_12H_WIDTH && y0 - y >= 0 && y0 - y < VGA_12H_HEIGHT)
        {
            put_pixel_vga_12h_mode(x0 + x, y0 - y, color);
        }

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        else
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// Draw a filled circle using the midpoint circle algorithm
void draw_filled_circle_vga_12h_mode(int x0, int y0, int radius, unsigned char color)
{
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        // Draw horizontal lines between octants for filling
        draw_line_vga_12h_mode(x0 - x, y0 + y, x0 + x, y0 + y, color);
        draw_line_vga_12h_mode(x0 - y, y0 + x, x0 + y, y0 + x, color);
        draw_line_vga_12h_mode(x0 - x, y0 - y, x0 + x, y0 - y, color);
        draw_line_vga_12h_mode(x0 - y, y0 - x, x0 + y, y0 - x, color);

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        else
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

// Wait for vertical sync to avoid screen tearing
void wait_vsync(void)
{
    // Wait until vertical retrace is not in progress
    while (inb(VGA_INSTAT_READ) & 8);

    // Wait until vertical retrace is in progress
    while (!(inb(VGA_INSTAT_READ) & 8));
}

void sleep(unsigned int ms)
{
    // Very simple busy-wait based delay
    // This is not accurate and depends on CPU speed
    // For a kernel, a better approach would be to use timer interrupts

    unsigned int i = 0;
    unsigned int j = 0;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 10000; j++);
    }
}

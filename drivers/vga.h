#ifndef VGA_H
#define VGA_H

#include "../drvutils/stdint.h"

// VGA memory constants
volatile uint8_t * VGA_MODE_PTR = (volatile uint8_t *)0xA0000; // VGA mem begins here.

// VGA register constants (from https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c)
#define VGA_AC_INDEX         0x3C0
#define VGA_AC_WRITE         0x3C0
#define VGA_AC_READ          0x3C1
#define VGA_MISC_WRITE       0x3C2
#define VGA_SEQ_INDEX        0x3C4
#define VGA_SEQ_DATA         0x3C5
#define VGA_DAC_READ_INDEX   0x3C7
#define VGA_DAC_WRITE_INDEX  0x3C8
#define VGA_DAC_DATA         0x3C9
#define VGA_MISC_READ        0x3CC
#define VGA_GC_INDEX         0x3CE
#define VGA_GC_DATA          0x3CF
#define VGA_CRTC_INDEX       0x3D4
#define VGA_CRTC_DATA        0x3D5
#define VGA_INSTAT_READ      0x3DA

// VGA regsiter values (from https://files.osdev.org/mirrors/geezer/osd/graphics/modes.c)
uint8_t mode_12h_regs_values[] =
{
/* MISC */
    0xE3,
/* SEQ */
    0x03, 0x01, 0x08, 0x00, 0x06,
/* CRTC */
    0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
    0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
    0xFF,
/* GC */
    0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
    0xFF,
/* AC */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x01, 0x00, 0x0F, 0x00, 0x00
};

/* 
    Assumes mode_12h_regs_values layout:
    [0]           : Miscellaneous Output Register
    [1-5]         : Sequencer Registers (5 registers)
    [6-30]        : CRT Controller Registers (25 registers)
    [31-39]       : Graphics Controller Registers (9 registers)
    [40-60]       : Attribute Controller Registers (21 registers)
*/
#define NUM_MISC  1
#define NUM_SEQ   5
#define NUM_CRTC  25
#define NUM_GC    9
#define NUM_AC    21

// VGA functions

void switch_to_vga_12h_mode(uint8_t * regs_values);
void clear_screen_vga_12h_mode(void);

void put_pixel_vga_12h_mode(int x, int y, uint8_t color);

void draw_line_vga_12h_mode(int x0, int y0, int x1, int y1, uint8_t color);

void draw_rectangle_vga_12h_mode(int x, int y, int width, int height, uint8_t color);
void draw_filled_rectangle_vga_12h_mode(int x, int y, int width, int height, uint8_t color);

void draw_circle_vga_12h_mode(int x0, int y0, int radius, uint8_t color);
void draw_filled_circle_vga_12h_mode(int x0, int y0, int radius, uint8_t color);

void wait_vsync(void);
void sleep(uint32_t ms);

// VGA 12h mode constants
#define VGA_12H_WIDTH 640
#define VGA_12H_HEIGHT 480

// VGA 12h mode color constants
#define VGA_BLACK           0x00
#define VGA_BLUE            0x01
#define VGA_GREEN           0x02
#define VGA_CYAN            0x03
#define VGA_RED             0x04
#define VGA_MAGENTA         0x05
#define VGA_BROWN           0x06
#define VGA_LIGHT_GRAY      0x07
#define VGA_DARK_GRAY       0x08
#define VGA_LIGHT_BLUE      0x09
#define VGA_LIGHT_GREEN     0x0A
#define VGA_LIGHT_CYAN      0x0B
#define VGA_LIGHT_RED       0x0C
#define VGA_LIGHT_MAGENTA   0x0D
#define VGA_YELLOW          0x0E
#define VGA_WHITE           0x0F

#endif // VGA_H

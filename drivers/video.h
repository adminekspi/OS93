#ifndef VIDEO_H
#define VIDEO_H

#include "../drvutils/stdint.h"

// Video text mode constants
volatile uint8_t * VIDEO_TEXT_MODE_PTR = (volatile uint8_t *)0xb8000; // Video mem begins here.

// Video text mode colors
#define TEXT_BLACK           0x00
#define TEXT_BLUE            0x01
#define TEXT_GREEN           0x02
#define TEXT_CYAN            0x03
#define TEXT_RED             0x04
#define TEXT_MAGENTA         0x05
#define TEXT_BROWN           0x06
#define TEXT_LIGHT_GRAY      0x07
#define TEXT_DARK_GRAY       0x08
#define TEXT_LIGHT_BLUE      0x09
#define TEXT_LIGHT_GREEN     0x0A
#define TEXT_LIGHT_CYAN      0x0B
#define TEXT_LIGHT_RED       0x0C
#define TEXT_LIGHT_MAGENTA   0x0D
#define TEXT_YELLOW          0x0E
#define TEXT_WHITE           0x0F

// Video type constants
enum video_type
{
    VIDEO_TYPE_NONE = 0x00,
    VIDEO_TYPE_COLOR = 0x20,
    VIDEO_TYPE_MONOCHROME = 0x30,
};

// Video text mode functions
void clear_screen(void);
void print_char(int8_t c, uint32_t x, uint32_t y, uint8_t color);
void print_string(int8_t * str, uint32_t x, uint32_t y, uint8_t color);

// Video type functions
// To be added later

#endif // VIDEO_H

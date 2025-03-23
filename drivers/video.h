#ifndef VIDEO_H
#define VIDEO_H

// Video text mode constants
volatile unsigned char * VIDEO_TEXT_MODE_PTR = (volatile unsigned char *)0xb8000; // Video mem begins here.

// Text colors
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

// Video text mode functions
void clear_screen(void);
void print_char(char c, unsigned int x, unsigned int y, unsigned char color);
void print_string(char * str, unsigned int x, unsigned int y, unsigned char color);

#endif // VIDEO_H

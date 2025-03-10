#ifndef VIDEO_H
#define VIDEO_H

// Video text mode constants
unsigned char * VIDEO_TEXT_MODE_PTR = (unsigned char *)0xb8000; // Video mem begins here.

// Video text mode functions
void clear_screen(void);
void print_char(char c, unsigned int x, unsigned int y, unsigned char color);
void print_string(char * str, unsigned int x, unsigned int y, unsigned char color);

#endif // VIDEO_H

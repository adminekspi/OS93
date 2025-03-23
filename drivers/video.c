#include "video.h"

void clear_screen(void)
{
    uint32_t i = 0;

    // this loop clears the screen
    while(i < 80 * 25 * 2)
    {
        VIDEO_TEXT_MODE_PTR[i] = ' ';
        VIDEO_TEXT_MODE_PTR[i + 1] = 0x07; // light grey
        i = i + 2;
    }
}

void print_char(int8_t c, uint32_t x, uint32_t y, uint8_t color)
{
    uint32_t index = (80 * y + x) * 2;
    VIDEO_TEXT_MODE_PTR[index] = c;
    VIDEO_TEXT_MODE_PTR[index + 1] = color;
}

void print_string(int8_t * str, uint32_t x, uint32_t y, uint8_t color)
{
    uint32_t i = 0;
    uint32_t index = (80 * y + x) * 2;

    if (str == 0 || x > 80 || y > 25)
    {
        return;
    }

    // this loop writes the string to video mem at the specified position
    while(str[i] != '\0' && index < 80 * 25 * 2)
    {
        VIDEO_TEXT_MODE_PTR[index] = str[i];
        VIDEO_TEXT_MODE_PTR[index + 1] = color;
        ++i;
        index = index + 2;
    }
}

uint16_t detect_video_type(void)
{
    return * VIDEO_TYPE_PTR;
}

enum video_type get_video_type(void)
{
    return (enum video_type)(detect_video_type() & 0x30); 
}

#include "drivers/ports.h"
#include "drivers/vga.h"
#include "drivers/video.h"
#include "kmain.h"

void kmain(void)
{
    enum video_type current_video_type = get_video_type();

    if (current_video_type == VIDEO_TYPE_COLOR)
    {
        switch_to_vga_12h_mode(mode_12h_regs_values);

        clear_screen_vga_12h_mode();

        put_pixel_vga_12h_mode(5, 5, 0x0F);
        draw_line_vga_12h_mode(10, 10, 50, 50, 0x01);

        draw_circle_vga_12h_mode(100, 100, 50, 0x02);
        draw_rectangle_vga_12h_mode(200, 200, 50, 50, 0x03);

        draw_filled_circle_vga_12h_mode(300, 300, 50, 0x04);
        draw_filled_rectangle_vga_12h_mode(100, 300, 50, 50, 0x05);
    }
    else
    {
        print_string(KMAIN_VIDEO_ERROR_MSG, 0, 0, VGA_WHITE);
    }

    while(1);
}

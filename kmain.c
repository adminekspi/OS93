#include "kmain.h"
#include "ports.h"
#include "vga.h"
#include "video.h"

void kmain(void)
{
    switch_to_vga_12h_mode(mode_12h_regs_values);

    clear_screen_vga_12h_mode();

    put_pixel_vga_12h_mode(5, 5, 0x0F);
    draw_line_vga_12h_mode(10, 10, 50, 50, 0x0F);
    draw_circle_vga_12h_mode(100, 100, 50, 0x0F);
    draw_rectangle_vga_12h_mode(200, 200, 50, 50, 0x0F);

    while(1);
}

#include "kmain.h"
#include "ports.h"
#include "vga.h"
#include "video.h"

void kmain(void)
{
    switch_to_vga_12h_mode(mode_12h_regs_values);
    clear_screen_vga_12h_mode();
    put_pixel_vga_12h_mode(5, 5, 0x0F);
    //get_font_vga_12h_mode();
    //draw_char_vga_12h_mode('A', 10, 10, 0x0F, 0x00);

    while(1);
}

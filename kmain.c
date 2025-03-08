#include "kmain.h"
#include "ports.h"
#include "vga.h"
#include "video.h"

void kmain(void)
{
    init_vga_12h_mode();
    clear_screen_vga();
    putpixel(100, 100, 15);

    while(1);
}

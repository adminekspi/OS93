#include "kmain.h"
#include "ports.h"
#include "vga.h"
#include "video.h"

void kmain(void)
{
    write_regs(mode_12h_regs);
    clear_screen_vga();
    putpixel(5, 5, 4);
    putpixel(6, 5, 4);
    putpixel(7, 5, 4);
    putpixel(8, 5, 4);
    putpixel(9, 5, 4);
    putpixel(10, 5, 4);

    while(1);
}

#include "drivers/ports.h"
#include "drivers/vga.h"
#include "drivers/video.h"
#include "drivers/ata.h"
#include "drivers/fat.h"
#include "drvutils/string.h"
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

        // Start ATA driver
        if (ata_init())
        {
            // Start FAT file system
            fat_fs_t fs;
            if (fat_init(&fs, ata_read_sectors, ata_write_sectors))
            {
                // Create example file
                if (create_hello_world_file(&fs))
                {
                    // Open and read file
                    fat_file_t file;
                    if (fat_open_file(&fs, (int8_t *)"HELLO.TXT", &file))
                    {
                        // Read data from file
                        uint32_t bytes_read = fat_read_file(&fs, &file, file_buffer, MAX_BUFFER_SIZE - 1);

                        if (bytes_read > 0)
                        {
                            // Set string end
                            file_buffer[bytes_read] = '\0';

                            // Draw a yellow circle if file is read successfully
                            if (bytes_read > 0 && file_buffer[0] == (int8_t)'H' && file_buffer[1] == (int8_t)'E' && file_buffer[2] == (int8_t)'L' && file_buffer[3] == (int8_t)'L' && file_buffer[4] == (int8_t)'O' && file_buffer[5] == (int8_t)'!')
                            {
                                draw_filled_circle_vga_12h_mode(400, 400, 50, 0x0E);
                            }
                        }

                        // Close file
                        fat_close_file(&file);
                    }
                    else
                    {
                        // Draw a red circle if file is not read successfully
                        draw_filled_circle_vga_12h_mode(400, 400, 50, 0x04);
                    }
                }
                else
                {
                    // Draw a red circle if file is not created successfully
                    draw_filled_circle_vga_12h_mode(400, 400, 50, 0x03);
                }
            }
            else
            {
                // Draw a red circle if FAT file system is not initialized successfully
                draw_filled_circle_vga_12h_mode(400, 400, 50, 0x02);
            }
        }
        else
        {
            // Draw a red circle if ATA driver is not initialized successfully
            draw_filled_circle_vga_12h_mode(400, 400, 50, 0x01);
        }
    }

    while(1);
}

static bool create_hello_world_file(fat_fs_t * fs)
{
    (void)fs;
    return true;
}

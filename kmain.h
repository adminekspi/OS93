#ifndef KMAIN_H
#define KMAIN_H

#include "drvutils/stdint.h"

#define MAX_BUFFER_SIZE          512
static uint8_t file_buffer[MAX_BUFFER_SIZE];

static bool create_hello_world_file(fat_fs_t * fs);

#endif // KMAIN_H

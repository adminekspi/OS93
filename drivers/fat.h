#ifndef FAT_H
#define FAT_H

#include "../drvutils/stdint.h"
#include "../drvutils/stddef.h"
#include "../drvutils/stdbool.h"

// FAT12/16/32 file system constants
#define FAT_SECTOR_SIZE          512
#define FAT_FILENAME_LENGTH      11
#define FAT_DIRECTORY_ENTRY_SIZE 32

// FAT boot sector
typedef struct
{
    uint8_t  jump_boot[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t  fat_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    
    // FAT12/16 spesifik
    uint8_t  drive_number;
    uint8_t  reserved1;
    uint8_t  boot_signature;
    uint32_t volume_id;
    uint8_t  volume_label[11];
    uint8_t  fs_type[8];
    
    // Gerisi
    uint8_t  boot_code[448];
    uint16_t boot_sector_signature;
} __attribute__((packed)) fat_boot_sector_t;

// FAT directory entry
typedef struct
{
    uint8_t  filename[FAT_FILENAME_LENGTH];
    uint8_t  attributes;
    uint8_t  reserved;
    uint8_t  creation_time_tenths;
    uint16_t creation_time;
    uint16_t creation_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t last_modification_time;
    uint16_t last_modification_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} __attribute__((packed)) fat_dir_entry_t;

// FAT file system structure
typedef struct
{
    fat_boot_sector_t boot_sector;
    uint32_t fat_begin_lba;
    uint32_t cluster_begin_lba;
    uint32_t root_dir_begin_lba;
    uint32_t root_dir_sectors;
    uint32_t data_sectors;
    uint32_t total_clusters;
    uint8_t  fat_type; // 12, 16 veya 32
} fat_fs_t;

// FAT file structure
typedef struct
{
    fat_dir_entry_t dir_entry;
    uint32_t current_cluster;
    uint32_t current_position;
} fat_file_t;

// ATA driver function pointers
typedef uint8_t (*ata_read_sectors_func)(uint32_t lba, uint8_t sector_count, uint8_t * buffer);
typedef uint8_t (*ata_write_sectors_func)(uint32_t lba, uint8_t sector_count, const uint8_t * buffer);

// FAT file system functions
bool fat_init(fat_fs_t * fs, ata_read_sectors_func read_func, ata_write_sectors_func write_func);
bool fat_open_file(fat_fs_t * fs, const int8_t * filename, fat_file_t * file);
uint32_t fat_read_file(fat_fs_t * fs, fat_file_t * file, uint8_t * buffer, uint32_t size);
void fat_close_file(fat_file_t * file);
bool fat_create_file(fat_fs_t * fs, const int8_t * filename);
uint32_t fat_write_file(fat_fs_t * fs, fat_file_t * file, const uint8_t * buffer, uint32_t size);

// Helper functions
uint32_t fat_get_next_cluster(fat_fs_t * fs, uint32_t current_cluster);
uint32_t fat_cluster_to_lba(fat_fs_t * fs, uint32_t cluster);
bool fat_find_file(fat_fs_t * fs, const int8_t * filename, fat_dir_entry_t* dir_entry);

#endif // FAT_H 
#include "fat.h"
#include "../drvutils/stdint.h"
#include "../drvutils/stdbool.h"
#include "../drvutils/string.h"

// Static variables
static ata_read_sectors_func _read_sectors = NULL;
static ata_write_sectors_func _write_sectors = NULL;
static uint8_t _fat_buffer[FAT_SECTOR_SIZE];

// Convert to FAT filename (helper function)
static void to_fat_filename(const int8_t * filename, int8_t * fat_filename)
{
    // Convert to DOS 8.3 filename format
    memset(fat_filename, ' ', 11);
    
    int i, j;
    for (i = 0, j = 0; filename[i] != '\0' && filename[i] != '.' && j < 8; i++, j++)
    {
        fat_filename[j] = toupper(filename[i]);
    }
    
    // Find extension
    while (filename[i] != '\0' && filename[i] != '.') i++;
    
    // Process extension (if exists)
    if (filename[i] == '.')
    {
        i++; // Skip dot
        for (j = 0; filename[i] != '\0' && j < 3; i++, j++)
        {
            fat_filename[8 + j] = toupper(filename[i]);
        }
    }
}

// Compare FAT filenames (helper function)
static bool compare_fat_filenames(const int8_t * fat_filename1, const int8_t * fat_filename2)
{
    return memcmp(fat_filename1, fat_filename2, 11) == 0;
}

// Initialize FAT file system
bool fat_init(fat_fs_t * fs, ata_read_sectors_func read_func, ata_write_sectors_func write_func)
{
    if (!read_func || !write_func)
    {
        return false;
    }

    _read_sectors = read_func;
    _write_sectors = write_func;

    // Read boot sector
    if (_read_sectors(0, 1, (uint8_t *)&fs->boot_sector) != 1)
    {
        return false;
    }

    // Check valid FAT signature
    if (fs->boot_sector.boot_sector_signature != 0xAA55)
    {
        return false;
    }

    // Calculate FAT configuration
    fs->fat_begin_lba = fs->boot_sector.reserved_sector_count;

    // For FAT12/16
    if (fs->boot_sector.root_entry_count != 0)
    {
        fs->root_dir_sectors = ((fs->boot_sector.root_entry_count * 32) + (fs->boot_sector.bytes_per_sector - 1)) / fs->boot_sector.bytes_per_sector;
        fs->root_dir_begin_lba = fs->fat_begin_lba + (fs->boot_sector.fat_count * fs->boot_sector.fat_size_16);

        uint32_t total_sectors = (fs->boot_sector.total_sectors_16 != 0) ?
                               fs->boot_sector.total_sectors_16 :
                               fs->boot_sector.total_sectors_32;

        fs->data_sectors = total_sectors - (fs->boot_sector.reserved_sector_count +
                          (fs->boot_sector.fat_count * fs->boot_sector.fat_size_16) +
                          fs->root_dir_sectors);

        fs->cluster_begin_lba = fs->root_dir_begin_lba + fs->root_dir_sectors;
    }
    else
    {
        // FAT32 support (simplified)
        return false;
    }

    // Determine FAT type
    fs->total_clusters = fs->data_sectors / fs->boot_sector.sectors_per_cluster;

    if (fs->total_clusters < 4085)
    {
        fs->fat_type = 12;
    }
    else if (fs->total_clusters < 65525)
    {
        fs->fat_type = 16;
    }
    else
    {
        fs->fat_type = 32;
    }
    
    return true;
}

// Find next cluster in FAT table
uint32_t fat_get_next_cluster(fat_fs_t * fs, uint32_t current_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t entry_offset;
    uint32_t cluster_value;

    // For FAT16
    if (fs->fat_type == 16)
    {
        fat_offset = current_cluster * 2;
        fat_sector = fs->fat_begin_lba + (fat_offset / FAT_SECTOR_SIZE);
        entry_offset = fat_offset % FAT_SECTOR_SIZE;

        if (_read_sectors(fat_sector, 1, _fat_buffer) != 1)
        {
            return 0xFFFF; // Read failed, accept last cluster
        }

        cluster_value = *((uint16_t *)&_fat_buffer[entry_offset]);

        // FAT16 value checks
        if (cluster_value >= 0xFFF8)
        {
            return 0xFFFF; // Last cluster
        }

        return cluster_value;
    }

    // For FAT12 (simplified support)
    else if (fs->fat_type == 12)
    {
        fat_offset = current_cluster + (current_cluster / 2); // 1.5 bytes per entry
        fat_sector = fs->fat_begin_lba + (fat_offset / FAT_SECTOR_SIZE);
        entry_offset = fat_offset % FAT_SECTOR_SIZE;

        // Read FAT sector
        if (_read_sectors(fat_sector, 1, _fat_buffer) != 1)
        {
            return 0xFFF; // Read failed, accept last cluster
        }

        // Value sector limit can be exceeded
        if (entry_offset == (FAT_SECTOR_SIZE - 1))
        {
            // Read second sector
            uint8_t second_buffer[FAT_SECTOR_SIZE];
            if (_read_sectors(fat_sector + 1, 1, second_buffer) != 1)
            {
                return 0xFFF;
            }

            // Combine values from two sectors
            uint16_t value = _fat_buffer[entry_offset] | (second_buffer[0] << 8);

            // Check even/odd cluster
            if (current_cluster & 0x1)
            {
                cluster_value = value >> 4; // Odd - high 12 bits
            }
            else
            {
                cluster_value = value & 0x0FFF; // Even - low 12 bits
            }
        }
        else
        {
            // Read from single sector
            uint16_t value = *((uint16_t*)&_fat_buffer[entry_offset]);
            
            // Check even/odd cluster
            if (current_cluster & 0x1)
            {
                cluster_value = value >> 4; // Odd - high 12 bits
            }
            else
            {
                cluster_value = value & 0x0FFF; // Even - low 12 bits
            }
        }

        // FAT12 value checks
        if (cluster_value >= 0xFF8)
        {
            return 0xFFF; // Last cluster
        }

        return cluster_value;
    }

    return 0xFFFFFFFF; // Unsupported FAT type
}

// Convert cluster number to LBA address
uint32_t fat_cluster_to_lba(fat_fs_t * fs, uint32_t cluster)
{
    // Cluster starts at 2, data region calculation is based on that
    return fs->cluster_begin_lba + ((cluster - 2) * fs->boot_sector.sectors_per_cluster);
}

// Find a file in the root directory
bool fat_find_file(fat_fs_t * fs, const int8_t * filename, fat_dir_entry_t * dir_entry)
{
    uint32_t current_sector = fs->root_dir_begin_lba;
    uint32_t sectors_to_read = fs->root_dir_sectors;

    int8_t fat_format_name[11];
    to_fat_filename(filename, fat_format_name);

    // Traverse root directory
    while (sectors_to_read > 0)
    {
        if (_read_sectors(current_sector, 1, _fat_buffer) != 1)
        {
            return false;
        }

        // Check each entry in this sector
        for (uint32_t i = 0; i < FAT_SECTOR_SIZE; i += 32)
        {
            fat_dir_entry_t * current_entry = (fat_dir_entry_t *)&_fat_buffer[i];

            // Directory entry is empty
            if (current_entry->filename[0] == 0)
            {
                return false;
            }

            // Deleted entry
            if (current_entry->filename[0] == 0xE5)
            {
                continue;
            }

            // Compare file names
            if (compare_fat_filenames((int8_t *)current_entry->filename, fat_format_name))
            {
                // Match found!
                memcpy(dir_entry, current_entry, sizeof(fat_dir_entry_t));
                return true;
            }
        }

        current_sector++;
        sectors_to_read--;
    }

    return false; // Not found
}

// Open a file
bool fat_open_file(fat_fs_t * fs, const int8_t * filename, fat_file_t * file)
{
    if (!fat_find_file(fs, filename, &file->dir_entry))
    {
        return false;
    }

    file->current_cluster = file->dir_entry.first_cluster_low;
    file->current_position = 0;

    return true;
}

// Read data from a file
uint32_t fat_read_file(fat_fs_t * fs, fat_file_t * file, uint8_t * buffer, uint32_t size)
{
    if (file->current_position >= file->dir_entry.file_size)
    {
        return 0; // File end reached
    }

    // Limit to file size
    if (file->current_position + size > file->dir_entry.file_size)
    {
        size = file->dir_entry.file_size - file->current_position;
    }

    uint32_t bytes_read = 0;
    uint32_t current_cluster = file->current_cluster;
    uint32_t cluster_size = fs->boot_sector.sectors_per_cluster * FAT_SECTOR_SIZE;
    uint32_t position_in_cluster = file->current_position % cluster_size;

    // If needed, move to the correct cluster
    uint32_t clusters_to_skip = file->current_position / cluster_size;
    for (uint32_t i = 0; i < clusters_to_skip; i++)
    {
        current_cluster = fat_get_next_cluster(fs, current_cluster);
        if (current_cluster >= 0xFFF8)
        {
            return 0; // File end prematurely
        }
    }

    // Read data
    while (bytes_read < size)
    {
        if (current_cluster >= 0xFFF8)
        {
            break; // File end
        }

        // Remaining bytes to read
        uint32_t cluster_bytes_left = cluster_size - position_in_cluster;
        uint32_t bytes_to_read = (size - bytes_read < cluster_bytes_left) ? (size - bytes_read) : cluster_bytes_left;

        // Convert cluster to LBA
        uint32_t start_sector = fat_cluster_to_lba(fs, current_cluster);
        uint32_t sector_offset = position_in_cluster / FAT_SECTOR_SIZE;
        uint32_t position_in_sector = position_in_cluster % FAT_SECTOR_SIZE;

        // Read sectors
        if (position_in_sector == 0 && bytes_to_read >= FAT_SECTOR_SIZE)
        {
            // Full sector read
            uint32_t sectors_to_read = bytes_to_read / FAT_SECTOR_SIZE;
            _read_sectors(start_sector + sector_offset, sectors_to_read, buffer + bytes_read);
            bytes_read += sectors_to_read * FAT_SECTOR_SIZE;
            position_in_cluster += sectors_to_read * FAT_SECTOR_SIZE;
        }
        else
        {
            // Partial sector read
            _read_sectors(start_sector + sector_offset, 1, _fat_buffer);
            uint32_t bytes_from_sector = (bytes_to_read < (FAT_SECTOR_SIZE - position_in_sector)) ? 
                                       bytes_to_read : (FAT_SECTOR_SIZE - position_in_sector);
            
            memcpy(buffer + bytes_read, _fat_buffer + position_in_sector, bytes_from_sector);
            bytes_read += bytes_from_sector;
            position_in_cluster += bytes_from_sector;
        }

        // Did we exceed the cluster size?
        if (position_in_cluster >= cluster_size)
        {
            current_cluster = fat_get_next_cluster(fs, current_cluster);
            position_in_cluster = 0;
        }
    }

    // Update file position
    file->current_position += bytes_read;
    file->current_cluster = current_cluster;

    return bytes_read;
}

// Close a file
void fat_close_file(fat_file_t * file)
{
    // If any cleanup is needed, do it here
    file->current_position = 0;
    file->current_cluster = 0;
}

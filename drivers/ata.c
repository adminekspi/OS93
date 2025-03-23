#include "ata.h"
#include "ports.h"
#include "../drvutils/stdint.h"

// ATA port for delays
static void ata_io_wait(void)
{
    // Delay for one read
    inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS);
    inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS);
    inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS);
    inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS);
}

// Wait for ATA controller to be ready
static bool ata_poll(bool check_err)
{
    uint8_t status;

    ata_io_wait(); // Short delay

    // Wait until BSY is cleared
    while ((status = inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS)) & ATA_SR_BSY);

    if (check_err)
    {
        if (status & ATA_SR_ERR)
        {
            return false;
        }

        if (status & ATA_SR_DF)
        {
            return false;
        }

        if (!(status & ATA_SR_DRQ))
        {
            return false;
        }
    }

    return true;
}

// Initialize ATA controller
bool ata_init(void)
{
    // Select primary device
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_DEVICE, 0xA0);
    ata_io_wait();

    // Identify device
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_io_wait();

    // Can the device respond?
    if (inb(ATA_PRIMARY_IO_BASE + ATA_REG_STATUS) == 0)
    {
        return false;
    }

    // Is the device ready?
    if (!ata_poll(true))
    {
        return false;
    }

    // Read identify data (we don't use it, we just check if the controller is working)
    for (int i = 0; i < 256; i++)
    {
        inw(ATA_PRIMARY_IO_BASE + ATA_REG_DATA);
    }
    
    return true;
}

// Read sectors from ATA (LBA28 mode)
uint8_t ata_read_sectors(uint32_t lba, uint8_t sector_count, uint8_t* buffer)
{
    if (!sector_count)
    {
        return 0;
    }

    // Select master device in LBA28 mode
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_ERROR, 0);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_SECCOUNT, sector_count);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBALO, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBAMID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBAHI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    uint16_t* buffer_ptr = (uint16_t*)buffer;

    for (uint8_t i = 0; i < sector_count; i++)
    {
        if (!ata_poll(true))
        {
            return i;
        }

        // Read 256 words (512 bytes) for each sector
        for (uint16_t j = 0; j < 256; j++)
        {
            buffer_ptr[j] = inw(ATA_PRIMARY_IO_BASE + ATA_REG_DATA);
        }

        buffer_ptr += 256;
    }

    return sector_count;
}

// Write sectors to ATA (LBA28 mode)
uint8_t ata_write_sectors(uint32_t lba, uint8_t sector_count, const uint8_t* buffer)
{
    if (!sector_count)
    {
        return 0;
    }

    // Select master device in LBA28 mode
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_DEVICE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_ERROR, 0);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_SECCOUNT, sector_count);
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBALO, (uint8_t)(lba & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBAMID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_LBAHI, (uint8_t)((lba >> 16) & 0xFF));
    outb(ATA_PRIMARY_IO_BASE + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    const uint16_t* buffer_ptr = (const uint16_t *)buffer;

    for (uint8_t i = 0; i < sector_count; i++)
    {
        if (!ata_poll(true))
        {
            return i;
        }

        // Write 256 words (512 bytes) for each sector
        for (uint16_t j = 0; j < 256; j++)
        {
            outw(ATA_PRIMARY_IO_BASE + ATA_REG_DATA, buffer_ptr[j]);
        }

        buffer_ptr += 256;
    }

    return sector_count;
} 
#include "ata.h"
#include "../../io.h"

#define ATA_DATA       0x1F0 //Data register
#define ATA_SECCOUNT0  0x1F2 //Sector count register
#define ATA_LBA0       0x1F3 //LBA low register
#define ATA_LBA1       0x1F4 //LBA mid register
#define ATA_LBA2       0x1F5 //LBA high register
#define ATA_HDDEVSEL   0x1F6 //Drive/Head register
#define ATA_COMMAND    0x1F7 //Sends commands
#define ATA_STATUS     0x1F7 //reads status

#define ATA_CMD_READ   0x20 // READ SECTORS (PIO)

#define ATA_SR_BSY     0x80 //device Busy
#define ATA_SR_DRQ     0x08 //Data request ready
#define ATA_CMD_WRITE 0x30
#define ATA_SR_ERR    0x01

static void ata_wait(void) {
    while (inb(ATA_STATUS) & ATA_SR_BSY)
        ;

    while (!(inb(ATA_STATUS) & ATA_SR_DRQ))
        ;
}

static void ata_flush(void) {
    outb(ATA_COMMAND, 0xE7);

    while (inb(ATA_STATUS) & ATA_SR_BSY)
        ;
}

int ata_read_sector(uint32_t lba, void *buffer) {
    uint16_t *buf = (uint16_t *)buffer;

    outb(ATA_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_SECCOUNT0, 1);

    outb(ATA_LBA0, lba & 0xFF);
    outb(ATA_LBA1, (lba >> 8) & 0xFF);
    outb(ATA_LBA2, (lba >> 16) & 0xFF);

    outb(ATA_COMMAND, ATA_CMD_READ);

    ata_wait();

    for (int i = 0; i < 256; i++) {
        buf[i] = inw(ATA_DATA);
    }

    return 0;
}

int ata_write_sector(uint32_t lba, const void *buffer) {
    const uint16_t *buf = (const uint16_t *)buffer;

    outb(ATA_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));

    outb(ATA_SECCOUNT0, 1);

    outb(ATA_LBA0, lba & 0xFF);
    outb(ATA_LBA1, (lba >> 8) & 0xFF);
    outb(ATA_LBA2, (lba >> 16) & 0xFF);

    outb(ATA_COMMAND, ATA_CMD_WRITE);

    ata_wait();

    for (int i = 0; i < 256; i++) {
        outw(ATA_DATA, buf[i]);
    }

    ata_flush();

    return 0;
}
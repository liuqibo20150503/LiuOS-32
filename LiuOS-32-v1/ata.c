#include "kernel.h"

#define ATA 0x1F0

static int wait_bsy(void) {
    int t = 100000;
    while (t-- > 0) if (!(inb(ATA+7) & 0x80)) return 0;
    return -1;
}

static int wait_drq(void) {
    int t = 100000;
    while (t-- > 0) {
        uint8_t s = inb(ATA+7);
        if (s & 0x01) return -1;
        if (s & 0x08) return 0;
    }
    return -1;
}

int ata_init(void) {
    if (wait_bsy()) return -1;
    outb(ATA+6, 0xA0);
    for (volatile int i = 0; i < 1000; i++);
    if (wait_bsy()) return -1;
    outb(ATA+7, 0xEC);
    if (wait_drq()) return -1;
    for (int i = 0; i < 256; i++) inw(ATA);
    return 0;
}

int ata_read(uint32_t lba, uint8_t* buf) {
    if (wait_bsy()) return -1;
    outb(ATA+6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA+2, 1);
    outb(ATA+3, lba & 0xFF);
    outb(ATA+4, (lba >> 8) & 0xFF);
    outb(ATA+5, (lba >> 16) & 0xFF);
    outb(ATA+7, 0x20);
    if (wait_drq()) return -1;
    uint16_t* b = (uint16_t*)buf;
    for (int i = 0; i < 256; i++) b[i] = inw(ATA);
    return 0;
}

int ata_write(uint32_t lba, uint8_t* buf) {
    if (wait_bsy()) return -1;
    outb(ATA+6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA+2, 1);
    outb(ATA+3, lba & 0xFF);
    outb(ATA+4, (lba >> 8) & 0xFF);
    outb(ATA+5, (lba >> 16) & 0xFF);
    outb(ATA+7, 0x30);
    if (wait_drq()) return -1;
    uint16_t* b = (uint16_t*)buf;
    for (int i = 0; i < 256; i++) outw(ATA, b[i]);
    outb(ATA+7, 0xE7);
    return wait_bsy();
}
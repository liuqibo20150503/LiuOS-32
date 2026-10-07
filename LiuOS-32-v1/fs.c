#include "kernel.h"

#define FS_MAGIC        0x4C465300
#define FS_SUPER_LBA    1000
#define FS_TABLE_LBA    1001
#define FS_DATA_LBA     1065

static void ser(file_t* f, uint8_t* b) {
    int o = 0;
    for (int i = 0; i < FNAME_LEN; i++) b[o++] = f->name[i];
    b[o++] = f->size & 0xFF;
    b[o++] = (f->size >> 8) & 0xFF;
    b[o++] = (f->size >> 16) & 0xFF;
    b[o++] = (f->size >> 24) & 0xFF;
    b[o++] = f->data_sector & 0xFF;
    b[o++] = (f->data_sector >> 8) & 0xFF;
    b[o++] = (f->data_sector >> 16) & 0xFF;
    b[o++] = (f->data_sector >> 24) & 0xFF;
}

static void deser(file_t* f, uint8_t* b) {
    int o = 0;
    for (int i = 0; i < FNAME_LEN; i++) f->name[i] = b[o++];
    f->size = b[o] | (b[o+1] << 8) | (b[o+2] << 16) | (b[o+3] << 24);
    o += 4;
    f->data_sector = b[o] | (b[o+1] << 8) | (b[o+2] << 16) | (b[o+3] << 24);
    o += 4;
}

int fs_init(void) {
    uint8_t b[512];
    if (ata_read(FS_SUPER_LBA, b)) return -1;

    uint32_t m = b[0] | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
    if (m != FS_MAGIC) {
        memset(b, 0, 512);
        b[0] = FS_MAGIC & 0xFF;
        b[1] = (FS_MAGIC >> 8) & 0xFF;
        b[2] = (FS_MAGIC >> 16) & 0xFF;
        b[3] = (FS_MAGIC >> 24) & 0xFF;
        ata_write(FS_SUPER_LBA, b);

        memset(b, 0, 512);
        for (int i = 0; i < MAX_FILES; i++) {
            ata_write(FS_TABLE_LBA + i, b);
        }
    }
    return 0;
}

int fs_find(const char* name, file_t* out, int* idx) {
    uint8_t b[512];
    for (int i = 0; i < MAX_FILES; i++) {
        ata_read(FS_TABLE_LBA + i, b);
        if (b[0] != 0) {
            file_t f;
            deser(&f, b);
            if (strcmp(f.name, name) == 0) {
                memcpy(out, &f, sizeof(f));
                *idx = i;
                return 0;
            }
        }
    }
    return -1;
}

int fs_create(const char* name) {
    uint8_t b[512];
    for (int i = 0; i < MAX_FILES; i++) {
        ata_read(FS_TABLE_LBA + i, b);
        if (b[0] == 0) {
            file_t f;
            memset(&f, 0, sizeof(f));
            strncpy(f.name, name, FNAME_LEN - 1);
            f.size = 0;
            f.data_sector = FS_DATA_LBA + i;

            memset(b, 0, 512);
            ser(&f, b);
            ata_write(FS_TABLE_LBA + i, b);
            return 0;
        }
    }
    return -1;
}

int fs_read(const char* name, uint8_t* buf, int max) {
    file_t f;
    int i;
    if (fs_find(name, &f, &i)) return -1;
    if (ata_read(f.data_sector, buf)) return -2;
    if (f.size > 512) f.size = 512;
    return f.size;
}

int fs_write(const char* name, uint8_t* data, int size) {
    file_t f;
    int i;
    if (fs_find(name, &f, &i)) return -1;
    if (size > 512) return -2;
    if (ata_write(f.data_sector, data)) return -3;

    f.size = size;
    uint8_t b[512];
    memset(b, 0, 512);
    ser(&f, b);
    ata_write(FS_TABLE_LBA + i, b);
    return size;
}

int fs_delete(const char* name) {
    file_t f;
    int i;
    if (fs_find(name, &f, &i)) return -1;

    uint8_t b[512];
    memset(b, 0, 512);
    ata_write(FS_TABLE_LBA + i, b);
    return 0;
}

void fs_list(void) {
    uint8_t b[512];
    int count = 0;

    puts("\n  NAME            SIZE\n");
    puts("  ====================\n");

    for (int i = 0; i < MAX_FILES; i++) {
        ata_read(FS_TABLE_LBA + i, b);
        if (b[0] != 0) {
            file_t f;
            deser(&f, b);
            puts("  ");
            puts(f.name);

            int l = strlen(f.name);
            for (int j = l; j < 16; j++) putc(' ');

            puti(f.size);
            puts(" bytes\n");
            count++;
        }
    }

    puts("\n  Total: ");
    puti(count);
    puts(" file(s)\n\n");
}
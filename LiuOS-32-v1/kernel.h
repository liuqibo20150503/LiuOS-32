#ifndef KERNEL_H
#define KERNEL_H

#define NULL ((void*)0)
#include <stddef.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(uint16_t port, uint16_t val) {
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void set_color(uint8_t fg, uint8_t bg);
void cls(void);
void putc(char c);
void puts(const char* s);
void puti(int n);

int strcmp(const char* a, const char* b);
int strlen(const char* s);
void strcpy(char* d, const char* s);
void strncpy(char* d, const char* s, int n);
void* memcpy(void* d, const void* s, size_t n);
void* memset(void* d, int c, size_t n);
void to_upper(char* s);

void* kmalloc(uint32_t size);
void kfree(void* p);
void mem_init(void);

char kgetc(void);
int kgets(char* buf, int max);

int ata_init(void);
int ata_read(uint32_t lba, uint8_t* buf);
int ata_write(uint32_t lba, uint8_t* buf);

#define MAX_FILES 64
#define FNAME_LEN 32

typedef struct {
    char name[FNAME_LEN];
    uint32_t size;
    uint32_t data_sector;
} file_t;

int fs_init(void);
int fs_create(const char* name);
int fs_delete(const char* name);
int fs_read(const char* name, uint8_t* buf, int max);
int fs_write(const char* name, uint8_t* data, int size);
void fs_list(void);

#endif
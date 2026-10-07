#include "kernel.h"

static uint16_t* vga = (uint16_t*)VGA_MEMORY;
static int cx = 0, cy = 0;
static uint8_t color = 0x0F;

void set_color(uint8_t fg, uint8_t bg) { color = (bg << 4) | fg; }

void cls(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) vga[i] = (color << 8) | ' ';
    cx = cy = 0;
}

void putc(char c) {
    if (c == '\n') { cx = 0; cy++; }
    else if (c == '\r') cx = 0;
    else if (c == '\b') { if (cx > 0) { cx--; vga[cy*VGA_WIDTH+cx] = (color<<8)|' '; } }
    else { vga[cy*VGA_WIDTH+cx] = (color<<8)|c; cx++; }
    if (cx >= VGA_WIDTH) { cx = 0; cy++; }
    if (cy >= VGA_HEIGHT) {
        for (int i = 0; i < VGA_WIDTH*79; i++) vga[i] = vga[i+VGA_WIDTH];
        for (int i = VGA_WIDTH*79; i < VGA_WIDTH*80; i++) vga[i] = (color<<8)|' ';
        cy = 24;
    }
}

void puts(const char* s) { while (*s) putc(*s++); }

void puti(int n) {
    if (n < 0) { putc('-'); n = -n; }
    if (n == 0) { putc('0'); return; }
    char b[16]; int i = 0;
    while (n > 0) { b[i++] = '0' + n%10; n /= 10; }
    while (i--) putc(b[i]);
}

int strcmp(const char* a, const char* b) {
    while (*a && *a == *b) { a++; b++; }
    return *(unsigned char*)a - *(unsigned char*)b;
}

int strlen(const char* s) { int n = 0; while (s[n]) n++; return n; }

void strcpy(char* d, const char* s) { while (*s) *d++ = *s++; *d = 0; }

void strncpy(char* d, const char* s, int n) {
    int i;
    for (i = 0; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0;
}

void* memcpy(void* d, const void* s, size_t n) {
    uint8_t* dd = (uint8_t*)d; const uint8_t* ss = (const uint8_t*)s;
    for (size_t i = 0; i < n; i++) dd[i] = ss[i];
    return d;
}

void* memset(void* d, int c, size_t n) {
    uint8_t* dd = (uint8_t*)d;
    for (size_t i = 0; i < n; i++) dd[i] = (uint8_t)c;
    return d;
}

void to_upper(char* s) { while (*s) { if (*s >= 'a' && *s <= 'z') *s -= 32; s++; } }
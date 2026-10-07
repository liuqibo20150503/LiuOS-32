#include "kernel.h"

static const char sc[] = {
    0, 0, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
    0, 'a','s','d','f','g','h','j','k','l',';','\'','`',
    0, '\\','z','x','c','v','b','n','m',',','.','/', 0,
    '*', 0, ' ', 0
};

char kgetc(void) {
    while (1) {
        uint8_t status = inb(0x64);
        if ((status & 1) && !(status & 0x20)) {
            uint8_t c = inb(0x60);
            if (c < 0x80 && c < sizeof(sc)) {
                char ch = sc[c];
                if (ch) return ch;
            }
        }
    }
}

int kgets(char* buf, int max) {
    int pos = 0;
    while (1) {
        char c = kgetc();
        if (c == '\n' || c == '\r') {
            buf[pos] = 0;
            putc('\n');
            return pos;
        } else if (c == '\b') {
            if (pos > 0) {
                pos--;
                putc('\b');
                putc(' ');
                putc('\b');
            }
        } else if (pos < max - 1) {
            buf[pos++] = c;
            putc(c);
        }
    }
}
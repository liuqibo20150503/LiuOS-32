#include "kernel.h"

void print_prompt(void) {
    set_color(0x0A, 0x00);
    puts("LiuOS>");
    set_color(0x0F, 0x00);
    putc(' ');
}

void cmd_help(void) {
    puts("\n");
    puts("  HELP     Show this help\n");
    puts("  CLS      Clear screen\n");
    puts("  VER      Version\n");
    puts("  ECHO     Echo text\n");
    puts("  DIR      List files\n");
    puts("  CREATE   Create file\n");
    puts("  WRITE    Write file\n");
    puts("  TYPE     Read file\n");
    puts("  DELETE   Delete file\n");
    puts("  TASKS    Task list\n");
    puts("  REBOOT   Reboot\n");
    puts("  SHUTDOWN Shutdown\n\n");
}

void cmd_ver(void) {
    puts("\n  LiuOS-32 Minimal v1.0\n");
    puts("  12 commands\n\n");
}

void cmd_echo(char* arg) {
    puts("\n  ");
    puts(arg);
    puts("\n\n");
}

void cmd_create(char* name) {
    if (strlen(name) == 0) {
        puts("\n  Usage: CREATE <filename>\n\n");
        return;
    }
    if (fs_create(name) == 0) {
        puts("\n  File created\n\n");
    } else {
        puts("\n  Create failed\n\n");
    }
}

void cmd_write(char* name) {
    if (strlen(name) == 0) {
        puts("\n  Usage: WRITE <filename>\n\n");
        return;
    }
    char buf[512];
    puts("\n  Content: ");
    int len = kgets(buf, 512);
    if (fs_write(name, (uint8_t*)buf, len) >= 0) {
        puts("  Written ");
        puti(len);
        puts(" bytes\n\n");
    } else {
        puts("\n  Write failed\n\n");
    }
}

void cmd_type(char* name) {
    if (strlen(name) == 0) {
        puts("\n  Usage: TYPE <filename>\n\n");
        return;
    }
    uint8_t buf[513];
    int sz = fs_read(name, buf, 512);
    if (sz > 0) {
        buf[sz] = 0;
        puts("\n  ");
        puts((char*)buf);
        puts("\n\n");
    } else {
        puts("\n  File not found\n\n");
    }
}

void cmd_delete(char* name) {
    if (strlen(name) == 0) {
        puts("\n  Usage: DELETE <filename>\n\n");
        return;
    }
    if (fs_delete(name) == 0) {
        puts("\n  Deleted\n\n");
    } else {
        puts("\n  Delete failed\n\n");
    }
}

void cmd_reboot(void) {
    puts("\n  Rebooting...\n");
    outb(0x64, 0xFE);
}

void cmd_shutdown(void) {
    puts("\n  Shutting down...\n");
    asm volatile("cli\nhlt");
}

void process(char* cmd) {
    to_upper(cmd);

    char* arg = cmd;
    while (*arg && *arg != ' ') arg++;
    if (*arg == ' ') {
        *arg = 0;
        arg++;
    }

    if (strcmp(cmd, "HELP") == 0) cmd_help();
    else if (strcmp(cmd, "CLS") == 0) cls();
    else if (strcmp(cmd, "VER") == 0) cmd_ver();
    else if (strcmp(cmd, "ECHO") == 0) cmd_echo(arg);
    else if (strcmp(cmd, "DIR") == 0) fs_list();
    else if (strcmp(cmd, "CREATE") == 0) cmd_create(arg);
    else if (strcmp(cmd, "WRITE") == 0) cmd_write(arg);
    else if (strcmp(cmd, "TYPE") == 0) cmd_type(arg);
    else if (strcmp(cmd, "DELETE") == 0) cmd_delete(arg);
    else if (strcmp(cmd, "TASKS") == 0) {
        puts("\n  PID  NAME     STATE\n");
        puts("  1    idle     READY\n");
        puts("  2    shell    RUNNING\n\n");
    }
    else if (strcmp(cmd, "REBOOT") == 0) cmd_reboot();
    else if (strcmp(cmd, "SHUTDOWN") == 0) cmd_shutdown();
    else if (strlen(cmd) > 0) {
        puts("\n  Unknown command: ");
        puts(cmd);
        puts("\n\n");
    }
}

void kernel_main(void) {
    asm volatile("cli");

    mem_init();
    ata_init();
    fs_init();

    puts("\n");
    puts("========================================\n");
    puts("    LiuOS-32 Minimal Kernel            \n");
    puts("    12 Real Commands                    \n");
    puts("========================================\n\n");
    puts("  Type HELP for commands\n\n");

    char cmd[128];

    while (1) {
        print_prompt();
        int len = kgets(cmd, 128);
        if (len > 0) process(cmd);
    }
}
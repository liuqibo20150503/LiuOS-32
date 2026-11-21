LiuOS-32 Development Notes

Each entry includes: Symptom, Cause, and Solution.

1. Toolchain Issues
1. MSYS2 i686-gcc fails silently (no output, exit code 1)
Symptom:

text
$ /mingw32/bin/i686-w64-mingw32-gcc.exe -c t.c -o t.o
Exit code: 1
(no output)
Cause:

MSYS2's gcc depends on cc1.exe, which depends on a set of DLLs.

After an MSYS2 upgrade, the DLL search path for cc1.exe no longer matches.

Even if gcc --version prints correctly, compilation fails silently.

Attempted (ineffective) solutions:

Copying DLLs: cp /mingw32/bin/*.dll /mingw32/lib/gcc/.../

Reinstalling: pacman -S mingw-w64-i686-gcc

Checking dependencies: ldd cc1.exe (only shows ntdll, no missing DLLs visible)

Final solution:

Abandon MSYS2's gcc.

Download i686-elf-tools-windows.zip (GCC 7.1.0, standalone toolchain).

Extract to C:\i686-elf-tools\.

Use C:\i686-elf-tools\bin\i686-elf-gcc.exe.

Lesson: MSYS2's 32-bit toolchain is unstable. Use a standalone i686-elf toolchain instead.

2. ld and objcopy not found
Symptom:

text
-bash: /mingw32/bin/i686-w64-mingw32-ld: No such file or directory
Cause:

mingw-w64-i686-binutils is installed, but the tools are not in /mingw32/bin/.

They are actually in /mingw32/i686-w64-mingw32/bin/, and without the i686-w64-mingw32- prefix.

Solution:

After installing the standalone i686-elf-tools, ld and objcopy are in C:\i686-elf-tools\bin\.

Named i686-elf-ld.exe and i686-elf-objcopy.exe.

3. ld -m elf_i386 reports "unrecognised emulation mode"
Symptom:

text
ld.exe: unrecognised emulation mode: elf_i386
Supported emulations: i386pep i386pe
Cause:

MSYS2's ld is a Windows PE linker.

It only recognizes i386pe, not Linux's elf_i386.

Solution:

Use i686-elf-ld.exe, which natively supports elf_i386.

4. Linking with gcc reports "cannot perform PE operations on non-PE output file"
Symptom:

text
cannot perform PE operations on non-PE output file 'kernel.bin'
Cause:

MSYS2's ld can only output PE format.

-Wl,--oformat,binary asks it to output raw binary, which it cannot do.

Solution:

Use i686-elf-ld + i686-elf-objcopy.

2. Bootloader Issues
5. Bootloader single read of 128 sectors fails
Symptom:

Bootloader executes, hangs at LBA read.

Error code 0E.

Cause:

QEMU's SeaBIOS has a bug with LBA extended read (AH=42h) on floppy.

All LBA reads on floppy return 0E (DMA boundary error).

Regardless of sector count or target address.

Attempted (ineffective) solutions:

Reducing sectors per read (64, 18, 1).

Changing target address to 0x20000 (avoiding 64KB boundary).

Using -fda.

Using -drive if=floppy.

Final solution:

Change QEMU to -drive if=ide (attach as IDE hard disk, not floppy).

Bootloader uses LBA read (IDE hard disk LBA works correctly).

Boot parameter: -boot c (boot from hard disk).

ata.c also accesses the same IDE device (consistency).

6. Bootloader code exceeds 512 bytes
Symptom:

text
bootloader.asm:292: error: TIMES value -229 is negative
Cause:

Added debug strings, error handling, CHS fallback logic.

Code exceeds 510 bytes.

Solution:

Trim the code.

Remove all debug strings.

Remove CHS fallback (keep only LBA).

Keep only the core flow.

Final minimal version:

4 batches of LBA reads, 64 sectors each.

Fits exactly in 512 bytes.

7. jmp 0x08:0x10000 truncated by nasm to jmp 0x08:0x0000
Symptom:

Bootloader enters protected mode, then jumps to 0x0000 (empty).

System hangs.

Cause:

0x10000 exceeds 16-bit offset range (0xFFFF).

In 16-bit mode, nasm only takes the lower 16 bits.

Solution:

First jump to a low-address stub inside the bootloader.

Then in 32-bit mode, far jump to 0x10000.

asm
a20_ok:
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_entry_16     ; jump to low-address stub

bits 32
pm_entry_16:
    mov ax, 0x10
    mov ds, ax
    ...
    jmp 0x08:0x10000         ; now 0x10000 is a 32-bit offset
3. Kernel Issues
8. size_t redefinition
Symptom:

text
error: conflicting types for 'size_t'
note: previous declaration with type 'long unsigned int'
Cause:

kernel.h has typedef unsigned long size_t;

But #include <stddef.h> already defines size_t.

Solution:

Remove typedef unsigned long size_t;.

Keep #include <stddef.h>.

9. strcpy wrong number of parameters
Symptom:

Compilation error.

Cause:

Declaration was char* strcpy(char* d, const char* s, int n);

But strcpy always takes 2 parameters.

Solution:

char* strcpy(char* d, const char* s);

10. gets / getc conflict with standard library
Symptom:

Linker reports undefined reference to 'kgets'.

Or conflicts with libc's gets/getc.

Cause:

Our gets/getc share names with standard C library.

Conflicts in some compiler environments.

Solution:

Rename to kgetc / kgets.

Update kernel.h, keyboard.c, kernel.c consistently.

11. IDT not initialized → timer interrupt triggers GPF
Symptom:

Kernel prints banner, then crashes immediately.

QEMU log shows v=0d (GPF) and INT=0x0e (page fault).

Cause:

IDT not initialized.

Timer interrupt (IRQ0) fires, CPU looks up IDT, finds all zeros.

Triggers GPF → double fault → triple fault → reboot.

Solution:

Add asm volatile("cli"); at the start of kernel_main.

Disable all hardware interrupts.

Keyboard switches to polling mode (no interrupts).

12. Keyboard doesn't work after cli
Symptom:

After cli, keyboard's hlt instruction never returns.

Cause:

Keyboard interrupts are disabled.

hlt in getc waits for an interrupt that never comes.

Solution:

Remove hlt.

Use pure polling:

c
char kgetc(void) {
    while (1) {
        uint8_t status = inb(0x64);
        if ((status & 1) && !(status & 0x20)) {
            uint8_t c = inb(0x60);
            ...
        }
    }
}
13. fs_init reads/writes LBA 1, overwriting kernel code
Symptom:

Crashes as soon as fs_init is called.

Cause:

Bootloader writes kernel.bin starting at LBA 1.

fs_init reads superblock from LBA 1.

Reads kernel machine code, not FS_MAGIC.

fs_init thinks disk is unformatted, clears LBA 1.

Kernel code is erased.

Solution:

Move file system to higher LBA.

FS_SUPER_LBA = 1000

FS_TABLE_LBA = 1001

FS_DATA_LBA = 1065

14. mem_init() not called
Symptom:

Crashes when using kmalloc.

Cause:

kernel_main forgot to call mem_init().

Heap not initialized, heap->size is random.

Solution:

Add mem_init(); at the start of kernel_main.

4. ATA / File System Issues
15. ata_write FLUSH CACHE timeout
Symptom:

After writing data, wait_bsy always times out.

Cause:

Sends 0xE7 (FLUSH CACHE), QEMU's default IDE doesn't respond.

Solution:

Remove outb(ATA+7, 0xE7).

Just return 0.

16. deser missing o += 4 at the end
Symptom:

Logic asymmetry (though data_sector is the last field, so it doesn't affect results).

Solution:

Add o += 4; at the end of deser, symmetric with ser.

17. fs_list shows all "occupied", filenames garbled
Symptom:

DIR shows 64 files, all garbled.

Cause:

ata_read uses IDE port 0x1F0.

But QEMU with -fda attaches a floppy controller (0x3F0).

Reads garbage data.

Solution:

QEMU uses -drive if=ide (attach as IDE hard disk).

Both ata.c and bootloader access the same IDE device.

After consistency, DIR works correctly.

18. Same file appears twice in DIR
Symptom:

text
a.txt           5 bytes
a.txt           0 bytes
Cause:

fs_create doesn't check if file already exists.

Repeated CREATE occupies a new file table entry.

Solution (to be fixed):

At the start of fs_create, call fs_find.

If it exists, return -2.

In kernel.c's cmd_create, handle -2 with "File already exists".

5. VGA Display Issues
19. Chinese characters displayed as white blocks
Symptom:

All Chinese characters are blank or blocks.

Cause:

VGA text mode uses 8x8 ASCII font.

Only supports characters 0x20-0x7F.

Chinese is double-byte, each byte exceeds 0x7F.

Solution:

Change all UI text to English.

For Chinese in the future, switch to graphics mode + custom bitmap font.
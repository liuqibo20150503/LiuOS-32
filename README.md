LiuOS-32 Minimal
A minimal 32-bit operating system built from scratch.

Supports protected mode, ATA driver, custom file system, and 12 built-in shell commands.

https://img.shields.io/badge/version-1.0-blue
https://img.shields.io/badge/license-GPL--3.0-green
https://img.shields.io/badge/platform-i386-lightgrey

✨ Features
Bootloader: Hand-written bootloader with LBA extended read, switches from real mode to 32-bit protected mode.

Drivers: ATA PIO read/write driver, keyboard polling driver, VGA text mode output.

File System: Custom three-region structure (superblock / file table / data area), supports file creation, read/write, and deletion.

Commands: 12 real, usable shell commands.

Memory: Simple kernel heap allocator (kmalloc / kfree).

🕹️ Built-in Commands
Command	Description
HELP	Show help
CLS	Clear screen
VER	Version info
ECHO	Echo text
DIR	List files
CREATE	Create file
WRITE	Write file
TYPE	Read file
DELETE	Delete file
TASKS	Task list
REBOOT	Reboot
SHUTDOWN	Shutdown
🛠️ Build & Run
Toolchain
NASM

i686-elf-tools (GCC 7.1.0)

QEMU

Build
bash
./build.bat
Run
bash
qemu-system-i386 -drive file=disk.img,format=raw,if=ide,index=0,media=disk -boot c -m 32M
📁 Project Structure
text
LiuOS-32/
├── bootloader.asm      # Bootloader (16-bit real mode → 32-bit protected mode)
├── kernel_entry.asm    # Kernel entry (32-bit)
├── kernel.c            # Kernel main + command processing
├── kernel.h            # Kernel header
├── string.c            # VGA output + string utilities
├── keyboard.c          # Keyboard polling driver
├── memory.c            # Kernel heap allocator
├── ata.c               # ATA PIO read/write driver
├── fs.c                # Custom file system
├── linker.ld           # Linker script
├── build.bat           # Build script
└── disk.img            # Bootable disk image
📖 Development Notes
For a detailed log of problems, causes, and solutions encountered during development, see DEVELOPMENT.md.

📝 Changelog
See CHANGELOG.md for version history.

📜 License
GPL-3.0 License

👤 Author
liuqibo20150503

⭐ Support
If you find this project interesting, feel free to star it.

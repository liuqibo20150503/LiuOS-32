@echo off
setlocal

set TOOLCHAIN=C:\i686-elf-tools\bin
set CFLAGS=-m32 -ffreestanding -nostdlib -fno-stack-protector -fno-pic -mno-red-zone -fno-builtin -O2

echo === 清理 ===
del /q *.o *.bin *.elf *.exe disk.img 2>nul

echo === 编译汇编 ===
nasm -f bin bootloader.asm -o bootloader.bin
if errorlevel 1 goto error

nasm -f elf32 kernel_entry.asm -o kernel_entry.o
if errorlevel 1 goto error

echo === 编译C ===
%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c kernel.c -o kernel.o
if errorlevel 1 goto error

%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c string.c -o string.o
if errorlevel 1 goto error

%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c keyboard.c -o keyboard.o
if errorlevel 1 goto error

%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c memory.c -o memory.o
if errorlevel 1 goto error

%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c ata.c -o ata.o
if errorlevel 1 goto error

%TOOLCHAIN%\i686-elf-gcc.exe %CFLAGS% -c fs.c -o fs.o
if errorlevel 1 goto error

echo === 链接 ===
%TOOLCHAIN%\i686-elf-ld.exe -m elf_i386 -T linker.ld -o kernel.elf ^
    kernel_entry.o kernel.o string.o keyboard.o memory.o ata.o fs.o
if errorlevel 1 goto error

echo === 提取二进制 ===
%TOOLCHAIN%\i686-elf-objcopy.exe -O binary kernel.elf kernel.bin
if errorlevel 1 goto error

echo === 制作镜像 ===
dd if=/dev/zero of=disk.img bs=512 count=2880 2>nul
dd if=bootloader.bin of=disk.img bs=512 count=1 conv=notrunc 2>nul
dd if=kernel.bin of=disk.img bs=512 seek=1 conv=notrunc 2>nul

echo.
echo === 构建完成 ===
dir kernel.bin disk.img
echo.
echo 运行: qemu-system-i386 -fda disk.img
goto end

:error
echo.
echo *** 构建失败 ***
exit /b 1

:end
endlocal
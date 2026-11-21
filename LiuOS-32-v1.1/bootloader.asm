org 0x7C00
bits 16

KERNEL_ADDR equ 0x20000

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov ax, 0x0003
    int 0x10

    mov al, 'R'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    mov al, '1'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    ; 用 LBA 读（IDE 支持）
    mov word [dap_sectors], 64
    mov word [dap_off], 0x0000
    mov word [dap_seg], 0x2000
    mov dword [dap_lba], 1

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc err_read

    mov al, '2'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov word [dap_off], 0x8000
    mov dword [dap_lba], 65

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc err_read

    mov al, '3'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov word [dap_off], 0x0000
    mov word [dap_seg], 0x3000
    mov dword [dap_lba], 129

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc err_read

    mov al, '4'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov word [dap_off], 0x8000
    mov dword [dap_lba], 193

    mov si, dap
    mov ah, 0x42
    mov dl, [boot_drive]
    int 0x13
    jc err_read

    mov al, 'K'
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10

    mov ax, 2401h
    int 15h
    jnc a20_ok

    call a20_wait
    mov al, 0xAD
    out 0x64, al
    call a20_wait
    mov al, 0xD0
    out 0x64, al
    call a20_wait2
    in al, 0x60
    push ax
    call a20_wait
    mov al, 0xD1
    out 0x64, al
    call a20_wait
    pop ax
    or al, 2
    out 0x60, al
    call a20_wait
    mov al, 0xAE
    out 0x64, al
    call a20_wait

a20_ok:
    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:pm_entry

a20_wait:
    in al, 0x64
    test al, 2
    jnz a20_wait
    ret

a20_wait2:
    in al, 0x64
    test al, 1
    jz a20_wait2
    ret

err_read:
    mov si, msg_err
    call print_16
    mov al, ah
    call print_hex
    jmp halt

halt:
    cli
    hlt
    jmp halt

print_16:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    jmp print_16
.done:
    ret

print_hex:
    push ax
    push bx
    push cx
    mov cx, 2
.loop:
    rol al, 4
    mov bl, al
    and bl, 0x0F
    add bl, '0'
    cmp bl, '9'
    jle .digit
    add bl, 7
.digit:
    push ax
    mov al, bl
    mov ah, 0x0E
    mov bx, 0x0007
    int 0x10
    pop ax
    loop .loop
    pop cx
    pop bx
    pop ax
    ret

dap:
    db 0x10
    db 0
dap_sectors:
    dw 64
dap_off:
    dw 0
dap_seg:
    dw 0x2000
dap_lba:
    dd 0
    dd 0

gdt_start:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_descriptor:
    dw $ - gdt_start - 1
    dd gdt_start

msg_err db ' Disk error! code=', 0
boot_drive db 0

bits 32
pm_entry:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp 0x08:KERNEL_ADDR

times 510-($-$$) db 0
dw 0xAA55
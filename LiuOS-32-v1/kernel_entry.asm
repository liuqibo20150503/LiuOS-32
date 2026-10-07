bits 32
section .text
global kernel_entry
extern kernel_main
extern _bss_start
extern _bss_end

kernel_entry:
    mov esp, 0x90000
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi
    jle .skip_bss
    xor eax, eax
    rep stosb
.skip_bss:
    call kernel_main
    cli
    hlt
    jmp $
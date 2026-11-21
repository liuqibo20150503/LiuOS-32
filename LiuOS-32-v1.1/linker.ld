ENTRY(kernel_entry)

SECTIONS {
    . = 0x20000;
    .text : { *(.text) }
    .data : { *(.data) }
    .bss : {
        _bss_start = .;
        *(.bss)
        *(COMMON)
        _bss_end = .;
    }
}
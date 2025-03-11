/* Custom linker script for RP2040 with filesystem section at the end of flash */

MEMORY
{
    /* Reserve the last 256KB of flash for our filesystem */
    FLASH(rx)     : ORIGIN = 0x10000000, LENGTH = 2048k - 256k
    RAM(rwx)      : ORIGIN = 0x20000000, LENGTH = 256k
    FILESYSTEM(r) : ORIGIN = 0x10000000 + 2048k - 256k, LENGTH = 256k
}

ENTRY(_entry_point)

EXTERN(BOOT2_FIRMWARE)

SECTIONS
{
    /* Second stage bootloader is prepended to the image, it must be 256 bytes aligned */
    .boot2 : {
        __boot2_start__ = .;
        KEEP(*(.boot2))
        __boot2_end__ = .;
    } > FLASH

    ASSERT(__boot2_end__ - __boot2_start__ == 256, "ERROR: Pico second stage bootloader must be 256 bytes in size")

    /* The second stage will always enter the image at the same address */
    .vectors : {
        __vectors_start__ = .;
        KEEP(*(.vectors))
        __vectors_end__ = .;
    } > FLASH

    .text : {
        /* Four more vectors: entry point, three for debug */
        . = ALIGN(4);
        *(.entry_point)
        . = . + 12; /* 3 more words */
        __logical_binary_start = .;
        KEEP(*(.reset))
        *(.text*)
        *(.init)
        *(.fini)
        *(SORT_NONE(.init_array*))
        __init_array_start = .;
        KEEP (*(.init_array*))
        __init_array_end = .;
        __preinit_array_start = .;
        KEEP (*(.preinit_array*))
        __preinit_array_end = .;
        *(.gcc_except_table)
        *(.eh_frame_hdr)
        *(.eh_frame)
        *(.gnu.linkonce.t.*)
        . = ALIGN(4);
    } > FLASH

    .rodata : {
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        __rodata_end = .;
        __binary_end = .;
    } > FLASH

    .data : {
        __data_start__ = .;
        __global_pointer$ = . + 0x800;
        *(.time_critical*)
        *(.data*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        __data_end__ = .;
    } > RAM AT > FLASH
    __data_source__ = LOADADDR(.data);

    .bss : {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    /* Be more careful with stack and heap allocation */
    .heap (NOLOAD) : {
        . = ALIGN(4);
        __end__ = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > RAM

    /* Stacks are full descending, so initial_sp is at the end of RAM */
    .stack (NOLOAD) : {
        . = ALIGN(4);
        __StackLimit = .;
        *(.stack*)
        . = . + 0x800; /* 2KB for stack */
        __StackTop = .;
        __initial_sp = .;
        __StackOneTop = .;
        /* Place stack for core 1 below the main stack */
        . = ALIGN(4);
        __StackOneBottom = .;
        . = . + 0x800; /* 2KB for core 1 stack */
    } > RAM

    /* Check for RAM overflow */
    ASSERT(__StackOneBottom + 0x800 <= (ORIGIN(RAM) + LENGTH(RAM)), "Error: RAM overflow")
    
    /* Add mutex array symbols expected by the Pico SDK */
    __mutex_array_start = .;
    __mutex_array_end = .;

    /* Align the filesystem to a flash sector boundary */
    .filesystem (NOLOAD) : {
        . = ALIGN(4096);
        __filesystem_start__ = .;
        KEEP(*(.littlefs_section))
        __filesystem_end__ = .;
    } > FILESYSTEM

    /* ARM exception handling sections */
    .ARM.extab : {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH
    
    .ARM.exidx : {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    /* PICO memory segments */
    .scratch_x : {
        __scratch_x_start__ = .;
        *(.scratch_x.*)
        . = ALIGN(4);
        __scratch_x_end__ = .;
    } > RAM AT > FLASH
    __scratch_x_source__ = LOADADDR(.scratch_x);

    .scratch_y : {
        __scratch_y_start__ = .;
        *(.scratch_y.*)
        . = ALIGN(4);
        __scratch_y_end__ = .;
    } > RAM AT > FLASH
    __scratch_y_source__ = LOADADDR(.scratch_y);

    /* Define memory regions and symbols to work with Pico SDK */
    __etext = LOADADDR(.data) + SIZEOF(.data);
    __StackLimit = __StackTop - 0x2000; /* 8KB of stack */
}
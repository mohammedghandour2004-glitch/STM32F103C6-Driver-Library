/*
 * <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< STM32F103C6.ld >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 * Author      : MOHAMMED GHANDOUR
 * Target      : STM32F103C6
 * Layer       : STARTUP
 * Component   : LINKER
 * Description : This file defines the Flash, SRAM, stack, and section memory layout.
 */

ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 0x8000
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 0x2800
}

_estack = ORIGIN(RAM) + LENGTH(RAM);
_stack_bottom = _estack - 0x400;

SECTIONS
{
    .isr_vector ORIGIN(FLASH) :
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        . = ALIGN(4);
        *(.text .text.*)
        *(.glue_7 .glue_7t)
        *(.rodata .rodata.*)
        *(.eh_frame .eh_frame.*)
        . = ALIGN(4);
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab .ARM.extab.* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx .ARM.exidx.* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH
    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    /* Reserve the downward-growing stack without initializing its memory. */
    .stack _stack_bottom (NOLOAD) :
    {
        . += 0x400;
    } > RAM

    ASSERT(MAX(_edata, _ebss) <= _stack_bottom,
           "STM32F103C6 RAM allocations overlap the reserved 1 KiB stack")
    ASSERT(LOADADDR(.data) + SIZEOF(.data) <= ORIGIN(FLASH) + LENGTH(FLASH),
           "STM32F103C6 FLASH overflow, including the .data load image")
}

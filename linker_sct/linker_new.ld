OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)

MEMORY
{
    FLASH (rx) : ORIGIN = @flash_base@, LENGTH = @flash_size@ 
    RAM (wx) : ORIGIN = @ram_base@, LENGTH = @ram_size@
}

PHDRS 
{
    text PT_LOAD;
    data PT_LOAD;
    rodata PT_LOAD;
    boot PT_LOAD;
    stack PT_LOAD;
}

SECTIONS
{
    .boot : {
        . = ALIGN(0x200);
        PROVIDE(__isr_vector_offset = 0);
        PROVIDE(__sisr_vector = .);
        KEEP(*(.isr_vector));
        PROVIDE(__eisr_vector = .);
    } > FLASH :boot

    .text : {
        . = ALIGN(4);
        PROVIDE(__stext = .);

        *(.text)
        *(.text.*)
        *(.text*)

        PROVIDE(__sctor = .);
        *(.ctors)
        PROVIDE(__ector = .);

        PROVIDE(__sdtor = .);
        *(.dtors)
        PROVIDE(__edtor = .);

        PROVIDE(__etext = .);
    } > FLASH :text

    .rodata : {
        . = ALIGN(4);
        PROVIDE(__srodata = .);
        *(.rodata)
        *(.rodata.*)
        *(.rodata*)

        . = ALIGN(4);
        __sinit_func = .;
        KEEP(*(SORT(.init_func.*)));
        __einit_func = .;

        . = ALIGN(4);
        __stest_cases = .;
        KEEP(*(SORT(.test_cases.*)));
        __etest_cases = .;

        . = ALIGN(4);
        __sdemo = .;
        KEEP(*(SORT(.demo.*)));
        __edemo = .;

        . = ALIGN(4);
        __sconsole_cmd = .;
        KEEP(*(.console.cmd));
        KEEP(*(.console.builtin.cmd));
        __econsole_cmd = .;

        PROVIDE(__erodata = .);
    } > FLASH :rodata

    .ARM.exidx : {
        PROVIDE_HIDDEN (__exidx_start = .);
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        PROVIDE_HIDDEN (__exidx_end = .);
    } > FLASH :rodata

    .stack (NOLOAD): {
        PROVIDE(__estack = .);
        /* stack size 0x400 */
        . += @stack_size@;
        . = ALIGN(0x4);
        PROVIDE(__stack = .);
    } > RAM :stack

    .data : {
        . = ALIGN(4);
        PROVIDE(__load_start = .);
        PROVIDE(__sdata = .);
        *(.data)
        *(.data.*)
        *(.data*)
        PROVIDE(__edata = .);
        PROVIDE(__load_end = .);
    } > RAM AT > FLASH :data

    PROVIDE(__load_addr = LOADADDR(.data));

    .bss (NOLOAD) : {
        . = ALIGN(4);
        PROVIDE(__sbss = .);
        *(.bss)
        *(.bss.*)
        *(.bss*)
        *(COMMON)
        PROVIDE(__ebss = .);
    } > RAM :data

    PROVIDE(end = .);
}

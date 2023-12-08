OUTPUT_ARCH(arm)
ENTRY(Reset_Handler)

MEMORY
{
    RAM (wx) : ORIGIN = @ram_base@, LENGTH = @ram_size@
}

SECTIONS
{
    .boot : {
        . = ALIGN(0x200);
        PROVIDE(__sisr_vector = .);
        PROVIDE(__isr_vector_offset = __sisr_vector - @ram_base@);
        KEEP(*(.isr_vector));
        PROVIDE(__eisr_vector = .);
    } > RAM

    .stack (NOLOAD) : {
        PROVIDE(__estack = .);
        . += @stack_size@;
        . = ALIGN(0x4);
        PROVIDE(__stack = .);
    } > RAM

    .text : {
        . = ALIGN(4);
        PROVIDE(__load_start = .);
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
    } > RAM

    PROVIDE(__load_addr = LOADADDR(.text));

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
    } > RAM

    .ARM.exidx : {
        PROVIDE_HIDDEN (__exidx_start = .);
        *(.ARM.exidx*)
        *(.gnu.linkonce.armexidx.*)
        PROVIDE_HIDDEN (__exidx_end = .);
    } > RAM

    .data : {
        . = ALIGN(4);
        PROVIDE(__sdata = .);
        *(.data)
        *(.data.*)
        *(.data*)
        PROVIDE(__edata = .);
        PROVIDE(__load_end = .);
    } > RAM

    .bss (NOLOAD) : {
        . = ALIGN(4);
        PROVIDE(__sbss = .);
        *(.bss)
        *(.bss.*)
        *(.bss*)
        *(COMMON)
        PROVIDE(__ebss = .);
    } > RAM

    PROVIDE(end = .);

    /* Stabs debugging sections.  */
    .stab          0 : { *(.stab) }
    .stabstr       0 : { *(.stabstr) }
    .stab.excl     0 : { *(.stab.excl) }
    .stab.exclstr  0 : { *(.stab.exclstr) }
    .stab.index    0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment       0 : { *(.comment) }
    .gnu.build.attributes : { *(.gnu.build.attributes .gnu.build.attributes.*) }
    /* DWARF debug sections.
        Symbols in the DWARF debugging sections are relative to the beginning
        of the section so we begin them at 0.  */
    /* DWARF 1.  */
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    /* GNU DWARF 1 extensions.  */
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    /* DWARF 1.1 and DWARF 2.  */
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    /* DWARF 2.  */
    .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
    .debug_abbrev   0 : { *(.debug_abbrev) }
    .debug_line     0 : { *(.debug_line .debug_line.* .debug_line_end) }
    .debug_frame    0 : { *(.debug_frame) }
    .debug_str      0 : { *(.debug_str) }
    .debug_loc      0 : { *(.debug_loc) }
    .debug_macinfo  0 : { *(.debug_macinfo) }
    /* SGI/MIPS DWARF 2 extensions.  */
    .debug_weaknames 0 : { *(.debug_weaknames) }
    .debug_funcnames 0 : { *(.debug_funcnames) }
    .debug_typenames 0 : { *(.debug_typenames) }
    .debug_varnames  0 : { *(.debug_varnames) }
    /* DWARF 3.  */
    .debug_pubtypes 0 : { *(.debug_pubtypes) }
    .debug_ranges   0 : { *(.debug_ranges) }
    /* DWARF 5.  */
    .debug_addr     0 : { *(.debug_addr) }
    .debug_line_str 0 : { *(.debug_line_str) }
    .debug_loclists 0 : { *(.debug_loclists) }
    .debug_macro    0 : { *(.debug_macro) }
    .debug_names    0 : { *(.debug_names) }
    .debug_rnglists 0 : { *(.debug_rnglists) }
    .debug_str_offsets 0 : { *(.debug_str_offsets) }
    .debug_sup      0 : { *(.debug_sup) }
    .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) KEEP (*(.gnu.attributes)) }
    .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
    /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }

    /* .special : {
        . = ALIGN(4);
        *(.ARM.attributes)
        *(.comment)
        *(.debug_info)
        *(.debug_abbrev)
        *(.debug_aranges)
        *(.debug_macro)
        *(.debug_line)
        *(.debug_frame)
        *(.debug_str)

    } > RAM */
}

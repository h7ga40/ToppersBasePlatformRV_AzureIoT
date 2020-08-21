/*
 *  @(#) $Id: riscv64elf.ld 127 2019-09-17 17:37:39Z roi $
 */

MEMORY
{
  /*
   * Memory with CPU cache.
   *6M CPU SRAM
   */
  ram (wxa!ri) : ORIGIN = 0x80000000, LENGTH = (6 * 1024 * 1024)
  /*
   * Memory without CPU cache
   * 6M CPU SRAM
  */
  ram_nocache (wxa!ri) : ORIGIN = 0x40000000, LENGTH = (6 * 1024 * 1024)
}

OUTPUT_ARCH(riscv)

PROVIDE(hardware_init_hook = 0);
PROVIDE(software_init_hook = 0);
PROVIDE(software_term_hook = 0);
PROVIDE(__idata_start = 0);
PROVIDE(__idata_end = 0);
STARTUP(start.o)

SECTIONS
{
    __stack_size = DEFINED(__stack_size) ? __stack_size : 8K;

    .init :
    {
        KEEP (*(.init))
    }>ram
    .text :
    {
        __text = . ;
        *(.text .stub .text.* .gnu.linkonce.t.*)

    }>ram
    .fini :
    {
        KEEP (*(.fini))
    }>ram
    _etext = . ;
    PROVIDE (etext = .) ;
    .rodata :
    {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }>ram
    . = ALIGN(8);
	.data : 
    {
        __data_start = . ;
        /* Writable data segment (.data segment) */
        *(.data .data.*)
        *(.sdata .sdata.* .sdata2 .sdata2.*)
        . = ALIGN(8);
        PROVIDE( __global_pointer$ = ABSOLUTE(.) + 0x800);
    }>ram
    .eh_frame : { KEEP (*(.eh_frame)) }>ram
    .gcc_except_table : { *(.gcc_except_table .gcc_except_table.*) }>ram
    .ctors :
    {
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o ) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*(.ctors))
    }>ram
    .dtors :
    {
        KEEP (*crtbegin.o(.dtors))
        KEEP (*(EXCLUDE_FILE (*crtend.o ) .dtors))
        KEEP (*(SORT(.dtors.*)))
        KEEP (*(.dtors))
    }>ram
    _edata  = . ;
    PROVIDE (edata = .) ;
    . = ALIGN(8) ;
    __bss_start = . ;
    _bss = .;
    .bss :
    {
        *(.bss .bss.*)
        *(.sbss .sbss.* .sbss2 .sbss2.*)
        *(COMMON)
    }>ram
    _end = . ;
    __bss_end = . ;
    _ebss = . ;
    .stack (NOLOAD) :
    {
    	*(.stack)
    }>ram
    .heap (NOLOAD) :
    {
        PROVIDE (_heap_start = .) ;
        *(.heap)
    }>ram
    PROVIDE (_heap_end = .) ;
    PROVIDE (end = .) ;
    .comment        0 : { *(.comment) }
    .debug          0 : { *(.debug) }
    .line           0 : { *(.line) }
    .debug_srcinfo  0 : { *(.debug_srcinfo) }
    .debug_sfnames  0 : { *(.debug_sfnames) }
    .debug_aranges  0 : { *(.debug_aranges) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
}

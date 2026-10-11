/* STM32 F427: flash 2048 KiB; contiguous SRAM 192 KiB.
 * 64 KiB CCM and 4 KiB backup SRAM are intentionally not allocated. */
ENTRY(Reset_Handler)
MEMORY {
 FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 2048K
 RAM (xrw) : ORIGIN = 0x20000000, LENGTH = 192K
}
_estack = ORIGIN(RAM) + LENGTH(RAM);
_Min_Heap_Size = 0;
_Min_Stack_Size = 0x800;
PHDRS { flash PT_LOAD FLAGS(5); ram PT_LOAD FLAGS(6); }
SECTIONS {
 .isr_vector : { . = ALIGN(4); KEEP(*(.isr_vector)) . = ALIGN(4); } >FLASH :flash
 .text : { *(.text*) *(.glue_7*) *(.eh_frame*) KEEP(*(.init)) KEEP(*(.fini)) . = ALIGN(4); _etext = .; } >FLASH :flash
 .rodata : { *(.rodata*) . = ALIGN(4); } >FLASH :flash
 .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH :flash
 .ARM.exidx : { __exidx_start = .; *(.ARM.exidx*) __exidx_end = .; } >FLASH :flash
 .preinit_array : { PROVIDE_HIDDEN(__preinit_array_start = .); KEEP(*(.preinit_array*)) PROVIDE_HIDDEN(__preinit_array_end = .); } >FLASH :flash
 .init_array : { PROVIDE_HIDDEN(__init_array_start = .); KEEP(*(SORT(.init_array.*))) KEEP(*(.init_array*)) PROVIDE_HIDDEN(__init_array_end = .); } >FLASH :flash
 .fini_array : { PROVIDE_HIDDEN(__fini_array_start = .); KEEP(*(SORT(.fini_array.*))) KEEP(*(.fini_array*)) PROVIDE_HIDDEN(__fini_array_end = .); } >FLASH :flash
 _sidata = LOADADDR(.data);
 .data : { . = ALIGN(4); _sdata = .; *(.data*) *(.RamFunc*) . = ALIGN(4); _edata = .; } >RAM AT>FLASH :ram
 .bss (NOLOAD) : { . = ALIGN(4); _sbss = .; __bss_start__ = .; *(.bss*) *(COMMON) . = ALIGN(4); _ebss = .; __bss_end__ = .; . = ALIGN(8); PROVIDE(end = .); PROVIDE(_end = .); . += _Min_Heap_Size + _Min_Stack_Size; . = ALIGN(8); } >RAM :ram
 .ARM.attributes 0 : { *(.ARM.attributes) }
 /DISCARD/ : { *(.note.GNU-stack) }
}
ASSERT(_ebss + _Min_Heap_Size + _Min_Stack_Size <= _estack, "SRAM overflow: stack reserve exhausted")

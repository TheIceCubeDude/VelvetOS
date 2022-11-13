OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
	/*. = 0x100000;*/
	. = 0x0;
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	.data : { *(.data) }
	.bss  : { *(.bss)  }
	.got : { _GLOBAL_OFFSET_TABLE_ = .; *(.got) }
}

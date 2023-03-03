OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
	/*. = 0x100000;*/
	. = 0x0;
	.text : { *(.text) }
	.rodata : ALIGN(4){*(.rodata) }
	.data : ALIGN(4) {*(.data) }
	.bss  : ALIGN(4) {*(.bss)  }
	.got : ALIGN(4) { _GLOBAL_OFFSET_TABLE_ = .;
	       		 *(.got) }
	.plt : {*(.plt) }
}

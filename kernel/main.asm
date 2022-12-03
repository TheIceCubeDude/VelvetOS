[BITS 32]

extern kmain
global _start

_start:
	;; Setup stack (ECX contains location of mmap, EDX contains location of fbInfo)
	mov esp, [mmap.stack - coreinfo + ecx]
	add esp, [mmap.stackSize - coreinfo + ecx]
	;; Setup segments
	mov bx, 0x10
	mov ss, bx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	
	push edx
	push ecx
	call kmain
	jmp $

	%include "structures/coreinfo.asm"

	;times kernelSize-($-$$) db 0


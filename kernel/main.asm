[BITS 32]

extern kmain
global _start

_start:
	;; Setup stack (ECX contains location of mmap)
	mov esp, [mmap.stack - mmap + ecx]
	add esp, stackSize
	;; Setup segments
	mov bx, 0x10
	mov ss, bx
	mov ds, bx
	mov es, bx
	mov fs, bx
	mov gs, bx
	
	;; C function parameter mmap
	push ecx
	call kmain
	hlt

	%include "structures/size_info.asm"
	%include "structures/mmap.asm"
	;times kernelSize-($-$$) db 0


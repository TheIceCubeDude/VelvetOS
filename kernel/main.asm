[BITS 32]
[ORG 0]

start:
	mov esp, stackSize
	;; Stack segment
	mov bx, 0x10
	mov ss, bx
	;; Heap segment
	mov bx, 0x18
	mov ds, bx
	;; Framebuffer segment
	mov bx, 0x20
	mov es, bx
	;; GDT segment
	mov bx, 0x28
	mov fs, bx
	;; Code segment
	mov bx, 0x30
	mov gs, bx
	
main:
	hlt

	%include "structures/size_info.asm"
	times kernelSize-($-$$) db 0


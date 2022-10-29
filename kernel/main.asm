[BITS 32]
[ORG 0]

setupSegments:
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
	;; NOTE: scheduler should set GS to the current program segment
	
main:
	hlt

	%include "structures/size_info.asm"
	times kernelSize-($-$$) db 0


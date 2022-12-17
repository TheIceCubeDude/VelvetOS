[BITS 32]

extern kmain
global _start

global halt
global font

_start:
	;; Setup segments
	mov ax, 0x10
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	;;Reload GDT again so that it is in high memory
	mov eax, [mmap.kernel - coreinfo + ecx]
	lgdt [gdtr + eax]
	;; Setup stack (ECX contains location of mmap, EBX contains location of fbInfo)
	mov esp, [mmap.stack - coreinfo + ecx]
	add esp, [mmap.stackSize - coreinfo + ecx]
	;; Align stack (we don't do this in mmap because stack grows downwards)
	mov edx, 0
	mov eax, esp
	mov ebp, 4
	div ebp
	sub esp, edx
	
	mov eax, font
	push eax
	push ebx
	push ecx
	call kmain
	jmp halt

halt:
	hlt
	jmp halt

font: incbin "Inconsolata-16b.psf" 

%include "kernel/core/memory.asm"
%include "structures/coreinfo.asm"
%include "structures/gdt.asm"

	;times kernelSize-($-$$) db 0


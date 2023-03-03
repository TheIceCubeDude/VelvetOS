[BITS 32]
SECTION .text 

_start:
	call .getAdr ;;Should be relative
	.getAdr:
	;;Gets saved EIP (pushed on stack from call)
	pop ebx
	;;EIP was incremented to point to the next instruction, so fix that
	sub ebx, .getAdr
	jmp short main

main: 
	mov eax, [counter + ebx]
	push eax
	call [printDec + ebx]
	pop eax
	
	mov eax, [counter + ebx]
	inc eax
	mov [counter + ebx], eax

	call [yield + ebx]

	jmp short main

SECTION .data
counter: dd 0

%include "OS_programs/libsyscall.asm"


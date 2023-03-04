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
	call .playSound

	mov eax, [counter + ebx]
	inc eax
	cmp eax, 40
	je short .end
	mov [counter + ebx], eax

	call [yield + ebx]

	jmp short main

	.playSound:
	mov ecx, 4

	mov eax, [counter + ebx]
	mul ecx
	add eax, duration
	add eax, ebx
	mov eax, [eax]
	push eax
	
	mov eax, [counter + ebx]
	mul ecx
	add eax, frequency
	add eax, ebx
	mov eax, [eax]
	push eax
	
	call [playSound + ebx]
	pop eax
	pop eax
	ret
	
	.end:
	mov eax, str0
	add eax, ebx
	push eax
	call [printf + ebx]
	cli
	hlt

SECTION .data
str0: db "Finished!", 0
counter: dd 0
;;Arrays of data from https://www.jk-quantized.com/blog/2013/11/22/tetris-theme-song-using-processing
frequency: dd 659, 493, 523, 587, 523, 493, 440, 440, 523, 659, 587, 523, 493, 523, 587, 659, 523, 440, 440, 440, 493, 523, 587, 698, 880, 783, 698, 659, 523, 659, 587, 523, 493, 493, 523, 587, 659, 523, 440, 440 
duration: dd 406, 203, 203, 406, 203, 203, 406, 203, 203, 406, 203, 203, 609, 203, 406, 406, 406, 406, 203, 203, 203, 203, 609, 203, 406, 203, 203, 609, 203, 406, 203, 203, 406, 203, 203, 406, 406, 406, 406, 406

%include "OS_programs/libsyscall.asm"


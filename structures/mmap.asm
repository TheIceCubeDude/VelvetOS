codeAccess	equ	10011010b
flags		equ	11001111b
dataAccess	equ	10010010b

nullSeg:	dq 0
codeSeg:
	.limit_low:	dw 0xFFFF
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db codeAccess
	.flags:		db flags
	.base_hi:	db 0
dataSeg:
	.limit_low:	dw 0xFFFF
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db flags
	.base_hi:	db 0
gdtr:
	dw gdtr - nullSeg - 1
	dd nullSeg

mmap:	
	.kernel:		dd 0
	.code:			dd 0
	.mmap:			dd 0
	.stack:			dd 0
	.heap:			dd 0
	.framebuffer:		dd 0
	.framebufferSize:	dd 0
mmapEnd:

%define gdtSize (gdtrEnd - nullSeg)
%define mmapSize (mmapEnd - mmap)
pModeMsg: db "Entering VBE, loading GDT and jumping to 32 bit protected mode kernel...", 0


	

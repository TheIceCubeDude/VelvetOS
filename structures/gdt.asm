codeAccess	equ	10011010b
noGranFlags	equ	01000000b
granFlags	equ	11000000b
dataAccess	equ	10010010b

nullSeg:	dq 0
kernelSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db codeAccess
	.flags:		db granFlags
	.base_hi:	db 0
stackSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db granFlags
	.base_hi:	db 0
heapSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db granFlags
	.base_hi:	db 0
framebufferSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db granFlags
	.base_hi:	db 0
gdtSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db noGranFlags
	.base_hi:	db 0

;; Starts off as data, but scheduler should set it to code before execution
codeSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db dataAccess
	.flags:		db granFlags
	.base_hi:	db 0

gdtr:
	dw gdtr - nullSeg - 1
	dd nullSeg

gdtrEnd:

%define gdtSize (gdtrEnd - nullSeg)
pModeMsg: db "Entering VBE, loading GDT and jumping to 32 bit protected mode kernel...", 0


	

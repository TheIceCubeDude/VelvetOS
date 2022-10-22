%define codeAccess	10011010b
%define noGranFlags	01000000b
%define granFlags	11000000b
%define dataAccess	10010010b

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

;; Program code segs will be created by the scheduler (that is why GDT seg is r/w)
codeSegs: times 10 dq 0

gdtr:
	dw codeSegs - nullSeg - 1
	dd nullSeg
	
pModeMsg: db "Entering VBE, loading GDT and jumping to 32 bit protected mode kernel...", 0


	

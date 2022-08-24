nullSeg:	dq 0
kernelSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db 0
	.flags:		db 0
	.base_hi:	db 0
stackSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db 0
	.flags:		db 0
	.base_hi:	db 0
heapSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db 0
	.flags:		db 0
	.base_hi:	db 0
programsSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db 0
	.flags:		db 0
	.base_hi:	db 0
framebufferSeg:
	.limit_low:	dw 0
	.base_low:	dw 0
	.base_mid:	db 0
	.access:	db 0
	.flags:		db 0
	.base_hi:	db 0
	
gdtr:
	dw gdtr - nullSeg - 1
	dd nullSeg



	

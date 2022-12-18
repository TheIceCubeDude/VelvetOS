coreinfo:
mmap:	
	.coreinfo:		dd 0
	.kernel:		dd 0
	.kernelSize:		dd 1048576 + 15
	.code:			dd 0
	.codeSize:		dd 0 ;;5242880 + 15
	.stack:			dd 0
	.stackSize:		dd 0 ;;2087152 + 15
	.heap:			dd 0
	.heapSize:		dd 0
fbInfo:
	.framebuffer:		dd 0
	.framebufferSize:	dd 0
	.framebufferWidth:	dw 0
	.framebufferHeight:	dw 0
	.framebufferBPL:	dw 0
coreinfoEnd:


%define coreinfoSize (coreinfoEnd - coreinfo)

	

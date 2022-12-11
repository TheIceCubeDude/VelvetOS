coreinfo:
mmap:	
	.coreinfo:		dd 0
	.kernel:		dd 0
	.kernelSize:		dd 1048576 + 3 ;; 3 bytes for alignment room
	.code:			dd 0
	.codeSize:		dd 5242880 + 3 ;; 3 bytes for alignment room
	.stack:			dd 0
	.stackSize:		dd 2087152
	.heap:			dd 0
	.heapSize:		dd 5242880 + 3 ;; 3 bytes for alignment room

fbInfo:
	.framebuffer:		dd 0
	.framebufferSize:	dd 0
	.framebufferWidth:	dw 0
	.framebufferHeight:	dw 0
	.framebufferBPL:	dw 0
coreinfoEnd:


%define coreinfoSize (coreinfoEnd - coreinfo)

	

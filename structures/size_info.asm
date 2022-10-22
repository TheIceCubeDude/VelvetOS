kernelSize:	dd 1048576	;1 meg
heapSize:	dd 5242880	;5 megs
stackSize:	dd 2097152	;2 megs
gdtSize: dd (gdtr + 8) - nullSeg

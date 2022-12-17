struct memoryMap {
	void *mmap;
	void *kernel;
	uint32_t kernelSize;
	void *code;
	uint32_t codeSize;
	void *stack;
	uint32_t stackSize;
	void *heap;
	uint32_t heapSize;
} __attribute__ ((packed));
struct framebufferInfo {
	uint32_t *framebuffer;
	uint32_t  framebufferSize;
	uint16_t framebufferWidth;
	uint16_t framebufferHeight;
	uint16_t framebufferBPL;
} __attribute__ ((packed));

static struct memoryMap *mmap;
static struct framebufferInfo *fbInfo;

void kpanic(uint8_t* cause) {
	//TODO: also implement double buffering by first implementing malloc and free and calloc
	//	also allocate segment sizes based on memory avaliable rather than hardcoding it
	setTextColours(0x00FF0000, 0x00FFFFFF);
	printf("Kernel Panic!!!");
	setTextColours(0x00A00000, 0x00FFFFFF);
	printf("Kernel offset & size:");
	printHex((uint32_t) mmap->kernel);
	printHex(mmap->kernelSize);
	printf("Stack offset & size:");
	printHex((uint32_t) mmap->stack);
	printHex(mmap->stackSize);
	printf("Heap offset & size:");
	printHex((uint32_t) mmap->heap);
	printHex(mmap->heapSize);
	printf("Program code offset & size:");
	printHex((uint32_t) mmap->code);
	printHex(mmap->codeSize);
	printf("VESA VBE framebuffer offset & size:");
	printHex((uint32_t)fbInfo->framebuffer);
	printHex(fbInfo->framebufferSize);
	printf("VESA VBE framebuffer width, height and Bytes Per Line (BPL)");
	printDec(fbInfo->framebufferWidth);
	printDec(fbInfo->framebufferHeight);
	printDec(fbInfo->framebufferBPL);
	printf("Cause of panic:");
	printf(cause);
	halt();
}

void _singleBufPrint() {
	setTextColours(0x000000FF, 0x00FFFF00);
	printf("VelvetOS v0.1 kernel now booting...");
	setTextColours(0x00472F1F, 0x00FFAF00);
	printf("Core graphics loaded.");
	return;
}

extern void kmain(struct memoryMap *mmapParam, struct framebufferInfo *fbInfoParam, void *font) {
	mmap = mmapParam;
	fbInfo = fbInfoParam;
	setHeap(mmap->heap);

	//Init graphics
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebufferSize, fbInfo->framebuffer);
	loadFont((void*) ((uint32_t)mmap->kernel + (uint32_t)font));
	consoleInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight);
	
	_singleBufPrint();
	if(!enableSSE()) {kpanic("CPU does not support SSE 2.0");}
	printf("SSE enabled.");
	enableDoubleBuffering();
	resetCursor();
	//Print stuff from before to get framebuf contents into backbuf
	//(we can't copy from framebuf to backbuf because it is sloooow)
	_singleBufPrint();
	printf("SSE enabled.");
	printf("Double buffering enabled.");
	for (int i=0; i<30; i++) {
		printDec(i);
	}
	kpanic("OS halted.");
	return;
}

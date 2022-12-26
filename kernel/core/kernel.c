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
	disableIrqs();
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

void playTheme() {
	uint16_t frequency[] = {659.25511, 493.8833, 523.25113, 587.32954, 523.25113, 493.8833, 440.0, 440.0, 523.25113, 659.25511, 587.32954, 523.25113, 493.8833, 523.25113, 587.32954, 659.25511, 523.25113, 440.0, 440.0, 440.0, 493.8833, 523.25113, 587.32954, 698.45646, 880.0, 783.99087, 698.45646, 659.25511, 523.25113, 659.25511, 587.32954, 523.25113, 493.8833, 493.8833, 523.25113, 587.32954, 659.25511, 523.25113, 440.0, 440.0};
	uint16_t duration[] = {406.250, 203.125, 203.125, 406.250, 203.125, 203.125, 406.250, 203.125, 203.125, 406.250, 203.125, 203.125, 609.375, 203.125, 406.250, 406.250, 406.250, 406.250, 203.125, 203.125, 203.125, 203.125, 609.375, 203.125, 406.250, 203.125, 203.125, 609.375, 203.125, 406.250, 203.125, 203.125, 406.250, 203.125, 203.125, 406.250, 406.250, 406.250, 406.250, 406.250};

	//Arrays of data from https://www.jk-quantized.com/blog/2013/11/22/tetris-theme-song-using-processing
	for (int i=0; i<40; i++) {
		playSound(frequency[i], duration[i]);
	}

	return;
}

extern void kmain(struct memoryMap *mmapParam, struct framebufferInfo *fbInfoParam, void *font) {
	//Setup some base stuff
	mmap = mmapParam;
	fbInfo = fbInfoParam;
	setHeap(mmap->heap);
	makeHeap(mmap->heapSize);

	//Init graphics
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebufferSize, fbInfo->framebuffer);
	consoleInit((void*) ((uint32_t)mmap->kernel + (uint32_t)font));
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

	//Init interrupts
	idtInit();
	printf("Interrupts have been set up.");

	//Init Programmable Interval Timer
	initPit();
	printf("PIT has been set up.");

	//Init PS/2 controller
	initPs2();
	printf("PS/2 hardware initalised.");
	enableIrqs();

	printf("Enter whenever ready for an epic theme.");
	uint8_t string[1024] = {'>', 0};
	uint8_t stringptr = 1;
	uint8_t ctrl = 1;
	while (ctrl) {
		uint8_t keys[32];
		getNewKeys(keys);
		for (uint8_t i=0; i<32; i++) {
			if (keys[i] == '\b' && (!(stringptr == 1))) {stringptr--; string[stringptr] = 0;}
			else if (keys[i] == '\n') {ctrl = 0;}
			else if (keys[i]) {string[stringptr] = keys[i]; stringptr++;}
		}
		printf(string);
	}

	printf("Epic theme in:");
	printDec(3);
	sleep(1000);
	printDec(2);
	sleep(1000);
	printDec(1);
	sleep(1000);
	playTheme();

	kpanic("OS halted.");
	return;
}



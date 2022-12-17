void halt();
void kpanic(char *cause);
char enableSSE();
void memcpy(char *destination, char *source, unsigned long size);
unsigned int memset(char *destination, unsigned long x, unsigned long size);

#include "core/heap.c"
#include "video/graphics.c"
#include "video/text.c"

struct memoryMap {
	char *mmap;
	char *kernel;
	unsigned long kernelSize;
	char *code;
	unsigned long codeSize;
	char *stack;
	unsigned long stackSize;
	char *heap;
	unsigned long heapSize;
} __attribute__ ((packed));
struct framebufferInfo {
	unsigned long *framebuffer;
	unsigned long framebufferSize;
	unsigned short framebufferWidth;
	unsigned short framebufferHeight;
	unsigned short framebufferBPL;
} __attribute__ ((packed));

static struct memoryMap *mmap;
static struct framebufferInfo *fbInfo;

void kpanic(char* cause) {
	//TODO: also implement double buffering by first implementing malloc and free and calloc
	//	also allocate segment sizes based on memory avaliable rather than hardcoding it
	setTextColours(0x00FF0000, 0x00FFFFFF);
	printf("Kernel Panic!!!");
	setTextColours(0x00A00000, 0x00FFFFFF);
	printf("Kernel offset & size:");
	printHex((unsigned long) mmap->kernel);
	printHex(mmap->kernelSize);
	printf("Stack offset & size:");
	printHex((unsigned long) mmap->stack);
	printHex(mmap->stackSize);
	printf("Heap offset & size:");
	printHex((unsigned long) mmap->heap);
	printHex(mmap->heapSize);
	printf("Program code offset & size:");
	printHex((unsigned long) mmap->code);
	printHex(mmap->codeSize);
	printf("VESA VBE framebuffer offset & size:");
	printHex((unsigned long)fbInfo->framebuffer);
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

extern void kmain(struct memoryMap *mmapParam, struct framebufferInfo *fbInfoParam, char *font) {
	mmap = mmapParam;
	fbInfo = fbInfoParam;
	setHeap(mmap->heap);

	//Init graphics
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebufferSize, fbInfo->framebuffer);
	loadFont((unsigned char*) ((unsigned long)mmap->kernel + (unsigned long)font));
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

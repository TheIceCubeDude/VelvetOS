#include "video/graphics.c"
#include "video/text.c"

struct memoryMap {
	unsigned long mmap;
	unsigned long kernel;
	unsigned long kernelSize;
	unsigned long code;
	unsigned long codeSize;
	unsigned long stack;
	unsigned long stackSize;
	unsigned long heap;
	unsigned long heapSize;
} __attribute__ ((packed));
struct framebufferInfo {
	unsigned long* framebuffer;
	unsigned long framebufferSize;
	unsigned short framebufferWidth;
	unsigned short framebufferHeight;
	unsigned short framebufferBPL;
} __attribute__ ((packed));

void _printCoreInfo(struct memoryMap *mmap, struct framebufferInfo *fbInfo) {
	//TODO: perhaps change this to kernel panic???
	//	also implement double buffering by first implementing malloc and free and calloc
	//	also allocate segment sizes based on memory avaliable rather than hardcoding it
	setTextColours(0x000000FF, 0x00FFFF00);
	printf("VelvetOS v0.1 kernel now booting...");
	setTextColours(0x00472F1F, 0x00FFAF00);
	printf("Kernel offset & size:");
	printHex(mmap->kernel);
	printHex(mmap->kernelSize);
	printf("Stack offset & size:");
	printHex(mmap->stack);
	printHex(mmap->stackSize);
	printf("Heap offset & size:");
	printHex(mmap->heap);
	printHex(mmap->heapSize);
	printf("Program code offset & size:");
	printHex(mmap->code);
	printHex(mmap->codeSize);
	printf("VESA VBE framebuffer offset & size:");
	printHex((unsigned long)fbInfo->framebuffer);
	printHex(fbInfo->framebufferSize);
	printf("VESA VBE framebuffer width, height and Bytes Per Line (BPL)");
	printDec(fbInfo->framebufferWidth);
	printDec(fbInfo->framebufferHeight);
	printDec(fbInfo->framebufferBPL);
	return;
}

extern void kmain(struct memoryMap *mmap, struct framebufferInfo *fbInfo, unsigned long font) {
	//Init graphics
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebuffer);
	loadFont((char*)mmap->kernel + font);
	initConsole(fbInfo->framebufferWidth, fbInfo->framebufferHeight);

	_printCoreInfo(mmap, fbInfo);
	return;
}

#include "video/graphics.c"

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

unsigned short divFac = 2;
extern void kmain(struct memoryMap *mmap, struct framebufferInfo *fbInfo) {
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebuffer);
	putPixel(fbInfo->framebufferWidth/divFac, fbInfo->framebufferHeight/divFac,0x00FFFFFF);
	return;
}

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

extern void kmain(struct memoryMap *mmap, struct framebufferInfo *fbInfo, unsigned long font) {
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebuffer);
	loadFont((char*)mmap->kernel + font);
	initConsole(fbInfo->framebufferWidth, fbInfo->framebufferHeight);
	setTextColours(0x00000000, 0x00FFAF00);
	printf("123456789101214161820222426283032343638404244464850525456586062X");
	printf("How much wood could a wood chuck chuck if a wood chuck chucked??");
	for (int i=3; i<64; i++) {
		printf("hi");
	}
	return;
}

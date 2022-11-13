struct memoryMap {
	unsigned long kernel;
	unsigned long code;
	unsigned long mmap;
	unsigned long stack;
	unsigned long heap;
	unsigned long* framebuffer;
	unsigned long framebufferSize;
} __attribute__ ((packed));

void wipeScreen(unsigned long *framebuf, unsigned long framebufSize, unsigned long colour) {
	for (unsigned long *ptr = framebuf; ptr<framebufSize / 4 + framebuf; ptr++) {
		//Pointer arithmetic means i++ is actually i=i+4 in a ulong
		*ptr = colour;
	}
	return;
}

int var = 5;
extern void kmain(struct memoryMap *mmap) {
	for (int i=0; i<var; i++) {
		wipeScreen(mmap->framebuffer, mmap->framebufferSize, 0x00FF0000);
		wipeScreen(mmap->framebuffer, mmap->framebufferSize, 0x000000FF);

	}
	return;
}

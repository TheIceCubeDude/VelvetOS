static unsigned short width;
static unsigned short height;
static unsigned short BPL;
static unsigned long size;
static unsigned long *trueFramebuffer;
static unsigned long *framebuffer;

static const unsigned int PIXEL_WIDTH = sizeof(unsigned long);
unsigned char doubleBuffering = 0;

void videoInit(unsigned short fbWidth, unsigned short fbHeight, unsigned short fbBPL, unsigned long fbSize, unsigned long* fb) {
	width = fbWidth;
	height = fbHeight;
	BPL = fbBPL;
	size = fbSize;
	trueFramebuffer = fb;
	framebuffer = fb;
	return;
}

void putPixel(unsigned short x, unsigned short y, unsigned long colour) {
	unsigned long *ptr = (unsigned long*) framebuffer + y*BPL/PIXEL_WIDTH + x;
	*ptr = colour;
	return;
}

unsigned long getPixel(unsigned short x, unsigned short y) {
	unsigned long *ptr = (unsigned long*) framebuffer + y*BPL/PIXEL_WIDTH + x;
	return *ptr;
}

void fillScreen(unsigned long colour) {
	memset((char*)framebuffer, colour, size);
	return;
}

void putScaledPixel(unsigned short x, unsigned short y, unsigned short scalex, unsigned short scaley, unsigned long colour) {
	unsigned long *ptr = (unsigned long*) framebuffer + y*scaley*BPL/PIXEL_WIDTH + x*scalex;
	for (int i=0; i<scaley; i++) {
		for (int j=0; j<scalex; j++) {
			*(ptr + j + (i*BPL/PIXEL_WIDTH)) = colour;
		}
	}
	return;
}

void enableDoubleBuffering() {
	framebuffer = (unsigned long*) malloc(size);
	doubleBuffering = 1;
	fillScreen(0);
	return;
}

void disableDoubleBuffering() {
	free((char*)framebuffer);
	framebuffer = trueFramebuffer;
	doubleBuffering = 0;
	return;
}

void swapBufs() {
	memcpy((char*)trueFramebuffer, (char*)framebuffer, size);
	return;
}

void _scrollY(unsigned int scale) {
	memcpy((char*)framebuffer, ((char*)framebuffer) + (scale * BPL), size - (scale * BPL));
	memset((char*)framebuffer + size - (scale * BPL), 0, scale * BPL);
	return;
}

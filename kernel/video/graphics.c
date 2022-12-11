//In pixels
unsigned short width;
unsigned short height;
unsigned short BPL;
unsigned long *framebuffer;

const unsigned int pixelWidth = sizeof(long);

void videoInit(unsigned short fbWidth, unsigned short fbHeight, unsigned short fbBPL, unsigned long* fb) {
	width = fbWidth;
	height = fbHeight;
	BPL = fbBPL;
	framebuffer = fb;
}

void putPixel(unsigned short x, unsigned short y, unsigned long colour) {
	unsigned long *ptr = (unsigned long*) framebuffer + y*BPL/pixelWidth + x;
	//if ((long)ptr > (long)(BPL/pixelWidth * height + framebuffer)) {return;}
	*ptr = colour;
	return;
}

unsigned long getPixel(unsigned short x, unsigned short y) {
	// TODO: DOUBLE BUFFERING TO SPEED THIS UP
	unsigned long *ptr = (unsigned long*) framebuffer + y*BPL/pixelWidth + x;
	return *ptr;
}

void fillScreen(unsigned long colour) {
	unsigned long *fbEnd = height*(BPL/pixelWidth) + framebuffer;
	for (unsigned long *i=framebuffer; i<=fbEnd; i++) {
		*i = colour;
	}
	return;
}

void _scrollY(unsigned int scale) {
	for (unsigned short i = 0; i<height - scale; i++) {
		for (unsigned short j = 0; j<BPL; j++) {
			putPixel(j, i, getPixel(j, i + scale));
		}
	}
	for (unsigned short i=height - scale; i<height; i++) {
		for (unsigned short j=0; j<width; j++) {
			putPixel(j, i, 0x00000000);
		}
	}
	return;
}

void putScaledPixel(unsigned short x, unsigned short y, unsigned short scalex, unsigned short scaley, unsigned long colour) {
	unsigned long *ptr = (unsigned long*) framebuffer + y*scaley*BPL/pixelWidth + x*scalex;
	for (int i=0; i<scaley; i++) {
		for (int j=0; j<scalex; j++) {
			*(ptr + j + (i*BPL/pixelWidth)) = colour;
		}
	}
}

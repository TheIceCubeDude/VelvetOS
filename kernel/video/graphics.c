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

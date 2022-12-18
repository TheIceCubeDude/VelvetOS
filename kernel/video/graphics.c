static uint16_t width;
static uint16_t height;
static uint16_t BPL;
static uint32_t size;
static uint32_t *trueFramebuffer;
static uint32_t *framebuffer;

static const uint8_t PIXEL_WIDTH = sizeof(uint32_t);
static uint8_t doubleBuffering = 0;

void videoInit(uint16_t fbWidth, uint16_t fbHeight, uint16_t fbBPL, uint32_t fbSize, uint32_t *fb) {
	width = fbWidth;
	height = fbHeight;
	BPL = fbBPL;
	size = fbSize;
	trueFramebuffer = fb;
	framebuffer = fb;
	return;
}

void putPixel(uint16_t x, uint16_t y, uint32_t colour) {
	uint32_t *ptr = (uint32_t*) framebuffer + y*BPL/PIXEL_WIDTH + x;
	*ptr = colour;
	return;
}

uint32_t getPixel(uint16_t x, uint16_t y) {
	uint32_t *ptr = (uint32_t*) framebuffer + y*BPL/PIXEL_WIDTH + x;
	return *ptr;
}

void fillScreen(uint32_t colour) {
	memset((uint8_t*)framebuffer, colour, size);
	return;
}

void putScaledPixel(uint16_t x, uint16_t y, uint16_t scalex, uint16_t scaley, uint32_t colour) {
	uint32_t *ptr = (uint32_t*) framebuffer + y*scaley*BPL/PIXEL_WIDTH + x*scalex;
	for (uint16_t i=0; i<scaley; i++) {
		for (uint16_t j=0; j<scalex; j++) {
			*(ptr + j + (i*BPL/PIXEL_WIDTH)) = colour;
		}
	}
	return;
}

void enableDoubleBuffering() {
	framebuffer = (uint32_t*) malloc(size);
	if (!framebuffer) {
		framebuffer = trueFramebuffer;
		kpanic("Not enough space on heap to allocate a framebuffer!");
	}
	doubleBuffering = 1;
	fillScreen(0);
	return;
}

void disableDoubleBuffering() {
	free(framebuffer);
	framebuffer = trueFramebuffer;
	doubleBuffering = 0;
	return;
}

void swapBufs() {
	memcpy(trueFramebuffer, framebuffer, size);
	return;
}

void _scrollY(uint16_t scale) {
	memcpy(framebuffer, ((char*)framebuffer) + (scale * BPL), size - (scale * BPL));
	memset((char*)framebuffer + size - (scale * BPL), 0, scale * BPL);
	return;
}

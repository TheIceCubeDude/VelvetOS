#define TRANSPARENT_COLOUR 0x80000000

struct psf {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t numGlyph;
	uint32_t bytesPerGlyph;
	uint32_t height, width;
} __attribute__ ((packed));

static struct psf *rawFont;
static uint8_t *glyphs;
static uint32_t bgColour;
static uint32_t fgColour;

static uint16_t cursorX = 0;
static uint16_t cursorY = 0;
static const uint16_t CONSOLE_WIDTH = 64;
static const uint16_t CONSOLE_HEIGHT = 32;
static uint16_t CONSOLE_SCALE_X;
static uint16_t CONSOLE_SCALE_Y;

void loadFont(void *font) {
	rawFont = (struct psf*)font;
	//Check valid .psf v2 font
	if (!(rawFont->magic == 0x864ab572)) {
		halt();
	}
	//Check 8*17
	if (!(rawFont->width==8 && rawFont->height==17)) {
		halt();
	}
	glyphs = (uint8_t*)rawFont + rawFont->headerSize;
	return;
}

void consoleInit(uint32_t width, uint32_t height) {
	CONSOLE_SCALE_X = width / (CONSOLE_WIDTH * rawFont->width);
	CONSOLE_SCALE_Y = height / (CONSOLE_HEIGHT * rawFont->height);
	return;
}

void resetCursor() {
	cursorX = 0;
	cursorY = 0;
	return;
}

void putChar(uint8_t *character, uint16_t x, uint16_t y, uint16_t scalex, uint16_t scaley) {
	for (uint16_t i=0; i<rawFont->height; i++) {
		for (uint16_t j=0; j<rawFont->width; j++) {
			if (*(glyphs + i + *character * rawFont->bytesPerGlyph) & 0x80>>j) {
				putScaledPixel(j + x, i + y, scalex, scaley, fgColour);
			} else if (!(bgColour == TRANSPARENT_COLOUR)) {
				putScaledPixel(j + x, i + y, scalex, scaley, bgColour);
			}
		}
	}
	return;	
}

void printf(uint8_t *string) {
	uint8_t *character = string;
	while(*character) {
		if (cursorY == CONSOLE_HEIGHT) {
			_scrollY(rawFont->height * CONSOLE_SCALE_Y);
			cursorY--;
		}
		putChar(character, cursorX * rawFont->width, cursorY * rawFont->height, CONSOLE_SCALE_X, CONSOLE_SCALE_Y);
		cursorX++;
		if (cursorX == CONSOLE_WIDTH) {
			cursorX = 0;
			cursorY++;
		}
		character++;
	}
	cursorY++;
	cursorX = 0;
	if (doubleBuffering) {swapBufs();}
	return;
}

void printHex(uint32_t value) {
	uint8_t reverseString[32];
	uint16_t counter = 0;

	do {
		counter++;
		uint32_t oldVal = value;
		value /= 16;
		reverseString[32 - counter] = "0123456789abcdef"[oldVal - (value * 16)];
	} while (value);

	counter += 2;
	char string[counter];
	for (uint16_t i=2; i<counter; i++) {
		string[i] = reverseString[i + (32 - counter)];
	}
	string[counter] = 0;
	string[0] = *"0";
	string[1] = *"x";
	printf((uint8_t*)&string);
	return;
}

void printDec(uint32_t value) {
	uint8_t reverseString[32];
	uint16_t counter = 0;

	do {
		counter++;
		uint32_t oldVal = value;
		value /= 10;
		reverseString[32 - counter] = "0123456789"[oldVal - (value * 10)];
	} while (value);

	uint8_t string[counter];
	for (uint16_t i=0; i<counter; i++) {
		string[i] = reverseString[i + (32 - counter)];
	}
	string[counter] = 0;
	printf((uint8_t*)&string);
	return;
}

//If bg is 0x80000000 it will be transparent
void setTextColours(uint32_t bg, uint32_t fg) {
	bgColour = bg;
	fgColour = fg;
	return;
}

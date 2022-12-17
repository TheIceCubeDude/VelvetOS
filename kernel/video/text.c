#define TRANSPARENT_COLOUR 0x80000000

struct psf {
	unsigned long magic;
	unsigned long version;
	unsigned long headerSize;
	unsigned long flags;
	unsigned long numGlyph;
	unsigned long bytesPerGlyph;
	unsigned long height, width;
} __attribute__ ((packed));

static struct psf *rawFont;
static unsigned char *glyphs;
static unsigned long bgColour;
static unsigned long fgColour;

static unsigned short cursorX = 0;
static unsigned short cursorY = 0;
static const unsigned short CONSOLE_WIDTH = 64;
static const unsigned short CONSOLE_HEIGHT = 32;
static unsigned short CONSOLE_SCALE_X;
static unsigned short CONSOLE_SCALE_Y;

void loadFont(char *font) {
	rawFont = (struct psf*)font;
	//Check valid .psf v2 font
	if (!(rawFont->magic == 0x864ab572)) {
		fillScreen(0x00FF0000);
		return;
	}
	//Check 8*17
	if (!(rawFont->width==8 && rawFont->height==17)) {
		fillScreen(0x000000FF);
		return;
	}
	glyphs = (unsigned char*)rawFont + rawFont->headerSize;
	return;
}

void consoleInit(unsigned long width, unsigned long height) {
	CONSOLE_SCALE_X = width / (CONSOLE_WIDTH * rawFont->width);
	CONSOLE_SCALE_Y = height / (CONSOLE_HEIGHT * rawFont->height);
	return;
}

void resetCursor() {
	cursorX = 0;
	cursorY = 0;
	return;
}

static void putChar(char *character, unsigned short x, unsigned short y, unsigned short scalex, unsigned short scaley) {
	for (short i=0; i<rawFont->height; i++) {
		for (short j=0; j<rawFont->width; j++) {
			if (*(glyphs + i + *character * rawFont->bytesPerGlyph) & 0x80>>j) {
				putScaledPixel(j + x, i + y, scalex, scaley, fgColour);
			} else if (!(bgColour == TRANSPARENT_COLOUR)) {
				putScaledPixel(j + x, i + y, scalex, scaley, bgColour);
			}
		}
	}
	return;	
}

void printf(char *string) {
	char *character = string;
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

void printHex(unsigned long value) {
	char reverseString[32];
	int counter = 0;

	do {
		counter++;
		unsigned long oldVal = value;
		value /= 16;
		reverseString[32 - counter] = "0123456789abcdef"[oldVal - (value * 16)];
	} while (value);

	counter += 2;
	char string[counter];
	for (int i=2; i<counter; i++) {
		string[i] = reverseString[i + (32 - counter)];
	}
	string[counter] = 0;
	string[0] = *"0";
	string[1] = *"x";
	printf((char*)&string);
	return;
}

void printDec(unsigned long value) {
	char reverseString[32];
	int counter = 0;

	do {
		counter++;
		unsigned long oldVal = value;
		value /= 10;
		reverseString[32 - counter] = "0123456789"[oldVal - (value * 10)];
	} while (value);

	char string[counter];
	for (int i=0; i<counter; i++) {
		string[i] = reverseString[i + (32 - counter)];
	}
	string[counter] = 0;
	printf((char*)&string);
	return;
}

//If bg is 0x80000000 it will be transparent
void setTextColours(unsigned long bg, unsigned long fg) {
	bgColour = bg;
	fgColour = fg;
	return;
}

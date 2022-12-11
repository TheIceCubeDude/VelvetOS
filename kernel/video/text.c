struct psf {
	unsigned long magic;
	unsigned long version;
	unsigned long headerSize;
	unsigned long flags;
	unsigned long numGlyph;
	unsigned long bytesPerGlyph;
	unsigned long height, width;
} __attribute__ ((packed));

struct psf *rawFont;
unsigned char *glyphs;
unsigned long bgColour;
unsigned long fgColour;

unsigned short cursorX = 0;
unsigned short cursorY = 0;
const unsigned short CONSOLE_WIDTH = 64;
const unsigned short CONSOLE_HEIGHT = 32;
unsigned short CONSOLE_SCALE_X;
unsigned short CONSOLE_SCALE_Y;

void loadFont(char *font) {
	rawFont = (struct psf*)font;
	//Check valid .psf v2 font
	if (!rawFont->magic == 0x864ab572) {
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

void initConsole(unsigned long width, unsigned long height) {
	CONSOLE_SCALE_X = width / (CONSOLE_WIDTH * rawFont->width);
	CONSOLE_SCALE_Y = height / (CONSOLE_HEIGHT * rawFont->height);
	return;
}

void putChar(char *character, unsigned short x, unsigned short y, unsigned short scalex, unsigned short scaley) {
	for (short i=0; i<rawFont->height; i++) {
		for (short j=0; j<rawFont->width; j++) {
			if (*(glyphs + i + *character * rawFont->bytesPerGlyph) & 0x80>>j) {
				putScaledPixel(j + x, i + y, scalex, scaley, fgColour);
			} else {
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
	return;
}

void setTextColours(unsigned long bg, unsigned long fg) {
	bgColour = bg;
	fgColour = fg;
}

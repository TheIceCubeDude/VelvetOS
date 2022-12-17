void videoInit(uint16_t fbWidth, uint16_t fbhHeight, uint16_t fbBPL, uint32_t fbSize, uint32_t *fb);
void putPixel(uint16_t x, uint16_t y, uint32_t colour);
uint32_t getPixel(uint16_t x, uint16_t y);
void fillScreen(uint32_t colour);
void putScaledPixel(uint16_t x, uint16_t y, uint16_t scalex, uint16_t scaley, uint32_t colour);
void enableDoubleBuffering();
void disableDoubleBuffering();
void swapBufs();

void loadFont(void *font);
void consoleInit(uint32_t width, uint32_t height);
void resetCursor();
void printf(uint8_t *string);
void printHex(uint32_t value);
void printDec(uint32_t value);
void setTextColours(uint32_t bg, uint32_t fg);

#define TRANSPARENT_COLOUR 0x80000000

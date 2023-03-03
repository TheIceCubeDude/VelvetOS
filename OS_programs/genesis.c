#include "libsyscall.h"

void _start() {
	fillScreen(0x00FF00FF);
	printf("hello");
	yield();
	return;
}

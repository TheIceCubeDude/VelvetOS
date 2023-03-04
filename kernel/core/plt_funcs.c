#include "stdint.h"
#include "include.h"
#include "../disk/include.h"
#include "../pit/include.h"
#include "../ps2/include.h"
#include "../video/include.h"

const uint32_t PLT_FUNCS[] = {
	//NOTE: functions in this compilation unit may be resolved to base 1048676. If so, just subtract from 1048576
	//Ignore above statement. This function list has been put in a seperate compilation unit to fix this issue.
	
	//Video
       	((uint32_t) putPixel),
	((uint32_t) getPixel),
	((uint32_t) fillScreen),
	((uint32_t) putScaledPixel),
	((uint32_t) swapBufs),
	((uint32_t) setCursorX),
	((uint32_t) setCursorY),
	((uint32_t) getCursorX),
	((uint32_t) getCursorY),
	((uint32_t) printf),
	((uint32_t) printHex),
	((uint32_t) printDec),
	((uint32_t) setTextColours),
	//PS2
	((uint32_t) getNewKeys),
	((uint32_t) getPressedKeys),
	//PIT/PC speaker
	((uint32_t) resetTicks),
	((uint32_t) getTicks),
	((uint32_t) sleep),
	((uint32_t) playFreq),
	((uint32_t) enableSpeaker),
	((uint32_t) disableSpeaker),
	((uint32_t) playSound),
	//ATA PIO disk / USTAR FS
	((uint32_t) pioReadDisk),
	((uint32_t) pioWriteDisk),
	((uint32_t) pioGetDrives),
	((uint32_t) ustarReadFile),
	((uint32_t) ustarGetDirectoryContents),
	((uint32_t) ustarSetStart),
	((uint32_t) ustarSetDisk),
	((uint32_t) ustarFindFile),
	//Core
	((uint32_t) memcpy),
	((uint32_t) memset),
	((uint32_t) malloc),
	((uint32_t) free),
	((uint32_t) addProcess),
	((uint32_t) destroyProcess),
	((uint32_t) yield),
	((uint32_t) addInterrupt)
};


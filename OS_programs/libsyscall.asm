;; Puts a 'PLT' at the .plt section of the ELF
SECTION .plt

global _start

start:

putPixel: dd 0
getPixel: dd 0
fillScreen: dd 0
putScaledPixel: dd 0
swapBufs: dd 0
setCursorX: dd 0
setCursorY: dd 0
getCursorX: dd 0
getCursorY: dd 0
printf: dd 0
printHex: dd 0
printDec: dd 0
setTextColours: dd 0

getNewKeys: dd 0
getPressedKeys: dd 0

resetTicks: dd 0
getTicks: dd 0
sleep: dd 0
playFreq: dd 0
enableSpeaker: dd 0
disableSpeaker: dd 0
playSound: dd 0

pioReadDisk: dd 0
pioWriteDisk: dd 0
pioGetDrives: dd 0
ustarReadFile: dd 0
ustarGetDirectoryContents: dd 0
ustarSetStart: dd 0
ustarSetDisk: dd 0
ustarFindFile: dd 0

memcpy: dd 0
memset: dd 0
malloc: dd 0
free: dd 0
addProcess: dd 0
destroyProcess: dd 0
yield: dd 0
addInterrupt: dd 0

end:

;;PLT size
dd end - start

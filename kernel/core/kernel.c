struct memoryMap {
	void *mmap;
	void *kernel;
	uint32_t kernelSize;
	void *code;
	uint32_t codeSize;
	void *stack;
	uint32_t stackSize;
	void *heap;
	uint32_t heapSize;
} __attribute__ ((packed));
struct framebufferInfo {
	uint32_t *framebuffer;
	uint32_t  framebufferSize;
	uint16_t framebufferWidth;
	uint16_t framebufferHeight;
	uint16_t framebufferBPL;
} __attribute__ ((packed));

static struct memoryMap *mmap;
static struct framebufferInfo *fbInfo;

void kpanic(uint8_t* cause) {
	//disableIrqs();
	setTextColours(0x00FF0000, 0x00FFFFFF);
	printf("Kernel Panic!!!");
	setTextColours(0x00A00000, 0x00FFFFFF);
	printf("Kernel offset & size:");
	printHex((uint32_t) mmap->kernel);
	printHex(mmap->kernelSize);
	printf("Stack offset & size:");
	printHex((uint32_t) mmap->stack);
	printHex(mmap->stackSize);
	printf("Heap offset & size:");
	printHex((uint32_t) mmap->heap);
	printHex(mmap->heapSize);
	printf("Program code offset & size:");
	printHex((uint32_t) mmap->code);
	printHex(mmap->codeSize);
	printf("VESA VBE framebuffer offset & size:");
	printHex((uint32_t)fbInfo->framebuffer);
	printHex(fbInfo->framebufferSize);
	printf("VESA VBE framebuffer width, height and Bytes Per Line (BPL)");
	printDec(fbInfo->framebufferWidth);
	printDec(fbInfo->framebufferHeight);
	printDec(fbInfo->framebufferBPL);
	printf("Cause of panic:");
	printf(cause);
	halt();
}

void _singleBufPrint() {
	setTextColours(0x000000FF, 0x00FFFF00);
	printf("VelvetOS v0.1 kernel now booting...");
	setTextColours(0x00472F1F, 0x00FFAF00);
	printf("Core graphics loaded.");
	return;
}

uint8_t findBootDisks() {
	uint8_t drives[4];
	pioGetDrives(drives);
	uint8_t bootDrives[4] = {0};
	uint8_t bootDrivesCount = 0;
	uint8_t selectedDrive;
	for (uint8_t i=0; i<4; i++) {
		if (drives[i]) {
			uint16_t mbr[256];
			pioReadDisk(0, 0, 1, i, mbr);
			uint32_t partition3 = mbr[486/2];
			uint8_t file[512];
			pioReadDisk(0, partition3, 1, i, (uint16_t*) file);
			if (*(file + 257) == *"ustar") {bootDrives[i] = 1; bootDrivesCount++; selectedDrive = i;}
		}
	}
	if (!bootDrivesCount) {
		kpanic("OS was not booted from an ATA drive!");
	}
	if (bootDrivesCount > 1) {
		printf("");
		printf("NOTE:");
		printf("Multiple drives contain VelvetOS systems.");
		printf("Drives available:");
		for (uint8_t i=0; i<4; i++) {if (bootDrives[i]) {printDec(i);}}
		printf("Which drive do you want to use for the rootFS? Type number now.");
		uint8_t keys[32] = {0};
		while ((!keys[0]) || !(keys[0] < 0x34 && keys[0] >= 0x30 &&  bootDrives[keys[0] - 0x30])) {getPressedKeys(keys);}
		keys[1] = 0;
		selectedDrive = keys[0] - 0x30;
	}

	printf("Boot drive:");
	printDec(selectedDrive);

	uint16_t mbr[256];
	pioReadDisk(0, 0, 1, selectedDrive, mbr);
	uint32_t partition3 = mbr[486/2];
	ustarSetDisk(selectedDrive);
	ustarSetStart(partition3);

	return selectedDrive;
}

void loadGenesis() {
	uint8_t bootDisk = findBootDisks();
	FILE genesis = ustarFindFile("|/genesis.ebin");
	if (!genesis.location) {
		printf("\'genesis.ebin\' not found! Select program to load.");
		uint8_t string[100] = {'>', '|', '/', 0};
		uint8_t stringptr = 3;
		uint8_t ctrl = 1;
		setCursorY(getCursorY() + 1);
		while (ctrl) {
			setCursorY(getCursorY() - 1);
			printf(string);
			uint8_t keys[32];
			getNewKeys(keys);
			for (uint8_t i=0; i<32; i++) {
				if (keys[i] == '\b' && (!(stringptr == 1))) {stringptr--;}
				else if (keys[i] == '\n') {ctrl = 0; setCursorY(getCursorY() + 1);}
				else if (keys[i] && !(keys[i] == '\b')) {string[stringptr] = keys[i]; stringptr++;}
			}
			string[stringptr] = '_';
			string [stringptr + 1] = ' ';
		}
		setCursorY(getCursorY() - 1);
		string[stringptr] = 0;
		string[stringptr + 1] = 0;
		
		genesis = ustarFindFile(string+1);
		if (genesis.type == 5) {
			printf("That is a directory! Contents:");
			uint32_t i = 0;
			while (1)  {
				uint8_t *item = ustarGetDirectoryContents(string+1, i);
				if (!item) {break;}
				printf(item);
				free(item);
				i++;
			}
			printf("Reboot to retry.");
			halt();
		}
		if (!genesis.location) {printf("File does not exist! (Remember: directories must end in \'/\', and if you enter in a directory its contents will be shown. Reboot to retry."); halt();}
	}
	struct process *genesisProcess = addProcess(genesis.size, 4096);
	if (!genesisProcess) {kpanic("Not enough memory to load genesis program!");}
	ustarReadFile(genesis, genesisProcess->code);

	//Test more than 1 proc
	FILE test = ustarFindFile("|/tetrisTheme.ebin");
	struct process *testProcess = addProcess(test.size, 4096);
	if (!testProcess) {kpanic("Not enough memory to load test program!");}
	ustarReadFile(test, testProcess->code);
}

extern void kmain(struct memoryMap *mmapParam, struct framebufferInfo *fbInfoParam, void *font) {
	//Setup some base stuff
	mmap = mmapParam;
	fbInfo = fbInfoParam;
	setHeap(mmap->code);
	makeHeap(mmap->codeSize);
	setHeap(mmap->heap);
	makeHeap(mmap->heapSize);

	//Init graphics
	videoInit(fbInfo->framebufferWidth, fbInfo->framebufferHeight, fbInfo->framebufferBPL, fbInfo->framebufferSize, fbInfo->framebuffer);
	consoleInit((void*) ((uint32_t)mmap->kernel + (uint32_t)font));
	_singleBufPrint();
	if(!enableSSE()) {kpanic("CPU does not support SSE 2.0");}
	printf("SSE enabled.");
	enableDoubleBuffering();
	setCursorX(0);
	setCursorY(0);
	fillScreen(0x00472F1F);
	//Print stuff from before to get framebuf contents into backbuf
	//(we can't copy from framebuf to backbuf because it is sloooow)
	_singleBufPrint();
	printf("SSE enabled.");
	printf("Double buffering enabled.");

	//Init interrupts
	idtInit();
	printf("Interrupts have been set up.");
	//Init Programmable Interval Timer
	initPit();
	printf("PIT has been set up.");

	//Init PS/2 controller, ports and devices
	initPs2();
	printf("PS/2 hardware initalised.");
	enableIrqs();

	//Init ATA hard disks for PIO mode, and check there is a valid ustar fs on the 3rd partition of one of them
	initAtaPio();
	printf("ATA hard disks initalised for PIO mode.");

	printf("Executing genesis program...");
	loadGenesis();
	exitKernel();
}



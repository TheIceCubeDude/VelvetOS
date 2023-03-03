static uint32_t sector;
static uint32_t disk;

void ustarReadFile(FILE file, void *buf) {
	//Calculate size in sectors (round up)
	if (!file.size) {return;}
	uint32_t size = 0;
	if (file.size % 512) {size++;};
	size += file.size/512;
	//Get file data (discard empty data because it is rounded up to the nearest sector)
	void* tmpBuf = malloc(size*512);
	pioReadDisk(0, file.location + 1, size, disk, tmpBuf);
	memcpy(buf, tmpBuf, file.size);
	free(tmpBuf);
	return;
}

uint8_t* ustarGetDirectoryContents(uint8_t *directory, uint32_t index) {
	uint32_t location = sector;
	uint32_t currentIndex = 0;
	uint32_t dirSlashCount = 0;
	//Get the amount of slash's in this directory name
	uint8_t *dirPtr = directory;
	while (*dirPtr) {
		if (*dirPtr == '/') {dirSlashCount++;}
		dirPtr++;
	}
	//Directories MUST end in /
	if (!(*(dirPtr-1) == '/')) {return 0;}
	while (1) {
		uint8_t fileInfo[512];
		pioReadDisk(0, location, 1, disk, (uint16_t*)fileInfo);
		//Check we haven't reached the end
		uint8_t sig[] = "ustar";
		for (uint8_t i=0; i<6; i++) {
			if (!(fileInfo[257 + i] == sig[i])) {return 0;}
		}
		uint32_t slashCount = 0;
		//Get the amount of slashes in file name(ignore the one it ends in, if it does end with one)
		for (uint8_t i=0; i<100; i++) {
			if (!fileInfo[i]) {break;}
			if (fileInfo[i] == '/' && fileInfo[i + 1])  {slashCount++;} 
		}
		//Check if this file's name begins with the directory and that it is a direct child (not descendant - based on slash count)
		uint32_t size = oct2bin(&fileInfo[124], 11);
		uint8_t type = oct2bin(&fileInfo[156], 1) - '0';
		uint8_t matching = 1;
		for (uint8_t i=0; i<100; i++) {
			if (!directory[i]) {break;}
			if (!(fileInfo[i] == directory[i])) {matching = 0; break;}
		}
		if (matching && slashCount == dirSlashCount) {
			if (currentIndex == index) {
				//Get the name of the file
				uint8_t *name = malloc(100);
				for (uint8_t i=0; i<100; i++) {
					name[i] = fileInfo[i];
					if (!fileInfo[i]) {break;}
				}
				return name;
			}
			currentIndex++;
		}
		//Skip the size of this file to get the metadata of the next
		//Convert to sectors and round up
		if (size) {
			if (size % 512) {location++;};
			location += size/512;
		}
		location++;
	}
}

void ustarSetStart(uint32_t inSector) {
	sector = inSector;
	return;
}

void ustarSetDisk(uint8_t inDisk) {
	disk = inDisk;
	return;
}

FILE ustarFindFile(uint8_t *filename)  {
	uint32_t location = sector;
	while (1) {
		uint8_t fileInfo[512];
		pioReadDisk(0, location, 1, disk, (uint16_t*)fileInfo);
		//Check we haven't reached the end
		uint8_t sig[] = "ustar";
		for (uint8_t i=0; i<6; i++) {
			if (!(fileInfo[257 + i] == sig[i])) {FILE file = {0, 0, 0}; return file;}
		}
		//Check if this is the file
		uint32_t size = oct2bin(&fileInfo[124], 11);
		uint8_t type = oct2bin(&fileInfo[156], 1);
		uint8_t matching = 1;
		for (uint8_t i=0; i<100; i++) {
			if ((!fileInfo[i]) && (!filename[i])) {break;}
			if (!(fileInfo[i] == filename[i])) {matching = 0; break;}
		}
		if (matching) {FILE file = {location, size, type}; return file;}
		else {
			//Skip the size of this file to get the metadata of the next
			//Convert to sectors and round up
			if (size) {
				if (size % 512) {location++;};
				location += size/512;
			} 
			location++;
		}
	}
}

uint32_t oct2bin(uint8_t *string, uint8_t size) {
	uint32_t number = 0;
	for (uint8_t i=0; i<size; i++) {
		uint8_t digit = string[i] - '0';
		number *= 8;
		number += digit;
	}
	return number;
}

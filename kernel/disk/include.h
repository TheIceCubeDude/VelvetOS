void initAtaPio();
uint8_t pioCheckDrive(uint16_t bus, uint8_t drive);
void pioReadDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf);
void pioWriteDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf);
void pioGetDrives(uint8_t *buf);

typedef struct {
	uint32_t location;
	uint32_t size;
	uint8_t type;
} FILE;

void ustarReadFile(FILE file, void* buf);
uint8_t* ustarGetDirectoryContents(uint8_t *directory, uint32_t index);
void ustarSetStart(uint32_t inSector);
void ustarSetDisk(uint8_t inDisk);
FILE ustarFindFile(uint8_t *filename);
uint32_t oct2bin(uint8_t *string, uint8_t size);

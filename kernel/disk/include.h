void initAtaPio();
uint8_t pioCheckDrive(uint16_t bus, uint8_t drive);
void pioReadDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf);
void pioWriteDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf);
void pioGetDrives(uint8_t *buf);

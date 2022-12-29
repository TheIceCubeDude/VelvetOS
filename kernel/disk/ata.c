//Base I/O ports
#define PRIMARY_BUS_IO 0x1F0
#define SECONDARY_BUS_IO 0x170
#define PRIMARY_BUS_CONTROL 0x3F6
#define SECONDARY_BUS_CONTROL 0x376

//IO base registers
#define DATA_REGISTER 0
#define ERR_REGISTER 1
#define FEATURES_REGISTER 1
#define SEC_COUNT_REGISTER 2
#define LBA_LO 3
#define LBA_MID 4
#define LBA_HI 5
#define DRIVE_REGISTER 6
#define STATUS_REGISTER 7
#define COMMAND_REGISTER 7

//Control base registers
#define STATUS_REGISTER_NOIRQ 0
#define DEVICE_ADDRESS_REGISTER 1
#define DEVICE_CONTROL_REGISTER 1

//Generic values and commands
#define MASTER_DRIVE_IDENTIFY 0xA0
#define SLAVE_DRIVE_IDENTIFY 0xB0
#define MASTER_DRIVE_ACCESS 0x40
#define SLAVE_DRIVE_ACCESS 0x50
#define IDENTIFY_COMMAND 0xEC
#define READ_SECTORS_48_COMMAND 0x24
#define WRITE_SECTORS_48_COMMAND 0x34
#define CACHE_FLUSH_COMMAND 0xE7

static uint8_t drivesAvailable[4] = {1, 1, 1, 1};

void initAtaPio() {
	//Crude check if busses have drives on them (0xFF means no drives - floating bus)
	if(inb(PRIMARY_BUS_IO + STATUS_REGISTER) == 0xFF) {drivesAvailable[0] = 0; drivesAvailable[1] = 0;}
	if(inb(SECONDARY_BUS_IO + STATUS_REGISTER) == 0xFF) {drivesAvailable[2] = 0; drivesAvailable[3] = 0;}

	//Proper check for hard drives using IDENTIFY command
	if (drivesAvailable[0] && (!pioCheckDrive(PRIMARY_BUS_IO, MASTER_DRIVE_IDENTIFY))) {drivesAvailable[0] = 0;}
	if (drivesAvailable[1] && (!pioCheckDrive(PRIMARY_BUS_IO, SLAVE_DRIVE_IDENTIFY))) {drivesAvailable[1] = 0;}
	if (drivesAvailable[2] && (!pioCheckDrive(SECONDARY_BUS_IO, MASTER_DRIVE_IDENTIFY))) {drivesAvailable[2] = 0;}
	if (drivesAvailable[3] && (!pioCheckDrive(SECONDARY_BUS_IO, SLAVE_DRIVE_IDENTIFY))) {drivesAvailable[3] = 0;}

	return;
}

void pioReadDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf) {
	if (drive > 3 || (!drivesAvailable[drive])) {return;}
	uint16_t bus;
	if 	(drive == 0) {bus = PRIMARY_BUS_IO; drive = MASTER_DRIVE_ACCESS;}
	else if (drive == 1) {bus = PRIMARY_BUS_IO; drive = SLAVE_DRIVE_ACCESS;}
	else if (drive == 2) {bus = SECONDARY_BUS_IO; drive = MASTER_DRIVE_ACCESS;}
	else if (drive == 3) {bus = SECONDARY_BUS_IO; drive = SLAVE_DRIVE_ACCESS;}
	
	outb(bus + DRIVE_REGISTER, drive);
	//Send high bytes
	outb(bus + SEC_COUNT_REGISTER, (uint8_t) (sectorCount >> 8) & 0xFF);
	outb(bus + LBA_LO, (uint8_t) (blockLo >> 24) & 0xFF);
	outb(bus + LBA_MID, (uint8_t) (blockHi) & 0xFF);
	outb(bus + LBA_HI, (uint8_t) (blockHi >> 8) & 0xFF);
	//Send low bytes
	outb(bus + SEC_COUNT_REGISTER, (uint8_t) sectorCount & 0xFF);
	outb(bus + LBA_LO, (uint8_t) (blockLo) & 0xFF);
	outb(bus + LBA_MID, (uint8_t) (blockLo >> 8) & 0xFF);
	outb(bus + LBA_HI, (uint8_t) (blockLo >> 16) & 0xFF);
	
	outb(bus + COMMAND_REGISTER, READ_SECTORS_48_COMMAND);

	//Poll for each sector transferred
	for (uint16_t i=0; i<sectorCount; i++) {
		while (!(inb(bus + STATUS_REGISTER) & 8)) {
			//Check for errors
			if (inb(bus + ERR_REGISTER)) {kpanic("Error reading sectors from ATA hard disk via PIO!");}
		}
		insw(bus + DATA_REGISTER, buf, 256);
		buf += 256;
	}

	return;
}

void pioWriteDisk(uint16_t blockHi, uint32_t blockLo, uint16_t sectorCount, uint8_t drive, uint16_t *buf) {
	if (drive > 3 || (!drivesAvailable[drive])) {return;}
	uint16_t bus;
	if 	(drive == 0) {bus = PRIMARY_BUS_IO; drive = MASTER_DRIVE_ACCESS;}
	else if (drive == 1) {bus = PRIMARY_BUS_IO; drive = SLAVE_DRIVE_ACCESS;}
	else if (drive == 2) {bus = SECONDARY_BUS_IO; drive = MASTER_DRIVE_ACCESS;}
	else if (drive == 3) {bus = SECONDARY_BUS_IO; drive = SLAVE_DRIVE_ACCESS;}
	
	outb(bus + DRIVE_REGISTER, drive);
	//Send high bytes
	outb(bus + SEC_COUNT_REGISTER, (uint8_t) (sectorCount >> 8) & 0xFF);
	outb(bus + LBA_LO, (uint8_t) (blockLo >> 24) & 0xFF);
	outb(bus + LBA_MID, (uint8_t) (blockHi) & 0xFF);
	outb(bus + LBA_HI, (uint8_t) (blockHi >> 8) & 0xFF);
	//Send low bytes
	outb(bus + SEC_COUNT_REGISTER, (uint8_t) sectorCount & 0xFF);
	outb(bus + LBA_LO, (uint8_t) (blockLo) & 0xFF);
	outb(bus + LBA_MID, (uint8_t) (blockLo >> 8) & 0xFF);
	outb(bus + LBA_HI, (uint8_t) (blockLo >> 16) & 0xFF);
	
	outb(bus + COMMAND_REGISTER, WRITE_SECTORS_48_COMMAND);

	//Poll for each sector transferred
	for (uint16_t i=0; i<sectorCount; i++) {
		while (!(inb(bus + STATUS_REGISTER) & 8)) {
			//Check for errors
			if (inb(bus + ERR_REGISTER)) {kpanic("Error writing sectors to ATA hard disk via PIO!");}
		}
		//We cannot use REP OUTSW
		for (uint16_t i=0; i<256; i++) {
			outw(bus + DATA_REGISTER, *(buf + i));
		}
		buf += 256;
	}

	outb(bus + COMMAND_REGISTER, CACHE_FLUSH_COMMAND);
	while (inb(bus + STATUS_REGISTER) & 128) {}
	return;
}

void pioGetDrives(uint8_t *buf) {
	for (uint8_t i=0; i<sizeof(drivesAvailable); i++) {
		*(buf + i) = drivesAvailable[i];
	}
	return;
}	

//Check if a drive supports 48bit ATA LBA PIO mode
uint8_t pioCheckDrive(uint16_t bus, uint8_t drive) {
	uint16_t buf[256];
	//Issue command
	outb(bus + DRIVE_REGISTER, drive);
	outb(bus + SEC_COUNT_REGISTER, 0);
	outb(bus + LBA_LO, 0);
	outb(bus + LBA_MID, 0);
	outb(bus + LBA_HI, 0);
	outb(bus + COMMAND_REGISTER, IDENTIFY_COMMAND);
	
	//Check status
	if (inb(bus + STATUS_REGISTER)) {
		//Poll until no longer BSY
		while (inb(bus + STATUS_REGISTER) & 0x80) {}
		//ATAPI and SATA are sometimes implemented incorrectly, so we check if they modified these values
		if (inb(bus + LBA_MID) || inb(bus + LBA_HI)) {return 0;}
		//Poll until we get a drive ERR or DRQ
		while (!(inb(bus + STATUS_REGISTER) & 8)) {if (!(inb(bus + STATUS_REGISTER) & 1)) {return 0;}}

		insw(bus + DATA_REGISTER, buf, 256);
		//Bit 10 means 48 bit mode supported
		if(buf[83] & 0x1024) {return 1;}
	}

	return 0;
}

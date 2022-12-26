#define DATA_PORT 0x60
#define CONTROL_PORT 0x64

static uint8_t port2Exists = 0;

//NOTE: PCI IRQ's must be configured to NOT use IRQ 11 in BIOS (at least on older IBM/Lenovo Thinkpads)

void initPs2() {
	//Disable first port
	outb(CONTROL_PORT, 0xAD);
	//Disable second port (ignored if doesn't exist)
	outb(CONTROL_PORT, 0xA7);

	//Flush the output buffer
	inb(DATA_PORT);

	//Get config byte
	outb(CONTROL_PORT, 0x20);
	pollToRead();
	uint8_t configByte = inb(DATA_PORT);
	//Disable IRQs and translation for the initalisation (clear bits 0, 1 & 6)
	configByte &= 0b10111100;
	outb(CONTROL_PORT, 0x60);
	pollToWrite();
	outb(DATA_PORT, configByte);
	//Check if there is a 2nd port (it's clock should be disabled - bit 5 should be set)
	if (configByte & 0b00100000) {port2Exists = 1;}

	//Do a self test on the controller
	outb(CONTROL_PORT, 0xAA);
	pollToRead();
	if (!(inb(DATA_PORT) == 0x55)) {kpanic("PS/2 controller failed self-test!");}
	//Restore the config byte incase the self test reset the controller
	outb(CONTROL_PORT, 0x60);
	pollToWrite();
	outb(DATA_PORT, configByte);
	
	//Do a self test on the first port
	outb(CONTROL_PORT, 0xAB);
	pollToRead();
	if (inb(DATA_PORT)) {kpanic("PS/2 first port failed self-test!");}
	//Do a self test on the second port (if it exists)
	if (port2Exists) {
		outb(CONTROL_PORT, 0xA9);
		pollToRead();
		if (inb(DATA_PORT)) {kpanic("PS/2 second port failed self-test!");}
	}

	//Enable first port
	outb(CONTROL_PORT, 0xAE);
	//Enable second port (if it exists)
	if(port2Exists) {outb(CONTROL_PORT, 0xA8);}
	//Enable interrupts for each port
	outb(CONTROL_PORT, 0x60);
	pollToWrite();
	if (port2Exists) {outb(DATA_PORT, configByte | 3);} 
	else {outb(DATA_PORT, configByte | 1);}

	//Reset and self test device on first port (no response means no device, so skip)
	if (port1Command(0xFF, 0)) {
		pollToRead();
		uint8_t response = inb(DATA_PORT);
		if (!(response == 0xAA || response == 0xFA)) {kpanic("Device on PS/2 first port failed self-test!");}
	}
	//Reset and self test device on second port (no response mans no device, so skip)
	if (port2Exists && port2Command(0xFF, 0)) {
		pollToRead();
		uint8_t response = inb(DATA_PORT);
	       if(!(response == 0xAA || response == 0xFA)) {kpanic("Device on PS/2 second port failed self-test!");}
	}
	
	initDevices();
	
	return;
}

void initDevices() {
	//Query device type on port 1 (disable scanning, send identify command)
	port1Command(0xF5, 0);
	port1Command(0xF2, 0);
	pollToRead();
	uint8_t type[2];
	type[0] = inb(DATA_PORT);
	pollToRead();
	type[1] = inb(DATA_PORT);
	//Check if it is a mouse, else its a keyboard
	if (!(type[0]) || type[0] == 3 || type[0] == 4) {addInterrupt(33, INTERRUPT_ATTRIBUTES, mouseMasterHandler);} 
	else {
		addInterrupt(33, INTERRUPT_ATTRIBUTES, keyboardMasterHandler);
		keyboard1Init();
	}
	//Enable scanning
	port1Command(0xF4, 0);

	//Query device type on port 2
	port2Command(0xF5, 0);
	port2Command(0xF2, 0);
	pollToRead();
	type[0] = inb(DATA_PORT);
	pollToRead();
	type[1] = inb(DATA_PORT);
	//Check if it is a mouse, else its a keyboard
	if (!(type[0]) || type[0] == 3 || type[0] == 4) {addInterrupt(44, INTERRUPT_ATTRIBUTES, mouseSlaveHandler);} 
	else {
		addInterrupt(44, INTERRUPT_ATTRIBUTES, keyboardSlaveHandler);
		keyboard2Init();
	}
	//Enable scanning
	port2Command(0xF4, 0);

	return;
}

uint8_t port1Command(uint8_t command, uint8_t data) {
	pollToWrite();
	outb(DATA_PORT, command);
	if(data) {
		pollToWrite();
		outb(DATA_PORT, data);
	}
	pollToRead();
	uint8_t response = inb(DATA_PORT);
	if (response == 0xFE) {response = port1Command(command, data);}
	return response;
}

uint8_t port2Command(uint8_t command, uint8_t data) {
	outb(CONTROL_PORT, 0xD4);
	pollToWrite();
	outb(DATA_PORT, command);
	if(data) {
		pollToWrite();
		outb(DATA_PORT, data);
	}
	pollToRead();
	uint8_t response = inb(DATA_PORT);
	if (response == 0xFE) {response = port2Command(command, data);}
	return response;
}

//Waits until DATA_PORT is ready to be read
void pollToRead() {
	uint8_t status = 0;
	uint32_t timeout = 1000000;
	while ((!status) && timeout) {
		status = inb(CONTROL_PORT) & 1;
		timeout--;
	}
	return;
}

//Waits until both ports are ready to be written
void pollToWrite() {
	uint8_t status = 1;
	uint32_t timeout = 1000000;
	while (status && timeout) {
		status = inb(CONTROL_PORT) & 2;
		timeout--;
	}
	return;
}

__attribute__ ((interrupt)) 
void keyboardMasterHandler(struct interruptFrame *frame) {
	keyboardIrq();
	returnMasterIrq();
	return;
}

__attribute__ ((interrupt)) 
void keyboardSlaveHandler(struct interruptFrame *frame) {
	keyboardIrq();
	returnSlaveIrq();
	return;
}

__attribute__ ((interrupt))
void mouseMasterHandler(struct interruptFrame *frame) {
	inb(DATA_PORT);
	returnMasterIrq();
	return;
}

__attribute__ ((interrupt)) 
void mouseSlaveHandler(struct interruptFrame *frame) {
	inb(DATA_PORT);
	returnSlaveIrq();
	return;
}

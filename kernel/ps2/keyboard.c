//Keys that are currently down (removed from buffer if the key is lifted)
static uint8_t pressedKeys[32];
//Keys that have been pressed since last check (stay in buffer if the key is lifted)
static uint8_t newKeys[32];
//Scan code decoding tables for UK QWERTY keyboard using scan code set 2
static const uint8_t symbolTable[] = {		'`', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
						'\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
						'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '#',
						'\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',
						' '};
static const uint8_t symbolShiftTable[] = {	(uint8_t)'¬', '!', '\"', (uint8_t)'£', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
						'\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
						'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '@', '~',
						'|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '\?',
						' '};
static const uint8_t scancodeTable[] = {	0x0E, 0x16, 0x1E, 0x26, 0x25, 0x2E, 0x36, 0x3D, 0x3E, 0x46, 0x45, 0x4E, 0x55, 0x66,
						0x0D, 0x15, 0x1D, 0x24, 0x2D, 0x2C, 0x35, 0x3C, 0x43, 0x44, 0x4D, 0x54, 0x5B, 0x5A,
						0x1C, 0x1B, 0x23, 0x2B, 0x34, 0x33, 0x3B, 0x42, 0x4B, 0x4C, 0x52, 0x5D,
						0x61, 0x1A, 0x22, 0x21, 0x2A, 0x32, 0x31, 0x3A, 0x41, 0x49, 0x4A,
						0x29};
static uint8_t shift = 0;
static uint8_t ctrl = 0;
static uint8_t alt = 0;
static uint8_t shiftLock = 0;
static uint8_t pressed = 1;

void keyboardIrq() {
	uint8_t inByte = inb(DATA_PORT);
	uint8_t symbol;
	uint8_t index = 0xFF;

	switch (inByte) {
		case 0xF0:
			//Key released
			pressed = 0;
			return;
			
		case 0x71:
			//Delete Key (only used for reboot atm)
			if (pressed && ctrl && alt) {reboot();}
			return;

		case 0x14:
			//Ctrl key (only used for reboot atm)
			if (!pressed) {ctrl = 0; pressed = 1;} else {ctrl = 1;}
			return;

		case 0x11:
			//Alt key (only used for reboot atm)
			if (!pressed) {alt = 0; pressed = 1;} else {alt = 1;}
			return;

		case 0x12:
		case 0x59:
			//Left shift or right shift pressed/released
			if (!pressed) {shift = 0; pressed = 1;} else {shift = 1;}
			return;

		case 0x58:
			//Caps lock (we call shift lock) pressed/released
			if (pressed) {if (shiftLock) {shiftLock = 0;} else {shiftLock = 1;}}
			else {pressed = 1;}
			return;

		default:
			//Regular key pressed/released
			for (uint8_t i=0; i<sizeof(scancodeTable); i++) {
				if (inByte == scancodeTable[i]) {
					index = i;
				}
			}
	}

	//Check if it is a recognised key
	if (index == 0xFF) {return;}

	//Check key modifiers (shift, altgr)
	if (shift ^ shiftLock) {
		symbol = symbolShiftTable[index];
		//Remove the regular version of this symbol from the array in case shift was pressed after the key was pressed
		for (uint8_t i=0; i<32; i++) {
			if (symbolTable[index] == pressedKeys[i]) {pressedKeys[i] = 0; break;}
		}
	} 
	else {
		symbol = symbolTable[index];
		//Remove the shifted version of this symbol from the array in case shift was lifted after the key was pressed
		for (uint8_t i=0; i<32; i++) {
			if (symbolShiftTable[index] == pressedKeys[i]) {pressedKeys[i] = 0; break;}
		}
	}

	//Proccess the key
	if (pressed) {
		//Check if in buffers, else add to buffers
		uint8_t inPressedKeys = 0;
		uint8_t inNewKeys = 0;
		for (uint8_t i=0; i<32; i++) {
			if (pressedKeys[i] == symbol) {inPressedKeys = 1;}
			if (newKeys[i] == symbol) {inNewKeys = 1;}
			if (inPressedKeys && inNewKeys) {break;}
		}
		for (uint8_t i=0; i<32; i++) {
			if ((!inPressedKeys) && (!pressedKeys[i])) {pressedKeys[i] = symbol; inPressedKeys = 1;}
			if ((!inNewKeys) && (!newKeys[i])) {newKeys[i] = symbol; inNewKeys = 1;}
			if (inPressedKeys && inNewKeys) {break;}
		}
	} else {
		pressed = 1;
		//Remove from buffer (shifted version and regular, incase shift was lifted at the same time as key)
		for (uint8_t i=0; i<32; i++) {
			if (pressedKeys[i] == symbolTable[index] || pressedKeys[i] == symbolShiftTable[index]) {pressedKeys[i] = 0;}
		}
	}

	return;
}

void getNewKeys(uint8_t *buf) {
	for (uint8_t i=0; i<sizeof(newKeys); i++) {
		buf[i] = newKeys[i];
		newKeys[i] = 0;
	}
	return;
}

void getPressedKeys(uint8_t *buf) {
	for (uint8_t i=0; i<sizeof(pressedKeys); i++) {
		buf[i] = pressedKeys[i];
	}
	return;
}

void keyboard1Init() {
	//Set scan code set 2
	port1Command(0xF0, 2);
	//Set typematic byte (30hz repeat rate, 0.75 seconds before repeat)
	port1Command(0xF3, 0b01000000);
	return;
}

void keyboard2Init() {
	//Set scan code set 2
	port2Command(0xF0, 2);
	//Set typematic byte 
	port2Command(0xF3, 0b01000000);
	return;
}

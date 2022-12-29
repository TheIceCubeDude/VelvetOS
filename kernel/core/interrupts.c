struct idtEntry {
	uint16_t offsetLow;
	uint16_t segmentSelector;
	uint8_t reserved;
	uint8_t attributes;
	uint16_t offsetHigh;
} __attribute__ ((packed));

struct idtDescriptor {
	uint16_t size;
	void *offset;
} __attribute__ ((packed));

static struct idtEntry *idt;
static struct idtDescriptor idtr;

void idtInit() {
	idt = malloc(256*8);
	idtr.size = (256*8) - 1;
	idtr.offset = idt;
	loadIdt(&idtr);

	reprogramPic();

	for (uint8_t i=0; i<32; i++) {
		switch (i) {
			case 8:
				addInterrupt(i, TRAP_ATTRIBUTES, (void*) doubleFaultHandler);
				break;
			default:
				addInterrupt(i, TRAP_ATTRIBUTES, (void*) generalExceptionHandler);
		}
	}
	for (uint8_t i=32; i<40; i++) {
		switch (i) {
			case 39:
				addInterrupt(i, INTERRUPT_ATTRIBUTES, (void*) spuriousMasterIrqHandler);
				break;
			default:
				addInterrupt(i, INTERRUPT_ATTRIBUTES, (void*) unhandledMasterIrqHandler);
		}
	}
	for (uint8_t i=40; i<48; i++) {
		switch (i) {
			case 47:
				addInterrupt(i, INTERRUPT_ATTRIBUTES, (void*) spuriousSlaveIrqHandler);
				break;
			default:
				addInterrupt(i, INTERRUPT_ATTRIBUTES, (void*) unhandledSlaveIrqHandler);
		}
	}
	for (uint8_t i=48; i<255; i++) {
		addInterrupt(i, INTERRUPT_ATTRIBUTES, (void*) unhandledInterruptHandler);
	}
	return;
}

void addInterrupt(uint8_t vector, uint8_t attributes, void *address) {
	struct idtEntry *vectorEntry = idt + vector;
	vectorEntry->offsetLow = (uint16_t)((uint32_t) address);
	vectorEntry->offsetHigh = (uint16_t)((uint32_t) address >> 16);
	vectorEntry->segmentSelector = 8;
	vectorEntry->reserved = 0;
	vectorEntry->attributes = attributes;
	return;
}

void reprogramPic() {
	//The PIC's has been programmed by the BIOS for real mode
	//The BIOS sets the IRQ's incorrectly: IRQ'S 0-7 conflict with Intel's reserved exceptions
	//We will fix this by reprogramming the PIC with the correct IRQ offsets
	//See http://www.brokenthorn.com/Resources/OSDevPic.html

	//Save masks by reading data registers without sending a command
	//uint8_t mask1 = inb(PIC1_DATA);
	//uint8_t mask2 = inb(PIC2_DATA);

	//Send initalisation code (ICW 1) 0b00010001
	outb(PIC1_COMMAND, 0x11);
	io_wait();
	outb(PIC2_COMMAND, 0x11);
	io_wait();

	//Send the new IRQ offsets (ICW 2) 0x20 & 0x28
	outb(PIC1_DATA, 0x20);
	io_wait();
	outb(PIC2_DATA, 0x28);
	io_wait();

	//Tell the PIC'S which IRQ to use to communicate with each other (ICW 3) 2
	//PIC1 uses each bit for each IRQ line, PIC2 uses first 3 bits to represent the IRQ lines in binary notation
	outb(PIC1_DATA, 4);
	io_wait();
	outb(PIC2_DATA, 2);
	io_wait();

	//Send the operation mode (ICW 4) 0b00000001
	outb(PIC1_DATA, 1);
	io_wait();
	outb(PIC2_DATA, 1);
	io_wait();

	//Initalisaton complete, now finally unmask PIT (0), both PS/2 (1&12), CMOS (8) and cascade (2)
	outb(PIC1_DATA, 0b11111000);
	outb(PIC2_DATA, 0b11101110);
	
	return;
}

__attribute__ ((interrupt))
void generalExceptionHandler(struct interruptFrame *frame) {
	printf("A general exception has occured!");
	return;
}

__attribute__ ((interrupt))
void unhandledMasterIrqHandler(struct interruptFrame *frame) {
	printf("An unhandled hardware IRQ had been called!");
	returnMasterIrq();
	return;
}

__attribute__ ((interrupt))
void unhandledSlaveIrqHandler(struct interruptFrame *frame) {
	printf("An unhandled hardware IRQ had been called!");
	returnSlaveIrq();
	return;
}

__attribute__ ((interrupt))
void unhandledInterruptHandler(struct interruptFrame *frame) {
	printf("An unhandled interrupt has been called!");
	return;
}

__attribute__ ((interrupt))
void spuriousMasterIrqHandler(struct interruptFrame *frame) {
	//Don't send End Of Interrupt for spurious interrupts
	return;
}

__attribute__ ((interrupt))
void spuriousSlaveIrqHandler(struct interruptFrame *frame) {
	//Master PIC doesn't know the interrupt was spurious, so we need to send End Of Interrupt
	returnMasterIrq();
	return;
}

__attribute__ ((interrupt))
void doubleFaultHandler(struct interruptFrame *frame) {
	kpanic("CPU has double faulted!");
	return;
}

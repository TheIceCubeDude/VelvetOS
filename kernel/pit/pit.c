#include "../core/include.h"
#include "include.h"

#define CHANNEL_0 0x40
#define CHANNEL_1 0x41
#define CHANNEL_2 0x42
#define COMMAND_REGISTER 0x43

static const uint32_t FREQUENCY_DIVISOR = 1193;
static uint32_t ms;

void initPit() {
	//Setup PIT channel 0 (which triggers IRQ0) to fire every 1ms
	//Firstly setup channel 0's mode
	outb(COMMAND_REGISTER, 0b00110100);
	
	//Set channel 0's counter so that it reaches 0 every 1ms
	//Send low 8 bits of counter then high 8 bits
	outb(CHANNEL_0, (uint8_t)FREQUENCY_DIVISOR);
	outb(CHANNEL_0, (uint8_t)(FREQUENCY_DIVISOR >> 8));

	//Setup the IRQ
	addInterrupt(32, INTERRUPT_ATTRIBUTES, timerIrq);

	resetTimer();
	return;
}

void resetTimer() {
	ms = 0;
	return;
}

uint32_t getTime() {
	return ms;
}

__attribute__ ((interrupt))
void timerIrq(struct interruptFrame *frame) {
	ms++;
	returnMasterIrq();
	return;
}

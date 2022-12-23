static const uint8_t SPEAKER = 0x61;
static uint8_t speakerVal;

void playFreq(uint16_t hz) {
	uint16_t freq = 1193180 / hz;
	outb(CHANNEL_2, (uint8_t)freq);
	outb(CHANNEL_2, (uint8_t)(freq >> 8));
	return;
}

void enableSpeaker() {
	speakerVal = inb(0x61);
	outb(SPEAKER, speakerVal | 3);
	return;
}

void disableSpeaker() {
	outb(0x61, speakerVal);
	return;
}

void playSound(uint16_t hz, uint32_t ms) {
	playFreq(hz);
	enableSpeaker();
	sleep(ms);
	disableSpeaker();
	return;
}

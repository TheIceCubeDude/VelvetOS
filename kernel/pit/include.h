//Timer
void initPit();
void resetTicks();
uint32_t getTicks();
void sleep(uint32_t ms);

__attribute__ ((interrupt))
void timerIrq(struct interruptFrame *frame);

//Speaker
void playFreq(uint16_t hz);
void enableSpeaker();
void disableSpeaker();
void playSound(uint16_t hz, uint32_t ms);

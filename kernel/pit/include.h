void initPit();
void resetTimer();
uint32_t getTime();
void timerIrq(struct interruptFrame *frame);

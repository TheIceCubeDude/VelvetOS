void initPs2();
uint8_t port1Command(uint8_t command, uint8_t data);
uint8_t port2Command(uint8_t command, uint8_t data);
void pollToRead();
void pollToWrite();
void initDevices();

__attribute__ ((interrupt))
void keyboardMasterHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void keyboardSlaveHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void mouseMasterHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void mouseSlaveHandler(struct interruptFrame *frame);

void keyboardIrq();
void keyboard1Init();
void keyboard2Init();
void getNewKeys(uint8_t *buf);
void getPressedKeys(uint8_t *buf);

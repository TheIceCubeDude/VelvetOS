#include "stdint.h"

//Kernel
void halt();
void kpanic(uint8_t *cause);

//Memory
uint8_t enableSSE();
void memcpy(void *destination, void *source, uint32_t size);
uint8_t memset(void *destination, uint32_t x, uint32_t size);

//Heap
void setHeap(void *address);
void makeHeap(uint32_t size);
void* malloc(uint32_t size);
void free(void* objectAddress);

//IO
void io_wait();
void outb(uint16_t port, uint8_t data);
void outw(uint16_t port, uint16_t data);
void outl(uint16_t port, uint32_t data);
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);
void insw(uint16_t port, void *buf, uint32_t size);

//Scheduling
struct process {
	void *code;
	uint32_t codeSize;
	uint32_t stackSize;
	uint8_t padding;
	void *stackPtr;
	uint8_t pltResolved;
	struct process *nextProc;
};
struct process* addProcess(uint32_t codeSize, uint32_t stackSize);
void _resolvePlt(struct process *process);
void destroyProcess(struct process *proc);
void exitKernel();
void yield();
void* getNextProcess(void *stackPtr);
void exitKernelAsm(void *stackPtr);

//Interrupts
#define TRAP_ATTRIBUTES 0b10001111 
#define INTERRUPT_ATTRIBUTES 0b10001110
#define PIC1_COMMAND 0x20
#define PIC2_COMMAND 0xA0
#define PIC1_DATA 0x21
#define PIC2_DATA 0xA1

struct interruptFrame {
	uint32_t eip;
	uint32_t cs;
	uint32_t flags;
	uint32_t esp;
	uint32_t ss;
};
void idtInit();
void reboot();
void addInterrupt(uint8_t vector, uint8_t attributes, void *address);
void reprogramPic();
void loadIdt(void *idtr);
void enableIrqs();
void disableIrqs();

static inline void returnMasterIrq() {
	outb(PIC1_COMMAND, 0x20);
	return;
}
static inline void returnSlaveIrq() {
	outb(PIC2_COMMAND, 0x20);
	outb(PIC1_COMMAND, 0x20);
	return;
}
__attribute__ ((interrupt))
void unhandledInterruptHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void generalExceptionHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void unhandledMasterIrqHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void unhandledSlaveIrqHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void spuriousMasterIrqHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void spuriousSlaveIrqHandler(struct interruptFrame *frame);
__attribute__ ((interrupt))
void doubleFaultHandler(struct interruptFrame *frame);

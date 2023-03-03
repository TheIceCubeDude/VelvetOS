static struct process *firstProc = 0;
static struct process *lastProc = 0;
static struct process *currentProc = 0;

struct process* addProcess(uint32_t codeSize, uint32_t stackSize) {
	//Make sure it is a multiple of 4
	uint8_t padding = 0;
	if (codeSize % 4) {padding = 4 - (codeSize % 4); codeSize += 4 - (codeSize % 4);}
	if (stackSize % 4) {stackSize += 4 - (stackSize % 4);}
	//Allocate it on code heap
	setHeap(mmap->code);
	void *code = malloc(codeSize + stackSize + padding); 	
	if (!code) {return 0;}
	setHeap(mmap->heap);
	//Set push return value on programs stack to the beginning of the program, and also push default regs
	uint32_t *progStack = (uint32_t*)(((uint32_t)code) + codeSize + padding + stackSize - 4);
	*progStack = (uint32_t)code;
	for(uint8_t i=0; i<8; i++) {
		progStack--;
		if (i==5) {*progStack = (uint32_t)progStack;} //Set ESP to stack
		else {*progStack = 0xdeadbeef;}
	}
	//Create and return a pointer to the process info
	struct process *proc = malloc(sizeof(struct process));
	*proc = (struct process){code, codeSize, stackSize, padding, progStack,  0, 0};
	if (lastProc) {lastProc->nextProc = proc;}
	lastProc = proc;
	if (!firstProc) {firstProc = proc;}
	return proc;
}

void _resolvePlt(struct process *process) {
	uint32_t funcs[] = {
		(((uint32_t) playSound) + ((uint32_t) mmap->kernel)),
	       	(((uint32_t) printDec) + ((uint32_t) mmap->kernel)),
	       	(((uint32_t) yield) + ((uint32_t) mmap->kernel))
	//NOTE: functions in this compilation unit may be resolved to base 1048676. If so, just subtract from 1048576
	};
	uint32_t *pltSig = (uint32_t*) ((uint32_t)process->code + process->codeSize - process->padding - 4);
	uint32_t pltSize = *pltSig + 4;
	uint32_t *plt = process->code + process->codeSize - process->padding - pltSize; 
	uint32_t funcIndex = 0;
	while (plt != pltSig) {
		printHex(funcs[funcIndex]);
		*plt = funcs[funcIndex]; 
		plt++;
		funcIndex++;
	}
	printf("Resolved PLT!");
	return;
}

void destroyProcess(struct process *proc) {
	//TODO: make the list re link or whatever
	setHeap(mmap->code);
	free(proc->code);
	setHeap(mmap->heap);
	free(proc);
	return;
}

void* getNextProcess(void *stackPtr) {
	currentProc->stackPtr = stackPtr;
	currentProc = currentProc->nextProc;
	if (!currentProc) {currentProc = firstProc;}
	if (!currentProc->pltResolved) {_resolvePlt(currentProc); currentProc->pltResolved = 1;}
	return currentProc->stackPtr;
}

void exitKernel() {
	currentProc = firstProc;
	if (!currentProc->pltResolved) {_resolvePlt(currentProc); currentProc->pltResolved = 1;}
	exitKernelAsm(currentProc->stackPtr);
}

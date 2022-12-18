static const uint32_t HEADER_SIGNATURE = 0xABCDEF88;
static const uint32_t HEAP_SIGNATURE = 0x123456DD;
static uint8_t *heapAddress;

typedef struct blockHeader blockHeader;
typedef struct heapMetadata heapMetadata;

struct blockHeader{
	uint32_t blockSig;
	blockHeader *nextBlock;
	blockHeader *previousBlock;
	void *blockEnd; 

} __attribute__ ((packed));
struct heapMetadata {
	uint32_t heapSig;
	uint32_t size;
	uint32_t padding[2];
} __attribute__ ((packed));

//World's worst heap implementation
//Optimised for the ease of programming it
//Slow, becomes very fragmented very quickly

void setHeap(void *address) {
	heapAddress = (uint8_t*)address;
	return;
}

void makeHeap(uint32_t size) {
	//Set metadata
	heapMetadata *hm = (heapMetadata*) heapAddress;
	hm->heapSig = HEAP_SIGNATURE;
	hm->size = size;

	//Create last block
	blockHeader *endingBlock = (blockHeader*) (heapAddress + size - sizeof(blockHeader));
	endingBlock->blockSig = HEADER_SIGNATURE;
	endingBlock->nextBlock = 0;
	endingBlock->blockEnd = heapAddress + size;

	//Create first block
	blockHeader *starterBlock = (blockHeader*) (heapAddress + sizeof(heapMetadata));
	endingBlock->previousBlock = starterBlock;
	starterBlock->blockSig = HEADER_SIGNATURE;
	starterBlock->nextBlock = endingBlock;
	starterBlock->previousBlock = 0;
	starterBlock->blockEnd = (void*) (((uint32_t) starterBlock) + sizeof(blockHeader));

	return;
}

void* malloc(uint32_t size) {
	//We want to keep heap alignments
	if (size % 4) {return 0;}
	size += sizeof(blockHeader);
	//Firstly check heap signature
	if (!(((heapMetadata*) heapAddress)->heapSig == HEAP_SIGNATURE)) {kpanic("Heap damaged!");}

	//Loop through blocks, keeping track of the smallest free memory that is bigger than or equal to size
	uint32_t freeSize = 0xFFFFFFFF;
	uint32_t *freeMem = 0;
	blockHeader *blockBeforeFreeMem;

	blockHeader *currentBlock = (blockHeader*) (heapAddress + sizeof(heapMetadata));
	while (1) {
		//Check if we are at endingBlock
		if (currentBlock->nextBlock == 0) {break;}
		//Check if current block is damaged
		if (!(currentBlock->blockSig == HEADER_SIGNATURE)) {kpanic("Heap overflow detected!");}

		//Calculate free space and check if it is best fit
		uint32_t currentFreeSpace = (uint32_t) currentBlock->nextBlock - (uint32_t) currentBlock->blockEnd;
		if ((currentFreeSpace < freeSize) && (currentFreeSpace >= size)) {
			blockBeforeFreeMem = currentBlock;
			freeSize = currentFreeSpace;
			freeMem = (uint32_t*)((uint32_t)currentBlock->blockEnd); //Do we need to +1?
		}

		//Prepare for next iteration
		currentBlock = currentBlock->nextBlock;
	}
	
	//Check if we found a block, and create a header on it
	if (!freeMem) {return 0;}
	blockHeader *newBlock = (blockHeader*) freeMem;
	newBlock->blockSig = HEADER_SIGNATURE;
	newBlock->nextBlock = blockBeforeFreeMem->nextBlock;
	newBlock->previousBlock = blockBeforeFreeMem;
	newBlock->blockEnd = (void*) ((uint32_t)newBlock + size);

	//Update the data of the previous and next block
	newBlock->nextBlock->previousBlock = newBlock;
	blockBeforeFreeMem->nextBlock = newBlock;

	return (void*) (((uint32_t) newBlock) + sizeof(blockHeader));
}

void free(void* objectAddress) {
	//Check objectAddress is valid
	blockHeader *block = (blockHeader*)((uint32_t) objectAddress - sizeof(blockHeader));
	if (!(block->blockSig == HEADER_SIGNATURE)) {kpanic("Object attempted to be free'd does not have correct signature!");}

	//Update the data of the previous and next block to ignore this block
	block->nextBlock->previousBlock = block->previousBlock;
	block->previousBlock->nextBlock = block->nextBlock;
	return;
}

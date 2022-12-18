#include "stdint.h"

void halt();
void kpanic(uint8_t *cause);

uint8_t enableSSE();
void memcpy(void *destination, void *source, uint32_t size);
uint8_t memset(void *destination, uint32_t x, uint32_t size);

void setHeap(void *address);
void makeHeap(uint32_t size);
void* malloc(uint32_t size);
void free(void* objectAddress);

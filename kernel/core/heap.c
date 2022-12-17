static void *heapAddress;

void setHeap(void *address) {
	heapAddress = address;
	return;
}

void* malloc(uint32_t size) {
	return heapAddress;
}

void free(void* objectAddress) {
	return;
}

static char *heapAddress;

void setHeap(char *address) {
	heapAddress = address;
	return;
}

char* malloc(unsigned long size) {
	return heapAddress;
}

void free(char* objectAddress) {
	return;
}

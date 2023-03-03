#include "../kernel/core/stdint.h"

//To create an executable binary (.ebin), link against libsyscall.a, and use script.ld
//Then objcopy to binary, including sections .text .data .rodata .bss .plt IN THAT ORDER
//This produces the following format:

//CODE
//DATA
//READ ONLY DATA
//UNINITALISED DATA
//SYSCALL FUNCTION LOOK UP TABLE

//Define syscall function ptr's
extern void (*yield)();
extern void (*printf)(uint8_t*);
extern void (*fillScreen)(uint32_t);

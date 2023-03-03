;; Puts a 'PLT' at the .plt section of the ELF
SECTION .plt

global _start
global yield 
global printDec
global halt

start:

playSound: dd 0
printDec: dd 0
yield: dd 0

end:

;;PLT size
dd end - start

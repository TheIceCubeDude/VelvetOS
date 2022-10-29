attrib_bootable equ 10000000b
attrib		equ 00000000b
kernelID	equ 0x8A

;; Remember: cylinder is 10 bits and sector is 6!

partition_1:
	.attributes:		db attrib_bootable
	.startingHead:		db 0
	.startingSector:	db 2
	.startingCylinder: 	db 0
	.systemID:		db kernelID
	.endingHead:		db 0
	.endingSector:		db 8
	.endingCylinder: 	db 0
	.lba:			dd 2
	.size:			dd 7

partition_2: 
	.attributes: 		db attrib
	.startingHead:		db 0
	.startingSector:	db 8
	.startingCylinder:	db 0
	.systemID:		db kernelID
	.endingHead:		db 0
	.endingSector:		db 42
	.endingCylinder		db 32
	.lba:			dd 8
	.size:			dd 2048

partition_3: times 16 db 0
partition_4: times 16 db 0

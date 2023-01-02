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
	.endingSector:		db 9
	.endingCylinder: 	db 0
	.lba:			dd 2
	.size:			dd 8

partition_2: 
	.attributes: 		db attrib
	.startingHead:		db 0
	.startingSector:	db 9
	.startingCylinder:	db 0
	.systemID:		db kernelID
	.endingHead:		db 0
	.endingSector:		db 43
	.endingCylinder		db 32
	.lba:			dd 9
	.size:			dd 2048

partition_3:
	.attributes: 		db attrib
	.startingHead:		db 0
	.startingSector:	db 0
	.startingCylinder:	db 0
	.systemID:		db kernelID
	.endingHead:		db 0
	.endingSector:		db 0
	.endingCylinder		db 0
	.lba:			dd 2057
	.size:			dd 2048

partition_4: times 16 db 0

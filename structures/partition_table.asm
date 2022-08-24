partition_1:
	.attributes:		db 10000000b
	.startingHead:		db 0
	.startingSector:	db 2	;Bits 6-7 are s-cylinder
	.startingCylinder: 	db 0
	.systemID:		db 0x8A	;Linux kernel partition
	.endingHead:		db 0
	.endingSector:		db 6	;Bits 6-7 are e-cylinder
	.endingCylinder: 	db 0
	.LBA:			db 2, 0, 0, 0 ;Little endian
	.size:			db 4, 0, 0, 0 ;Little endian

partition_2: times 16 db 0
partition_3: times 16 db 0
partition_4: times 16 db 0

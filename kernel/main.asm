[BITS 32]

main:
	hlt
	db times 1048576-($-$$) db 0

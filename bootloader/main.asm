[BITS 16]
[ORG 0x7c00]

main:	
	cli
	
	mov ax, 0
	mov ds, ax
	mov ss, ax
	
	;; Setup temporary stack (grows down)
	mov esp, 0x7BFF
	mov ebp, 0

	;; Do bootloader-y stuff
	push dx
	push dx
	call enterVGAMode
	pop dx
	call loadBootloader
	
	call A20_enable
	call mMap_prepareMemory
	call mMap_enterUnrealMode
	pop dx
	call mMap_loadKernel
	call mMap_relocateCoreinfo

	;; Once we enter VBE, BIOS printing will not work
	;; pModeMsg is located in GDT structure file
	mov dx, pModeMsg
	mov bl, 00001111b
	call textPrint
	call VBE_enterGraphicsMode
	jmp enterPMode

loadBootloader:
	mov ax, 0
	mov es, ax
	mov bx, 0x7E00
	
	;; DL is set by BIOS (drive no.)
	;; Sector 1 is MBR, sector 2+ is bootloader (sector 0 isn't a thing)
	mov ah, 2
	mov al, [partition_1.size]
	mov ch, 0
	mov cl, [partition_1.lba]
	mov dh, 0
	int 0x13

	jc .err
	mov bl, 00000010b
	mov dx, .msg1
	call textPrint
	ret

	.err:
	mov dx, .msg0
	call textErr

	.msg0: db "Couldn't load bootloader!", 0
	.msg1: db "Bootloader succesfully loaded!", 0

enterVGAMode:
	mov dx, .msg0
	call textPrint

	mov ah, 0
	mov al, 0x12
	int 0x10

	;; Check if we are in desired mode
	mov ah, 0xF
	int 0x10
	cmp al, 0x12
	jne .fail

	mov dx, .msg1
	mov bl, 00000010b
	call textPrint
	ret

	.fail:
	mov dx, .msg3
	mov bl, 00001110b
	call textPrint
	ret

	.msg0: db "Entering VGA mode 12h", 0
	.msg1: db "Entered VGA mode 12h!", 0
	.msg3: db "Couldn't enter VGA mode 12h. Will proceed with boot anyways.", 0

enterPMode:
	cli
	;; ECX contains location of mmap
	mov ecx, [mmap.coreinfo]
	;; EBX contains location of fbInfo
	mov ebx, ecx
	add ebx, fbInfo - coreinfo
	;; Load GDT and toggle protected mode bit
	lgdt[gdtr]
	mov eax, cr0
	or al, 1
	mov cr0, eax
	;; Jump to kernel
	mov eax, dword [mmap.kernel]
	jmp 8:jumpToKernel

textErr:
	;; DX  - String ptr
	push dx
	mov dx, .err
	mov bl, 11101100b
	call textPrint

	pop dx
	mov bl, 00001111b
	call textPrint
	jmp $

	.err: db "Error: ", 0

textPrint:
	;; DX - String ptr
	;; BL - Colour
	mov ax, 0
	mov es, ax

	mov cx, 0
	mov bh, 0
	mov ah, 0x0E

	call .iterate

	;; Increment line
	push dx
	push bx
	mov ah, 3
	mov bh, 0
	mov dl, 0
	int 0x10

	inc dh
	mov ah, 2
	mov dl, 0
	int 0x10
	pop bx
	pop dx
	ret

	.iterate:
	push cx
	add cx, dx
	push bx
	mov bx, cx
	mov al, [es:bx]
	pop bx
	cmp al, 0
	je .ret
	mov cx, 1
	int 0x10
	pop cx
	inc cx
	jmp .iterate
	
	.ret:
	pop cx
	ret

MBRData:
	times 446-($-$$) db 0
	%include "structures/partition_table.asm"
	
	.sig:
	times 510-($-$$) db 0
	dw 0xAA55

bootUtils:
	%include "structures/gdt.asm"
	%include "structures/coreinfo.asm"
	%include "bootloader/VBE.asm"
	%include "bootloader/memory_map.asm"
	%include "bootloader/A20.asm"

[BITS 32]
jumpToKernel: jmp eax

times 4608-($-$$) db 0
align 4
tmp:

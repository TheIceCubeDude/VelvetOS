%define desiredBpp 32
VBE_enterGraphicsMode:
	call getAvaliableModes
	call findBestMode
	call selectMode
	call setFramebufferMmap
	ret

getAvaliableModes:	
	;; Query VBE for avaliable modes
	mov ax, 0x4F00

	;; Load info for VBE 2.0+ systems
	mov bx, 0
	mov es, bx
	mov bx, VBEInfoStruct
	mov di, bx
	int 0x10

	;; Check if VBE is supported and call worked
	cmp al, 0x4F
	jne .notSupported
	cmp ah, 0
	jne .err
	mov dx, .msg2
	mov bl, 00000010b
	call textPrint
	
	ret

	.notSupported:
	mov dx, .msg0
	jmp textErr

	.err:
	mov dx, .msg1
	jmp textErr

	.msg0: db "VESA VBE is not supported on this system!", 0
	.msg1: db "VESA VBE video mode query returned error!", 0
	.msg2: db "Got VBE supported modes!", 0
	
findBestMode:
	mov eax, dword [VBEInfoStruct.VBESig]
	cmp eax, "VESA"
	jne .sigErr

	;; Remember: everything here is little-endian
	;; Offset of video modes list
	mov bx, word [VBEInfoStruct.videoModes]
	;; Segment of video modes list
	mov ax, word [VBEInfoStruct.videoModes + 2]
	mov es, ax
	mov ax, 0
	mov edx, 0
	jmp .loopThroughModes

	.loopThroughModes:
	;; BX is mem loc
	;; AX is highest resolution mode no.
	;; EDX is highest resolution mode resolution
	;; CX is current mode
	
	mov cl, [es:bx]
	mov ch, [es:bx + 1]

	add bx, 2
	cmp cx, 0xFFFF
	jne .analyzeMode
	cmp edx, 0
	je .noModes

	push ax
	push edx

	mov cx, ax
	call .loadModeData
	cmp ax, 0x004F
	jne .getChosenModeInfoErr
	
	mov edx, .msg3
	mov bl, 00000010b
	call textPrint
	
	pop edx
	pop ax
	ret

	.analyzeMode:
	push bx
	push es
	push ax

	;; Load data structure
	call .loadModeData

	cmp ax, 0x004F
	jne .modeInfoErr

	pop ax
	call .checkMode
	
	pop es
	pop bx
	jmp .loopThroughModes

	.checkMode:
	;; Check if linear framebuffer is supported
	mov dx, word[VBEModeInfoStruct.attributes]
	bt dx, 7
	jnc .ret
	
	;; Check bpp is the desired one
	mov dx, 0
	mov dl, byte[VBEModeInfoStruct.bpp]
	cmp dl, desiredBpp
	jne .ret
	
	;; Calculate resolution -> width * height [maybe in the future factor in bpp? But for now that results in an enourmous number so we won't do that]
	push eax
	push edx
	mov eax, 0
	mov edx, 0
	
	mov ax, word[VBEModeInfoStruct.width]
	mul word[VBEModeInfoStruct.height]
 ;	mul byte[VBEModeInfoStruct.bpp]
	
	pop edx
	cmp eax, edx
	;; Good chance that a  better bpp is later in the list?? So use greater than OR equal to.
	jge .select
	
	pop eax
	ret

	.select:
	mov edx, eax
	pop eax
	mov ax, cx
	ret

	.ret: ret

	.loadModeData:
	mov ax, VBEModeInfoStruct
	mov di, ax
	mov ax, 0
	mov es, ax

	mov ax, 0x4F01
	;; CX contains mode no.
	int 0x10
	ret

	.noModes:
	mov dx, .msg1
	call textErr

	.sigErr:
	mov dx, .msg0
	call textErr

	.modeInfoErr:
	;; Converts number to text very hackily
	push edx
	mov dx, .msg2
	mov bl, 00001110b
	call textPrint
	pop edx
	pop ax
	pop es
	pop bx
	jmp .loopThroughModes

	.getChosenModeInfoErr:
	mov dx, .msg4
	call textErr

	.msg0: db "VBE info block does not contain VESA signature! Your device probably only supports VBE 1.0", 0
	.msg1: db "No VBE video modes found!", 0
	.msg2: db "Could not get info for a video mode, skipping...", 0
	.msg3: db "Succesfully chosen VBE mode!", 0
	.msg4: db "Could not load the chosen mode's info!", 0

selectMode:
	mov bx, ax
	mov ax, 0x4F02
	
	;; Set linear framebuffer bit and clear don't wipe bit
	or bh, 01000000b
	and bh, 011111111b
	int 0x10
	
	cmp ax, 0x004F
	jne .err
	
	ret

	.err:
	;; Drop into text mode
	mov ax, 0
	int 0x10
	
	mov dx, .msg0
	call textErr

	.msg0: db "Could not load VBE mode, returned error!", 0

setFramebufferMmap:
	;; Set base
	mov ecx, dword [VBEModeInfoStruct.framebuffer]
	mov eax, [mmap.coreinfo]

	mov dx, word [VBEModeInfoStruct.width]
	mov [fbInfo.framebufferWidth - coreinfo + eax], dx
	mov dx, word [VBEModeInfoStruct.height]
	mov [fbInfo.framebufferHeight - coreinfo + eax], dx
	mov dx, word [VBEModeInfoStruct.pitch]
	mov [fbInfo.framebufferBPL - coreinfo + eax], dx

	push eax
	mov [fbInfo.framebuffer - mmap + eax], ecx
	;; Calculate size of framebuffer and set limit
	mov edx, 0
	mov eax, 0
	mov ecx, 0
	mov ax, word [VBEModeInfoStruct.height]
	mov cx, word [VBEModeInfoStruct.pitch]
	mul cx
	;; Result is in DX:AX not EAX, so merge them into EDX
	shl edx, 16
	mov dx, ax
	pop eax
	mov [fbInfo.framebufferSize - coreinfo + eax], edx
	ret

VBEInfoStruct:
	.VBESig:	dd "VBE2"
	.version:	dw 0
	.oem:		dd 0
	.capabilities:	dd 0
	.videoModes:	dd 0
	.videoMemory:	dw 0
	.softwareRev:	dw 0
	.vendor:	dd 0
	.productName:	dd 0
	.productRev:	dd 0
	.reserved:	times 222 db 0
	.oemData:	times 256 db 0

VBEModeInfoStruct:
	.attributes:		dw 0
	.windowA:		db 0
	.windowB:		db 0
	.granularity:		dw 0
	.windowSize:		dw 0
	.segmentA:		dw 0
	.segmentB:		dw 0
	.winFuncPtr:		dd 0
	.pitch:			dw 0
	.width:			dw 0
	.height:		dw 0
	.wChar:			db 0
	.yChar:			db 0
	.planes:		db 0
	.bpp:			db 0
	.banks:			db 0
	.memoryModel:		db 0
	.bankSize:		db 0
	.imagePages:		db 0
	.reserved0:		db 0
	.redMask:		db 0
	.redPosition:		db 0
	.greenMask:		db 0
	.greenPosition:		db 0
	.blueMask:		db 0
	.bluePosition:		db 0
	.reservedMask:		db 0
	.reservedPosition:	db 0
	.directColourAttributes:db 0
	.framebuffer:		dd 0
	.offScreenMemOff:	dd 0
	.offScreenMemSize:	dw 0
	.reserved1:		times 206 db 0
	

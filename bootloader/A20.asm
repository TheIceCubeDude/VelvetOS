A20_enable:
	;; Check if BIOS has already enabled A20
	call .check
	cmp cx, bx
	jne .ret
	
	;; Else try enabling via BIOS
	mov dx, .msg1
	call .methodFail
	call .enableBIOS

	;; Check if A20 is enabled
	call .check
	cmp cx, bx
	jne .ret

	;; Else try enabling via PS/2 controller
	mov dx, .msg2
	call .methodFail
	call .enablePS2

	;; Check if A20 is enabled
	call .check
	cmp cx, bx
	jne .ret

	;; Give up
	mov dx, .msg3
	call textErr

	.ret:
	mov dx, .msg0
	mov bl, 00000010b
	call textPrint
	ret

	.methodFail:
	mov bl, 00001110b
	call textPrint
	ret

	.msg0: db "A20 line is enabled!", 0
	.msg1: db "BIOS has not already enabled A20. Attempting to enable via BIOS...", 0
	.msg2: db "Could not enable A20 via BIOS. Attempting to enable via PS/2 controller...", 0
	.msg3: db "Could not enable A20 via any method!", 0
	
	.check:
	;; Check if memory wraps around at 1 meg, looking for boot signature
	mov ax, 0xFFFF
	mov es, ax
	mov ax, 0x7E0E
	mov di, ax
	
	mov bx, word[es:di]

	mov ax, 0
	mov es, ax
	mov ax, 0x7DFE
	mov di, ax
	
	mov cx, word[es:di]
	ret

	.enableBIOS:
	;; Check the function is supported
	mov ax, 0x2403
	int 0x15
	jb .ret
	cmp ah, 0
	jne .ret

	;; Activate
	mov ax, 0x2401
	int 0x15
	ret

	.enablePS2:
	;; Disable first PS/2 port
	call .waitForInput
	mov al, 0xAD
	out 0x64, al

	;; Get controller output port
	call .waitForInput
	mov al, 0xD0
	out 0x64, al
	
	call .waitForOutput
	in al, 0x60
	push ax

	;; Write controller output port with A20 enabled
	call .waitForInput
	mov al, 0xD1
	out 0x64, al

	call .waitForInput
	pop ax
	or al, 00000010b
	out 0x60, al

	;; Re-enable the first PS/2 port
	call .waitForInput
	mov al, 0xAE
	out 0x64, al

	;; Wait for it to be processed
	call .waitForInput
	ret

	.waitForInput:
	in al, 0x64
	bt ax, 1
	jc .waitForInput
	ret

	.waitForOutput:
	in al, 0x64
	bt ax, 0
	jnc .waitForOutput
	ret


%include "structures/gdt.asm"
%include "structures/size_info.asm"	

mMap_prepareMemory:
	call getMemoryMap
	call prepGDT
	ret

mMap_loadKernel:
	;; Before anything, enter unreal mode to access >1MB of memory but still have BIOS
	push dx
	call .enterUnrealMode
	;; We will load 4KB of the kernel at a time to kernelHold
	;; After each 4KB framgment has been loaded, we copy it to kernelSeg
	;; Firstly, find out how many fragments we will have
	mov eax, kernelSize
	mov edx, 0
	mov ecx, 4096
	div ecx
	mov ebp, eax
	dec ebp
	;; Now begin the loop
	pop dx 
	mov ecx, 0
	call .loadFragmentsLoop

	mov dx, .kernelLoaded
	mov bx, 00000010b
	call textPrint
	ret

	.kernelLoaded: db "Kernel succesfully loaded from boot media!", 0

	.loadFragmentsLoop:
	;; Load fragment off disk to kernelHold
	push dx
	call .loadFragment
	;; Now copy the fragment in kernelHold to final destination in kernelSeg
	;; Firstly, get the base location
        mov ah, [kernelSeg.base_hi]
        mov al, [kernelSeg.base_mid]
        shl eax, 16
        mov ax, [kernelSeg.base_low]
	;; Now calculate Counter * 4KB to give us offset
	push eax
	mov ebx, 4096
	mov eax, ecx
	mov edx, 0
	mul ebx
	mov ebx, eax
	pop eax
	;; Add the offset to the base location to give us final location
	add eax, ebx
	;; Now copy the 4KB over
	push ecx
	mov ecx, 0
	call .copyFragment
	pop ecx
	pop dx
	inc ecx
	cmp ecx, ebp
	jne .loadFragmentsLoop
	ret

	.copyFragment:
	;; Will automatically use DS, which is in unreal mode and thus can access > 1MB
	mov bl, [kernelHold + ecx]
	mov [eax + ecx], bl
	inc ecx
	cmp ecx, 4096
	jne .copyFragment
	ret

	.loadFragment:
	push ecx
	push dx
	;; Calculate sectors
	;; Firstly get how many sectors we have (8 sectors per fragment)
	mov eax, ecx
	mov edx, 0
	mov ebx, 8
	mul ebx
	;; Then add our base offset
	mov ebx, [partition_2.lba]
	add eax, ebx
	;; Prepare some registers for the interrupt
	mov [dap.sectorCount], word 8
	mov [dap.lba], eax
	mov si, dap
	pop dx
	;; Call the interrupt
	mov ah, 0x42	;Extended read from disk command
	int 0x13
	pop ecx
	ret

	.enterUnrealMode:	
	;; Save segment
	push ds
	;; Load GDT and enter protected mode
	lgdt [.unrealGDTR]
	mov eax, cr0
	or al, 1
	mov cr0, eax
	jmp $+2		;Stops 386/486 from crashing

	;; Load segment
	mov bx, 8
	mov ds, bx
	
	;; Go back into real mode
	and al, 0xFE
	mov cr0, eax
	
	;; Reset segment
	pop ds
	ret
	
	.unrealGDT:
	dq 0
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0
	
	.unrealGDTR:
	dw .unrealGDTR - .unrealGDT - 1
	dd .unrealGDT

;; Disk Address Packet - used for BIOS extended read, which provides support for LBA
dap:	
	.size: 		db 0x10
	.unused:	db 0
	.sectorCount:	dw 8
	.offset:	dw kernelHold
	.segment:	dw 0
	.lba:		dq 0

getMemoryMap:
	;; Get a map of high memory (>1 Meg) and set the GDT accordingly
	;; EBP used as counter
	
	mov ax, 0x1000
	mov es, ax
	mov di, 0

	mov ebx, 0
	mov ebp, 0

	;; Perform first call
	mov eax, 0x0000E820
	mov ecx, 24
	mov edx, 0x0534D4150
	int 0x15

	;; Check if there are any errors (EAX should be set to signature, "SMAP")
	cmp ebx, 0
	je .err
	cmp eax, 0x0534D4150
	jne .err
	jc .err
	
	call .getEntries
	call .success
	
	ret

	.getEntries:
	;; Finish off the last entry:
	
	;; Force valid ACPI 3.x entry
	mov [es:di + 20], dword 1

	;; Go to next (empty) entry on list
	add di, 24
	inc ebp

	;; Get the next entry:
	
	;; This stuff gets trashed after call
	mov eax, 0x0000E820
	mov ecx, 24
	;; Signature, "SMAP"
	mov edx, 0x0534D4150

	int 0x15

	;; Check if at end of list
	jc .ret
	cmp ebx, 0
	je .ret

	jmp .getEntries

	.ret: ret
	
	.err:
	mov dx, .msg0
	call textErr

	.success:
	;; TODO: merge overlapping or adjacent free memory
	;; NOTE: might need to save some registers, though ebp is fine
	;; Hackily converts text to numbers, assuming there are under 10 entries
	mov dx, .msg1
	mov bl, 00000010b
	mov ax, bp
	add ax, 0x30
	mov [.msg1Status], word ax
	call textPrint
	ret

	.msg0: db "Failed to get memory map!", 0
	.msg1: db "Successfully got memory map, with a entry count of: "
	.msg1Status: db "??", 0

prepGDT:
	;; Set GDT values based on memory map
	mov ax, 0x1000
	mov es, ax
	mov di, 0
	mov bx, 0
	mov ecx, 0
	;;inc bp

	call .findMem

	;;Check there is enough RAM
	;; (All segment should have been allocated)
	mov dx, .minRamErr
	mov ax, word[kernelSeg.limit_low]
	cmp ax, 0
	je textErr
	mov ax, word[gdtSeg.limit_low]
	cmp ax, 0
	je textErr
	mov ax, word[heapSeg.limit_low]
	cmp ax, 0
	je textErr
	mov ax, word[stackSeg.limit_low]
	cmp ax, 0
	je textErr

	mov dx, .gdtPrepSuccess
	mov bx, 00000010b
	call textPrint

	ret

	.minRamErr: db "Not enough RAM!", 0
	.gdtPrepSuccess: db "GDT successfully prepared with memory map!", 0

	.findMem:
	;; Check if we are at the end of the list
	cmp bp, bx
	je .ret
	inc bx

	;; Check mem's length fits in 32 bits?
	;; No, because if it doesn't it wont be used anyways (we have enough for all segments)

	jmp .accessableMem
	;; Check mem's base address fits in 32 bits (NOTE: CURRENTLY DISABLED IDK IT DOESNT WORK LOL BRUH )
	mov eax, dword [es:di]
	cmp eax, 0
	je .accessableMem
	jmp .checkNextMem

	.accessableMem:
	;; Check memory is usable
	mov eax, dword [es:di + 16]
	cmp eax, 1
	je .usableMem
	jmp .checkNextMem

	.usableMem:
	;; Fit as many segments as possible into this region
	mov eax, dword [es:di + 8]	; Space left in segment
	mov ecx, dword [es:di + 4]	; Low 32 bits of segment offset
	jmp .kernelCheck

	.kernelCheck:
	;; Check we havn't already set kernelSeg
	cmp word [kernelSeg.limit_low], 0
	jne .gdtCheck
	;; Check this mem is big enough for kernelSeg
	cmp eax, kernelSize
	jge .setKernel
	jmp .gdtCheck
	
	.setKernel:
	sub eax, kernelSize
	add ecx, kernelSize
	;; Load low 16 bits
	mov word [kernelSeg.base_low], cx
	;; Load mid 8 bits and hi 8 bits
	shr ecx, 16
	mov byte [kernelSeg.base_mid], cl
	mov byte [kernelSeg.base_hi], ch

	;; Load limit
	push eax
	push edx
	push ecx
	
	;;Calculate 4kb pages
	mov ecx, 4096
	mov edx, 0
	mov eax, kernelSize
	div ecx
	cmp edx, 0
	mov dx, .granularityNotDivisible
	jne textErr

	;;Load low 16 bits
	mov [kernelSeg.limit_low], ax
	;;Pull bits 16-20 into top of al
	mov ax, 0
	shr eax, 12
	mov dl, [kernelSeg.flags]
	;; Put top 4 bits of flags into low of dh
	shl dx, 4
	;; Load low 4 bit
	mov dl, al 
	;; Shift flags back to give us flags + low 4 bits of limit
	shr dx, 4
	mov [kernelSeg.flags], dl
	pop ecx
	pop edx
	pop eax
	jmp .gdtCheck

	.gdtCheck:
	;;Check we haven't already set gdtSeg
	cmp word [gdtSeg.limit_low], 0
	jne .heapCheck
	;;Check this mem is big enough for gdtSeg
	cmp eax, gdtSize
	jge .setGdt
	jmp .heapCheck
	
	.setGdt: 
	sub eax, gdtSize
	add ecx, gdtSize
	;; Load low 16 bits
	mov word [gdtSeg.base_low], cx
	;; Load mid 8 bits and hi 8 bits
	shr ecx, 16
	mov byte [gdtSeg.base_mid], cl
	mov byte [gdtSeg.base_hi], ch

	;; Set limit without granularity (gdt is too small)
	;;Load low 16 bits
	push eax
	push edx
	mov eax, gdtSize
	mov [gdtSeg.limit_low], ax
	;;Pull bits 16-20 into top of al
	mov ax, 0
	shr eax, 12
	mov dl, [gdtSeg.flags]
	;; Put top 4 bits of flags into low of dh
	shl dx, 4
	;; Load low 4 bit
	mov dl, al 
	;; Shift flags back to give us flags + low 4 bits of limit
	shr dx, 4
	mov [gdtSeg.flags], dl
	pop edx
	pop eax

	jmp .heapCheck

	.heapCheck:
	;;Check we haven't already set heapSeg
	cmp word [heapSeg.limit_low], 0
	jne .stackCheck
	;;Check this mem is big enough for heapSeg
	cmp eax, heapSize
	jge .setHeap
	jmp .stackCheck

	.setHeap:
	sub eax, heapSize
	add ecx, heapSize
	;; Load low 16 bits
	mov word [heapSeg.base_low], cx
	;; Load mid 8 bits hi 8 bits
	shr ecx, 16
	mov byte [heapSeg.base_mid], cl
	mov byte [heapSeg.base_hi], ch

	;; Load limit
	push eax
	push edx
	push ecx
	
	;;Calculate 4kb pages
	mov ecx, 4096
	mov edx, 0
	mov eax, heapSize
	div ecx
	cmp edx, 0
	mov dx, .granularityNotDivisible
	jne textErr

	;;Load low 16 bits
	mov [heapSeg.limit_low], ax
	;;Pull bits 16-20 into top of al
	mov ax, 0
	shr eax, 12
	mov dl, [heapSeg.flags]
	;; Put top 4 bits of flags into low of dh
	shl dx, 4
	;; Load low 4 bit
	mov dl, al 
	;; Shift flags back to give us flags + low 4 bits of limit
	shr dx, 4
	mov [heapSeg.flags], dl

	pop ecx
	pop edx
	pop eax

	jmp .stackCheck

	.stackCheck:
	;;Check we haven't already set te stackSeg
	cmp word [stackSeg.limit_low], 0
	jne .checkNextMem
	;;Check this mem is big enough for stackSeg
	cmp eax, stackSize
	jge .setStack
	jmp .checkNextMem

	.setStack:
	sub eax, stackSize
	add ecx, stackSize
	;; Load low 16 bits
	mov word [stackSeg.base_low], cx
	;; Load mid 8 bits and hi 8 bits
	shr ecx, 16
	mov byte [stackSeg.base_mid], cl
	mov byte [stackSeg.base_hi], ch

	;; Load limit
	push eax
	push edx
	push ecx
	
	;;Calculate 4kb pages
	mov ecx, 4096
	mov edx, 0
	mov eax, stackSize
	div ecx
	cmp edx, 0
	mov dx, .granularityNotDivisible
	jne textErr

	;;Load low 16 bits
	mov [stackSeg.limit_low], ax
	;;Pull bits 16-20 into top of al
	mov ax, 0
	shr eax, 12
	mov dl, [stackSeg.flags]
	;; Put top 4 bits of flags into low of dh
	shl dx, 4
	;; Load low 4 bit
	mov dl, al 
	;; Shift flags back to give us flags + low 4 bits of limit
	shr dx, 4
	mov [stackSeg.flags], dl

	pop ecx
	pop edx
	pop eax

	jmp .checkNextMem

	.checkNextMem:
	add di, 24
	jmp .findMem

	.granularityNotDivisible: db "GDT granularity pages are not even with target sizes of segments!", 0
	
	.ret: ret

	

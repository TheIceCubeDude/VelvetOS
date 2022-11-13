%include "structures/mmap.asm"
%include "structures/size_info.asm"	

mMap_prepareMemory:
	call getMemoryMap
	call prepMmap
	ret

mMap_relocateMmap:
	;; This will copy the GDT to the gdtSeg
	mov ecx, 0
	mov eax, [mmap.mmap]
	jmp .copyLoop
	
	.copyLoop:
	mov bl, [mmap + ecx]
	mov [eax + ecx], bl
	inc ecx
	cmp ecx, mmapSize
	jle .copyLoop
	ret

mMap_loadKernel:
	;; We will load 4KB of the kernel at a time to kernelHold
	;; After each 4KB framgment has been loaded, we copy it to kernelSeg
	;; Firstly, find out how many fragments we will have
	push dx
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
	;; Now copy the fragment in kernelHold to final destination
	;; Firstly, get the base location
        mov eax, [mmap.kernel]
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

mMap_enterUnrealMode:	
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
	
	mov ax, 0x8c0
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

prepMmap:
	;; Set mmap values based on memory map
	mov ax, 0x8c0
	mov es, ax
	mov di, 0
	mov bx, 0
	mov ecx, 0
	;;inc bp

	call .findMem

	;;Check there is enough RAM
	;; (All segment should have been allocated)
	mov dx, .minRamErr
	mov eax, dword [mmap.kernel]
	cmp eax, 0
	je textErr
	mov eax, dword [mmap.mmap]
	cmp eax, 0
	je textErr
	mov eax, dword [mmap.heap]
	cmp eax, 0
	je textErr
	mov eax, dword [mmap.stack]
	cmp eax, 0
	je textErr
	mov eax, dword [mmap.code]
	cmp eax, 0
	je textErr

	mov dx, .mmapPrepSuccess
	mov bx, 00000010b
	call textPrint

	ret

	.minRamErr: db "Not enough RAM!", 0
	.mmapPrepSuccess: db "Memory map succesfully prepared!", 0

	.findMem:
	;; Check if we are at the end of the list
	cmp bp, bx
	je .ret
	inc bx

	;; Check mem's base address fits in 32 bits
	mov eax, dword [es:di+4]
	cmp eax, 0
	je .accessableMem
	jmp .checkNextMem

	.accessableMem:
	;; Check memory is usable
	mov eax, dword [es:di + 16]
	cmp eax, 1
	jne .checkNextMem
	;; Check memory is high memory (prevent bootloader being overriden)
	mov eax, dword [es:di]
	cmp eax, 0xFFFFF
	jle .checkNextMem
	jmp .usableMem

	.usableMem:
	;; Fit as many segments as possible into this region
	mov eax, dword [es:di + 8]	; Space left in segment
	mov ecx, dword [es:di]		; Segment offset
	jmp .kernelCheck

	.kernelCheck:
	;; Check we havn't already set kernelSeg
	cmp word [mmap.kernel], 0
	jne .mmapCheck
	;; Check this mem is big enough for kernelSeg
	cmp eax, kernelSize
	jge .setKernel
	jmp .mmapCheck
	
	.setKernel:
	mov [mmap.kernel], ecx
	sub eax, kernelSize
	add ecx, kernelSize
	jmp .mmapCheck

	.mmapCheck:
	;;Check we haven't already set mmapSeg
	cmp word [mmap.mmap], 0
	jne .heapCheck
	;;Check this mem is big enough for mmapSeg
	cmp eax, mmapSize
	jge .setMmap
	jmp .heapCheck
	
	.setMmap: 
	mov [mmap.mmap], ecx
	sub eax, mmapSize
	add ecx, mmapSize
	jmp .heapCheck

	.heapCheck:
	;;Check we haven't already set heapSeg
	cmp dword [mmap.heap], 0
	jne .stackCheck
	;;Check this mem is big enough for heapSeg
	cmp eax, heapSize
	jge .setHeap
	jmp .stackCheck

	.setHeap:
	mov [mmap.heap], ecx
	sub eax, heapSize
	add ecx, heapSize
	jmp .stackCheck

	.stackCheck:
	;;Check we haven't already set te stackSeg
	cmp dword [mmap.stack], 0
	jne .codeCheck
	;;Check this mem is big enough for stackSeg
	cmp eax, stackSize
	jge .setStack
	jmp .codeCheck

	.setStack:
	mov dword [mmap.stack], ecx
	sub eax, stackSize
	add ecx, stackSize
	jmp .codeCheck

	.codeCheck:
	;; Check we havn't already set codeSeg
	cmp dword [mmap.code], 0
	jne .checkNextMem
	;; Check this mem is big enough for codeSeg
	cmp eax, codeSize
	jge .setCode
	jmp .checkNextMem
	
	.setCode:
	mov dword [mmap.code], ecx
	sub eax, codeSize
	add ecx, codeSize
	jmp .checkNextMem

	.checkNextMem:
	add di, 24
	jmp .findMem
	
	.ret: ret

	

mMap_prepareMemory:
	call getMemoryMap
	mov ecx, 3
	call getMmapSizes
	ret

mMap_relocateCoreinfo:
	;; This will copy the coreinfo to the coreinfo seg
	mov ecx, 0
	mov eax, [mmap.coreinfo]
	jmp .copyLoop
	
	.copyLoop:
	mov bl, [coreinfo + ecx]
	mov [eax + ecx], bl
	inc ecx
	cmp ecx, coreinfoSize
	jle .copyLoop
	ret

mMap_loadKernel:
	;; We will load 4KB of the kernel at a time to tmp
	;; After each 4KB framgment has been loaded, we copy it to kernelSeg
	push dx
	;; Before anything, check extended int 13h functions are supported
	mov ah, 0x41
	mov bx, 0x55AA
	int 0x13
	jc .unsupported

	;; Firstly, find out how many fragments we will have
	mov eax, [mmap.kernelSize]
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

	.unsupported:
	mov dx, .unsupportedErr
	call textErr

	.unsupportedErr: db "Int 13h extentions not supported on this BIOS!", 0

	.loadFragmentsLoop:
	;; Load fragment off disk to tmp
	push dx
	call .loadFragment
	;; Now copy the fragment in tmp to final destination
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
	mov bl, [tmp + ecx]
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
	.offset:	dw tmp
	.segment:	dw 0
	.lba:		dq 0

getMemoryMap:
	;; Get a map of high memory (>1 Meg) and set the GDT accordingly
	;; EBP used as counter
	
	mov ax, tmp
	mov dx, 0
	mov bx, 16
	div bx
	mov es, ax
	mov [.tmpSeg], ax
	mov di, 0

	mov ebx, 0
	mov ebp, 0

	;; Perform first call
	mov eax, 0xE820
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

	.tmpSeg: dw 0

	.getEntries:
	;; Finish off the last entry:
	
	;; Force valid ACPI 3.x entry
	mov [es:di + 20], dword 1

	;; Go to next (empty) entry on list
	add di, 24
	inc ebp

	;; Get the next entry:
	
	;; This stuff gets trashed after call
	mov eax, 0xE820
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
	mov dx, .msg1
	mov bl, 00000010b
	call textPrint
	ret

	.msg0: db "Failed to get memory map!", 0
	.msg1: db "Successfully got memory map!", 0

getMmapSizes:
	;; Set mmap values based on memory map
	push ecx
	mov ax, [getMemoryMap.tmpSeg]
	mov es, ax
	mov di, 0
	mov bx, 0
	mov ecx, 0
	;;inc bp

	call .findMem

	;; Reserve for coreinfo and kernel
	sub ecx, coreinfoSize
	sub ecx, [mmap.kernelSize]
	
	;; Certain fraction of memory for each thing
	mov eax, ecx
	mov edx, 0
	pop ecx
	div ecx
	mov [mmap.heapSize], eax
	mov [mmap.codeSize], eax
	mov [mmap.stackSize], eax
	jmp prepMmap

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
	cmp eax, 1048576
	jb .checkNextMem
	jmp .usableMem

	.usableMem:
	add ecx, [es:di + 12]
	jmp .checkNextMem

	.checkNextMem:
	add di, 24
	jmp .findMem

	.ret: ret

prepMmap:
	push ecx
	;; Zero out potentially set values in mmap
	mov [mmap.kernel], dword 0
	mov [mmap.coreinfo], dword 0
	mov [mmap.heap], dword 0
	mov [mmap.stack], dword 0
	mov [mmap.code], dword 0

	;; Set mmap values based on memory map
	mov ax, [getMemoryMap.tmpSeg]
	mov es, ax
	mov di, 0
	mov bx, 0
	mov ecx, 0
	;;inc bp

	call .findMem

	;;Checks
	;; (All segment should have been allocated)
	pop ecx
	mov eax, dword [mmap.kernel]
	cmp eax, 0
	je .notEnoughMem
	mov eax, dword [mmap.coreinfo]
	cmp eax, 0
	je .notEnoughMem
	mov eax, dword [mmap.heap]
	cmp eax, 0
	je .memTooFragmented
	mov eax, dword [mmap.stack]
	cmp eax, 0
	je .memTooFragmented
	mov eax, dword [mmap.code]
	cmp eax, 0
	je .memTooFragmented

	mov dx, .mmapPrepSuccess
	mov bx, 00000010b
	call textPrint

	ret

	.minRamErr: db "Not enough RAM!", 0
	.fragmentedMsg: db "Memory is too fragmented, allocating less memory (expect this many times)", 0
	.mmapPrepSuccess: db "Memory map succesfully prepared!", 0

	.notEnoughMem:
	mov dx, .minRamErr
	call textErr

	.memTooFragmented:
	push ecx
	push bp
	mov bl, 00001110b
	mov dx, .fragmentedMsg
	call textPrint
	pop bp
	pop ecx
	inc ecx
	jmp getMmapSizes

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
	cmp eax, 1048576
	jb .checkNextMem
	jmp .usableMem

	.usableMem:
	;; Fit as many segments as possible into this region
	mov eax, dword [es:di + 8]	; Space left in segment
	mov ecx, dword [es:di]		; Segment offset
	jmp .kernelCheck
	
	.alignMem:
	;; Aligns eax
	push edx
	push ebx
	push eax
	mov edx, 0
	mov ebx, 16
	div ebx
	cmp edx, 0
	je .alignMemOptimal
	mov eax, 16
	sub eax, edx
	mov edx, eax
	pop eax
	add eax, edx
	pop ebx
	pop edx
	ret

	.alignMemOptimal:
	pop eax
	pop ebx
	pop edx
	ret


	.kernelCheck:
	;; Check we havn't already set kernelSeg
	cmp dword [mmap.kernel], 0
	jne .coreinfoCheck
	;; Check this mem is big enough for kernelSeg
	cmp eax, [mmap.kernelSize]
	jae .setKernel
	jmp .coreinfoCheck
	
	.setKernel:
	mov [mmap.kernel], ecx
	sub eax, [mmap.kernelSize]
	add ecx, [mmap.kernelSize]
	;; Align kernel
	push eax
	mov eax, [mmap.kernel]
	call .alignMem
	mov [mmap.kernel], eax
	pop eax
	jmp .coreinfoCheck

	.coreinfoCheck:
	;;Check we haven't already set coreinfoSeg
	cmp dword [mmap.coreinfo], 0
	jne .heapCheck
	;;Check this mem is big enough for coreinfoSeg
	cmp eax, coreinfo
	jae .setCoreinfo
	jmp .heapCheck
	
	.setCoreinfo: 
	mov [mmap.coreinfo], ecx
	sub eax, coreinfoSize
	add ecx, coreinfoSize
	jmp .heapCheck

	.heapCheck:
	;;Check we haven't already set heapSeg
	cmp dword [mmap.heap], 0
	jne .stackCheck
	;;Check this mem is big enough for heapSeg
	cmp eax, [mmap.heapSize]
	jae .setHeap
	jmp .stackCheck

	.setHeap:
	mov [mmap.heap], ecx
	sub eax, [mmap.heapSize]
	add ecx, [mmap.heapSize]
	;; Align heap
	push eax
	mov eax, [mmap.heap]
	call .alignMem
	mov [mmap.heap], eax
	pop eax
	jmp .stackCheck

	.stackCheck:
	;;Check we haven't already set te stackSeg
	cmp dword [mmap.stack], 0
	jne .codeCheck
	;;Check this mem is big enough for stackSeg
	cmp eax, [mmap.stackSize]
	jae .setStack
	jmp .codeCheck

	.setStack:
	mov dword [mmap.stack], ecx
	sub eax, [mmap.stackSize]
	add ecx, [mmap.stackSize]
	jmp .codeCheck

	.codeCheck:
	;; Check we havn't already set codeSeg
	cmp dword [mmap.code], 0
	jne .checkNextMem
	;; Check this mem is big enough for codeSeg
	cmp eax, [mmap.codeSize]
	jae .setCode
	jmp .checkNextMem
	
	.setCode:
	mov dword [mmap.code], ecx
	sub eax, [mmap.codeSize]
	add ecx, [mmap.codeSize]
	;; Align code
	push eax
	mov eax, [mmap.code]
	call .alignMem
	mov [mmap.code], eax
	pop eax
	jmp .checkNextMem

	.checkNextMem:
	add di, 24
	jmp .findMem
	
	.ret: ret

	

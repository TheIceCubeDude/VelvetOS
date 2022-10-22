%include "structures/gdt.asm"
%include "structures/size_info.asm"	

mMap_prepareMemory:
	call getMemoryMap
	call prepGDT
	ret
	
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

	ret

	.minRamErr: db "Not enough RAM!", 0

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
	cmp eax, dword [kernelSize]
	jge .setKernel
	jmp .gdtCheck
	
	.setKernel:
	sub eax, [kernelSize]
	add ecx, [kernelSize]
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
	mov eax, dword [kernelSize]
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
	cmp eax, dword [gdtSize]
	jge .setGdt
	jmp .heapCheck
	
	.setGdt: 
	sub eax, [gdtSize]
	add ecx, [gdtSize]
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
	mov eax, dword [gdtSize]
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
	cmp eax, dword [heapSize]
	jge .setHeap
	jmp .stackCheck

	.setHeap:
	sub eax, [heapSize]
	add ecx, [heapSize]
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
	mov eax, dword [heapSize]
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
	cmp eax, dword [stackSize]
	jge .setStack
	jmp .checkNextMem

	.setStack:
	sub eax, [stackSize]
	add ecx, [stackSize]
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
	mov eax, dword [stackSize]
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

	

%include "structures/gdt.asm"

mMap_prepareMemory:
	call getMemoryMap
	call prepGDT
	ret
	
getMemoryMap:
	;; Get a map of high memory (>1 Meg) and set the GDT accordingly
	;; EBP used as counter
	
	mov ax, 0x8000
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
	jmp $
	ret

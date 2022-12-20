global enableSSE
global memcpy
global memset

enableSSE:
	push ebx
	;; Test for SSE 2.0  presence (26th bit.  SSE 1.0 is 25th bit)
	mov eax, 1
	cpuid
	test edx, 1<<26
	jz .fail
	;; Enable SSE
	mov eax, cr0
	and ax, 0xFFFB
	or ax, 2
	mov cr0, eax
	mov eax, cr4
	or ax, 3<<9
	mov cr4, eax
	mov eax, 1
	mov edx, 0
	pop ebx
	ret

	.fail:
	mov eax, 0
	mov edx, 0
	pop ebx
	ret

memcpy:
	push ebp
	mov ebp, esp
	push ebx
	mov eax, [ebp + 8] 	;;EAX -> destination
	mov ebx, [ebp + 12]	;;EBX -> source
	mov ecx, [ebp + 16]	;;ECX -> size in bytes
	;; Find how many bytes to pre-copy
	push eax
	push ecx
	mov edx, 0
	mov eax, ecx
	mov ecx, 16
	div ecx
	pop ecx
	pop eax
	sub ecx, edx
	cmp edx, 0
	jne .preCopy
	jmp .checkAlignment

	.preCopy:
	push ecx
	push edi
	push esi
	mov ecx, edx
	mov edi, eax
	mov esi, ebx

	rep movsb

	pop esi
	pop edi
	pop ecx
	add eax, edx
	add ebx, edx
	jmp .checkAlignment

	.checkAlignment:
	push eax
	push ecx
	mov edx, 0
	mov ecx, 16
	div ecx
	pop ecx
	pop eax
	cmp edx, 0
	jne .copyUnalignedPrep
	push eax
	push ecx
	mov edx, 0
	mov ecx, 16
	mov eax, ebx
	div ecx
	pop ecx
	pop eax
	cmp edx, 0
	jne .copyUnalignedPrep
	jmp .copyAlignedPrep

	.copyUnalignedPrep:
	mov ebp, ecx
	mov ecx, 0
	mov edx, 16
	jmp .copyUnaligned

	.copyAlignedPrep:
	mov ebp, ecx
	mov ecx, 0
	mov edx, 16
	jmp .copyAligned
	
	.copyAligned:
	movdqa xmm0, [ebx + ecx]
	movdqa [eax + ecx], xmm0
	add ecx, edx
	cmp ecx, ebp
	jbe .copyAligned
	jmp .ret

	.copyUnaligned:
	movdqu xmm0, [ebx + ecx]
	movdqu [eax + ecx], xmm0
	add ecx, edx
	cmp ecx, ebp
	jbe .copyUnaligned
	jmp .ret

	.ret: 
	mov eax, 0
	mov edx, 0
	pop ebx
	pop ebp
	ret

memset:
	push ebp
	mov ebp, esp
	push ebx
	mov eax, [ebp + 8] 	;;EAX -> destination
	mov ebx, [ebp + 12]	;;EBX -> x
	mov ecx, [ebp + 16]	;;ECX -> size in bytes
	;; Find out how many dwords need to be pre-set
	push eax
	push ecx
	mov eax, ecx
	mov edx, 0
	mov ecx, 16
	div ecx
	mov eax, edx
	mov ecx, 4
	div ecx
	cmp edx, 0
	jne .err
	mov edx, eax
	pop ecx
	pop eax
	mov ebp, 0
	cmp edx, 0
	;; EDX -> # of dwords to pre-set
	jne .preSet
	jmp .checkAlignment

	.preSet:
	mov [eax + ebp], ebx
	add ebp, 4
	cmp ebp, edx
	jne .preSet
	mov [eax + ebp], ebx
	push eax
	push ecx
	mov eax, edx
	mov edx, 0
	mov ecx, 4
	mul ecx
	mov edx, eax
	pop ecx
	pop eax
	sub eax, edx
	jmp .checkAlignment

	.checkAlignment:
	push eax
	push ecx
	mov edx, 0
	mov ecx, 4
	div ecx
	pop ecx
	pop eax
	cmp edx, 0
	je .prepAligned
	jmp .prepUnaligned

	.prepAligned:
	movd xmm0, ebx
	unpcklps xmm0, xmm0
	unpcklpd xmm0, xmm0
	mov ebp, 0
	mov edx, 16
	jmp .alignedSet
	
	.prepUnaligned:
	movd xmm0, ebx
	unpcklps xmm0, xmm0
	unpcklpd xmm0, xmm0
	mov ebp, 0
	mov edx, 16
	jmp .unalignedSet

	.alignedSet:
	movdqa [eax + ebp], xmm0
	add ebp, edx
	cmp ebp, ecx
	jbe .alignedSet
	jmp .ret

	.unalignedSet:
	movdqu [eax + ebp], xmm0
	add ebp, edx
	cmp ebp, ecx
	jbe .unalignedSet
	jmp .ret

	.ret: 
	mov eax, 0
	mov edx, 0
	pop ebx
	pop ebp
	ret

	.err:
	;; Size must be a multiple of 4 (memset operates on dwords)
	mov eax, 1
	mov edx, 0
	pop ebx
	pop ebp
	ret
	

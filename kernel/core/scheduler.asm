global yield
global exitKernelAsm
extern getNextProcess

yield:
	pushad
	push esp
	call getNextProcess
	pop ebx
	mov esp, eax
	popad
	ret 
	
exitKernelAsm:
	mov eax, [esp + 4]
	mov esp, eax
	popad
	ret
	
	

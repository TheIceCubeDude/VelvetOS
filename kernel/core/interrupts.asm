global loadIdt
global enableIrqs
global disableIrqs

extern printf

loadIdt:
	push ebp
	mov ebp, esp
	mov eax, [ebp + 8]
	lidt [eax]
	pop ebp
	ret

enableIrqs: 
	sti
	ret

disableIrqs:
	cli
	ret

;; itrWrapper:
;;	pushad
;;	cld
;;	call interruptHandler
;;	popad
;;	iret

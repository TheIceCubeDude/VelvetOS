global io_wait
global outb
global outw
global outl
global inb
global inw
global inl

io_wait:
	;; IO is slow, so it can be used as a crude delay
	mov al, 0
	out 0x80, al
	ret

outb:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	mov eax, [ebp + 12]
	out dx, al
	pop ebp
	ret
outw:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	mov eax, [ebp + 12]
	out dx, ax
	pop ebp
	ret
outl:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	mov eax, [ebp + 12]
	out dx, eax
	pop ebp
	ret

inb:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	mov eax, 0
	in al, dx
	mov edx, 0
	pop ebp
	ret

inw:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	mov eax, 0
	in ax, dx
	mov edx, 0
	pop ebp
	ret

inl:
	push ebp
	mov ebp, esp
	mov edx, [ebp + 8]
	in eax, dx
	mov edx, 0
	pop ebp
	ret


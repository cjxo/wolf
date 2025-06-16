.code
; rcx = RState
; edx = X
; r8d = Y
; r9d = W
; H
; Colour
R_FillRectangle proc
	sub rsp, 40
	; rcx = RState
	; edx = XStart = X
	; r8d = YStart = Y
	; r9d = XEnd = X + W
	; ... = YEnd = Y + H
	add r9d, edx
	add dword ptr [rsp + 80], r8d

	xor rax, rax
	cmp edx, 0
	cmovl edx, eax
	cmp r8d, 0
	cmovl r9d, eax
	mov eax, dword ptr [rcx + 8]
	dec eax
	cmp r9d, eax
	cmovge r9d, eax
	mov eax, dword ptr [rcx + 12]
	dec eax
	mov r10d, dword ptr [rsp + 80]
	cmp r10d, eax
	cmovge r10d, eax
	mov dword ptr [rsp + 80], r10d

	mov eax, dword ptr [rcx + 8]
	shl rax, 2
	mov qword ptr [rsp + 0], rax

	mov r10d, edx
	mov eax, dword ptr [rcx + 8]
	mul r8d
	add eax, r10d
	shl rax, 2
	mov edx, r10d

	mov r10, qword ptr [rcx]
	add r10, rax

	mov dword ptr [rsp + 8], edx
	mov eax, dword ptr [rsp + 88]

	jmp loop_y_test
loop_y:
	inc r8d
	mov edx, dword ptr [rsp + 8]
	mov r11, r10
	jmp loop_x_test
loop_x:
	inc edx
	mov dword ptr [r11], eax
	add r11, 4

loop_x_test:
	cmp edx, r9d
	jle loop_x
	add r10, qword ptr [rsp + 0]

loop_y_test:
	cmp r8d, dword ptr [rsp + 80]
	jle loop_y

	add rsp, 40
	ret
R_FillRectangle endp
end
.code

; rcx = RState
; edx = X
; r8d = Y
; r9d = W
; H
; Colour
R_WireRectangle proc
	sub rsp, 40
	; rcx = RState
	; edx = XStart = X
	; r8d = YStart = Y
	; r9d = XEnd = X + W
	; [rsp + 80] = YEnd = Y + H
	; [rsp + 0] = XEnd2 = XEnd
	; [rsp + 4] = YEnd2 = YEnd
	; [rsp + 8] = Pitch

	cmp r9d, 0
	jle done
	cmp dword ptr [rsp + 80], 0
	jle done

	mov eax, dword ptr [rcx + 8]
	dec eax
	mov dword ptr [rsp + 8], eax
	mov eax, dword ptr [rcx + 12]
	dec eax
	mov dword ptr [rsp + 12], eax

	xor rax, rax
	add r9d, edx                     ; XEnd
	mov dword ptr [rsp + 0], r9d     ; XEnd2
	add dword ptr [rsp + 80], r8d    ; YEnd
	mov eax, dword ptr [rsp + 80]
	mov dword ptr [rsp + 4], eax     ; YEnd2

	cmp edx, dword ptr [rcx + 8]
	jge done
	cmp r8d, dword ptr [rcx + 12]
	jge done
	cmp r9d, 0
	jl done
	cmp dword ptr [rsp + 4], 0
	jl done

	xor eax, eax
	cmp edx, 0
	cmovl edx, eax
	cmp r8d, 0
	cmovl r8d, eax
	
	cmp r9d, dword ptr [rcx + 8]
	cmovge r9d, dword ptr [rsp + 8]

	mov eax, dword ptr [rsp + 80]
	cmp eax, dword ptr [rcx + 12]
	cmovge eax, dword ptr [rsp + 12]
	mov dword ptr [rsp + 80], eax
	
	mov eax, dword ptr [rcx + 8]
	shl rax, 2
	mov qword ptr [rsp + 16], rax
	
	mov dword ptr [rsp + 24], edx
	mov eax, dword ptr [rcx + 8]
	mul r8d
	add eax, dword ptr [rsp + 24]
	shl eax, 2
	add rax, qword ptr [rcx]
	mov r10, rax

	cmp r8d, 0
	jl x_ge_0
	mov r11, r10
	mov edx, dword ptr [rsp + 24]
	mov eax, dword ptr [rsp + 88]
	jmp y_ge_0_test
y_ge_0_loop:
	inc edx
	mov dword ptr [r11], eax
	add r11, 4
y_ge_0_test:
	cmp edx, r9d
	jle y_ge_0_loop

x_ge_0:
	cmp dword ptr [rsp + 24], 0
	jl xend_l_width
	mov r11, r10
	mov edx, r8d
	jmp x_ge_0_test
x_ge_0_loop:
	inc edx
	mov dword ptr [r11], eax
	add r11, qword ptr [rsp + 16]
x_ge_0_test:
	cmp edx, dword ptr [rsp + 80]
	jle x_ge_0_loop

xend_l_width:
	mov eax, [rcx + 8]
	cmp dword ptr [rsp + 0], eax
	jge yend_l_height

	mov eax, r9d
	sub eax, dword ptr [rsp + 24]
	shl rax, 2
	mov r11, r10
	add r11, rax
	mov eax, dword ptr [rsp + 88]
	mov edx, r8d
	jmp xend_l_width_test
xend_l_width_loop:
	inc edx
	mov dword ptr [r11], eax
	add r11, qword ptr [rsp + 16]
xend_l_width_test:
	cmp edx, dword ptr [rsp + 80]
	jle xend_l_width_loop

yend_l_height:
	mov eax, dword ptr [rcx + 12]
	cmp dword ptr [rsp + 4], eax
	jge done

	mov eax, dword ptr [rsp + 80]
	sub eax, r8d
	mul qword ptr [rsp + 16]
	mov r11, r10
	add r11, rax
	mov eax, dword ptr [rsp + 88]
	mov edx, dword ptr [rsp + 24]
	jmp yend_l_height_test
yend_l_height_loop:
	inc edx
	mov dword ptr [r11], eax
	add r11, 4
yend_l_height_test:
	cmp edx, r9d
	jle yend_l_height_loop

done:
	add rsp, 40
	ret
R_WireRectangle endp

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

	mov eax, dword ptr [rcx + 8]
	dec eax
	mov dword ptr [rsp + 12], eax

	mov eax, dword ptr [rcx + 12]
	dec eax
	mov dword ptr [rsp + 16], eax

	xor rax, rax
	cmp edx, 0
	cmovl edx, eax
	cmp r8d, 0
	cmovl r8d, eax
	cmp r9d, dword ptr [rcx + 8]
	cmovge r9d, dword ptr [rsp + 12]
	mov eax, dword ptr [rsp + 80]
	cmp eax, dword ptr [rcx + 12]
	cmovge eax, dword ptr [rsp + 16]
	mov dword ptr [rsp + 80], eax

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

; rcx = RState
; edx = X
; r8d = YStart
; r9d = YEnd
; Colour
R_VerticalLine proc
	cmp edx, 0
	jl done
	cmp edx, dword ptr [rcx + 8]
	jge done
	
	cmp r8d, r9d
	jle @0
	xchg r8d, r9d

@0:
; begin jumps
;	mov eax, dword ptr [rcx + 12]
;	cmp r8d, 0
;	jge y_start_ge
;	xor r8d, r8d
;y_start_ge:
;	cmp r8d, eax
;	cmovg r8d, eax
;	cmp r9d, 0
;	jge y_end_ge
;	xor r9d, r9d
;y_end_ge:
;	cmp r9d, eax
;	cmovg r9d, eax
; end jumps

; begin conditional moves
	xor rax, rax
	cmp r8d, 0
	cmovl r8d, eax
	cmp r9d, 0
	cmovl r9d, eax
	mov eax, dword ptr [rcx + 12]
	cmp r8d, eax
	cmovg r8d, eax
	cmp r9d, eax
	cmovg r9d, eax
; end conditional moves

	mov r10d, edx
	mov eax, dword ptr [rcx + 8]
	mul r8d
	add eax, r10d
	shl rax, 2
	add rax, qword ptr [rcx]
	mov rdx, rax
	mov r10d, dword ptr [rsp + 40]
	mov eax, dword ptr [rcx + 8]
	shl rax, 2

	jmp draw_loop_test
draw_loop:
	inc r8d
	mov dword ptr [rdx], r10d
	add rdx, rax
draw_loop_test:
	cmp r8d, r9d
	jl draw_loop

done:
	ret
R_VerticalLine endp
end
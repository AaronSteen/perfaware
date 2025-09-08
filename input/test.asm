bits 16

; Group 1 — r/m ↔ reg

mov si, bx
mov dh, al
mov al, [bx + si]
mov bx, [bp + di]
mov dx, [bp]
mov ah, [bx + si + 4]
mov al, [bx + si + 4999]
mov [bx + di], cx
mov [bp + si], cl
mov [bp], ch

; Group 2 — imm → r/m

mov [bp + di], byte 7
mov [di + 901], word 347
add si, 5
add ah, 30
adc si, 5
adc ah, 30
sub si, 5
sub ah, 30
sbb si, 5
sbb ah, 30

; Group 3 — unary r/m

inc ax
inc cx
inc dh
inc byte [bp + 1002]
inc word [bx + 39]
dec ax
neg ax
not ah
mul cx
shr ax, 1

; Group 4 — accumulator imm

add ax, 1000
add al, 9
adc ax, 1000
adc al, 9
sub ax, 1000
sub al, 9
sbb ax, 1000
sbb al, 9
cmp ax, 23909
and al, 93

; Group 5 — reg encoded in opcode

mov cl, 12
mov ch, -12
mov cx, 12
mov cx, -12
mov dx, 3948
mov dx, -3948
push cx
push ax
push dx
push cs

; Group 6 — one-byte no operands

xlat
lahf
sahf
pushf
popf
aaa
daa
aas
das
nop

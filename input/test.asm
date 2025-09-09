; Group 1 — r/m ↔ reg

bits 16

mov si, bx
; hex: 89 DE
; bin: [1000 1001] [1101 1110]

mov dh, al
; hex: 88 C6
; bin: [1000 1000] [1100 0110]

mov al, [bx + si]
; hex: 8A 00
; bin: [1000 1010] [0000 0000]

mov bx, [bp + di]
; hex: 8B 1B
; bin: [1000 1011] [0001 1011]

mov dx, [bp]
; hex: 8B 56 00
; bin: [1000 1011] [0101 0110] [0000 0000]

mov ah, [bx + si + 4]
; hex: 8A 60 04
; bin: [1000 1010] [0110 0000] [0000 0100]

mov al, [bx + si + 4999]
; hex: 8A 80 87 13
; bin: [1000 1010] [1000 0000] [1000 0111] [0001 0011]

mov [bx + di], cx
; hex: 89 09
; bin: [1000 1001] [0000 1001]

mov [bp + si], cl
; hex: 88 0A
; bin: [1000 1000] [0000 1010]

mov [bp], ch
; hex: 88 6E 00
; bin: [1000 1000] [0110 1110] [0000 0000]

; Group 2 — imm → r/m

mov [bp + di], byte 7
; hex: C6 03 07
; bin: [1100 0110] [0000 0011] [0000 0111]

mov [di + 901], word 347
; hex: C7 85 85 03 5B 01
; bin: [1100 0111] [1000 0101] [1000 0101] [0000 0011] [0101 1011] [0000 0001]

add si, 5
; hex: 83 C6 05
; bin: [1000 0011] [1100 0110] [0000 0101]

add ah, 30
; hex: 80 C4 1E
; bin: [1000 0000] [1100 0100] [0001 1110]

adc si, 5
; hex: 83 D6 05
; bin: [1000 0011] [1101 0110] [0000 0101]

adc ah, 30
; hex: 80 D4 1E
; bin: [1000 0000] [1101 0100] [0001 1110]

sub si, 5
; hex: 83 EE 05
; bin: [1000 0011] [1110 1110] [0000 0101]

sub ah, 30
; hex: 80 EC 1E
; bin: [1000 0000] [1110 1100] [0001 1110]

sbb si, 5
; hex: 83 DE 05
; bin: [1000 0011] [1101 1110] [0000 0101]

sbb ah, 30
; hex: 80 DC 1E
; bin: [1000 0000] [1101 1100] [0001 1110]

; Group 3 — unary r/m

inc ax
; hex: 40
; bin: [0100 0000]

inc cx
; hex: 41
; bin: [0100 0001]

inc dh
; hex: FE C6
; bin: [1111 1110] [1100 0110]

inc byte [bp + 1002]
; hex: FE 86 EA 03
; bin: [1111 1110] [1000 0110] [1110 1010] [0000 0011]

inc word [bx + 39]
; hex: FF 47 27
; bin: [1111 1111] [0100 0111] [0010 0111]

dec ax
; hex: 48
; bin: [0100 1000]

neg ax
; hex: F7 D8
; bin: [1111 0111] [1101 1000]

not ah
; hex: F6 D4
; bin: [1111 0110] [1101 0100]

mul cx
; hex: F7 E1
; bin: [1111 0111] [1110 0001]

shr ax, 1
; hex: D1 E8
; bin: [1101 0001] [1110 1000]

; Group 4 — accumulator imm

add ax, 1000
; hex: 05 E8 03
; bin: [0000 0101] [1110 1000] [0000 0011]

add al, 9
; hex: 04 09
; bin: [0000 0100] [0000 1001]

adc ax, 1000
; hex: 15 E8 03
; bin: [0001 0101] [1110 1000] [0000 0011]

adc al, 9
; hex: 14 09
; bin: [0001 0100] [0000 1001]

sub ax, 1000
; hex: 2D E8 03
; bin: [0010 1101] [1110 1000] [0000 0011]

sub al, 9
; hex: 2C 09
; bin: [0010 1100] [0000 1001]

sbb ax, 1000
; hex: 1D E8 03
; bin: [0001 1101] [1110 1000] [0000 0011]

sbb al, 9
; hex: 1C 09
; bin: [0001 1100] [0000 1001]

cmp ax, 23909
; hex: 3D 65 5D
; bin: [0011 1101] [0110 0101] [0101 1101]

and al, 93
; hex: 24 5D
; bin: [0010 0100] [0101 1101]

; Group 5 — reg encoded in opcode

mov cl, 12
; hex: B1 0C
; bin: [1011 0001] [0000 1100]

mov ch, -12
; hex: B5 F4
; bin: [1011 0101] [1111 0100]

mov cx, 12
; hex: B9 0C 00
; bin: [1011 1001] [0000 1100] [0000 0000]

mov cx, -12
; hex: B9 F4 FF
; bin: [1011 1001] [1111 0100] [1111 1111]

mov dx, 3948
; hex: BA 6C 0F
; bin: [1011 1010] [0110 1100] [0000 1111]

mov dx, -3948
; hex: BA 94 F0
; bin: [1011 1010] [1001 0100] [1111 0000]

push cx
; hex: 51
; bin: [0101 0001]

push ax
; hex: 50
; bin: [0101 0000]

push dx
; hex: 52
; bin: [0101 0010]

push cs
; hex: 0E
; bin: [0000 1110]

; Group 6 — one-byte no operands

xlat
; hex: D7
; bin: [1101 0111]

lahf
; hex: 9F
; bin: [1001 1111]

sahf
; hex: 9E
; bin: [1001 1110]

pushf
; hex: 9C
; bin: [1001 1100]

popf
; hex: 9D
; bin: [1001 1101]

aaa
; hex: 37
; bin: [0011 0111]

daa
; hex: 27
; bin: [0010 0111]

aas
; hex: 3F
; bin: [0011 1111]

das
; hex: 2F
; bin: [0010 1111]

nop
; hex: 90
; bin: [1001 0000]

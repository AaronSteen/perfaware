; ========================================================================
;
; (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Please see https://computerenhance.com for further information
;
; ========================================================================

; ========================================================================
; LISTING 42
; ========================================================================

;
; NOTE(casey): This is not meant to be a real compliance test for 8086
; disassemblers. It's just a reasonable selection of opcodes and patterns
; to use as a first pass in making sure a disassembler handles a large
; cross-section of the encoding. To be absolutely certain you haven't
; missed something, you would need a more exhaustive listing!
;

bits 16

; 0
mov si, bx
mov dh, al
mov cl, 12
mov ch, -12
mov cx, 12

; 5
mov cx, -12
mov dx, 3948
mov dx, -3948
mov al, [bx + si]
mov bx, [bp + di]

; 10
mov dx, [bp]
mov ah, [bx + si + 4] ; 11
mov al, [bx + si + 4999] ; 12
mov [bx + di], cx
mov [bp + si], cl

; 15
mov [bp], ch
mov ax, [bx + di - 37]
mov [si - 300], cx
mov dx, [bx - 32]
mov [bp + di], byte 7

;20
mov [di + 901], word 347
mov bp, [5] ; 21
mov bx, [3458]
mov ax, [2555]
mov ax, [16]
mov [2554], ax
mov [15], ax

;27
push word [bp + si]
push word [3000]
push word [bx + di - 30]
push cx
push ax
push dx
push cs

; 34
pop word [bp + si]
pop word [3]
pop word [bx + di - 3000]
pop sp 
pop di
pop si
pop ds

; 41
xchg ax, [bp - 1000]
xchg [bx + 50], bp
xchg ax, ax
xchg ax, dx
xchg ax, sp
xchg ax, si
xchg ax, di
xchg cx, dx
xchg si, cx
xchg cl, ah

; 51
in al, 200
in al, dx
in ax, dx

; 54
out 44, ax
out dx, al

; 56
xlat
lea ax, [bx + di + 1420]
lea bx, [bp - 50]
lea sp, [bp - 1003]
lea di, [bx + si - 7]

; 61
lds ax, [bx + di + 1420]
lds bx, [bp - 50]
lds sp, [bp - 1003]
lds di, [bx + si - 7]

; 65
les ax, [bx + di + 1420]
les bx, [bp - 50]
les sp, [bp - 1003]
les di, [bx + si - 7]

; 69
lahf
sahf
pushf
popf

; 73
add cx, [bp]
add dx, [bx + si]
add [bp + di + 5000], ah
add [bx], al
add sp, 392

; 78
add si, 5
add ax, 1000
add ah, 30
add al, 9
add cx, bx

; 83
add ch, al

; 84
adc cx, [bp]
adc dx, [bx + si]
adc [bp + di + 5000], ah
adc [bx], al
adc sp, 392
adc si, 5
adc ax, 1000
adc ah, 30
adc al, 9
adc cx, bx
adc ch, al

; 95
inc ax
inc cx
inc dh
inc al
inc ah

; 100
inc sp
inc di
inc byte [bp + 1002]
inc word [bx + 39]
inc byte [bx + si + 5]

; 105
inc word [bp + di - 10044]
inc word [9349]
inc byte [bp]

; 108
aaa
daa

; 110
sub cx, [bp]
sub dx, [bx + si]
sub [bp + di + 5000], ah
sub [bx], al
sub sp, 392

; 115
sub si, 5
sub ax, 1000
sub ah, 30
sub al, 9
sub cx, bx

; 120
sub ch, al

; 121
sbb cx, [bp]
sbb dx, [bx + si]
sbb [bp + di + 5000], ah
sbb [bx], al
sbb sp, 392

; 126
sbb si, 5
sbb ax, 1000
sbb ah, 30
sbb al, 9
sbb cx, bx

; 131
sbb ch, al

; 131
dec ax
dec cx
dec dh
dec al
dec ah

; 136
dec sp
dec di
dec byte [bp + 1002]
dec word [bx + 39]
dec byte [bx + si + 5]

; 141
dec word [bp + di - 10044]
dec word [9349]
dec byte [bp]

; 144
neg ax
neg cx
neg dh
neg al
neg ah

; 149
neg sp
neg di
neg byte [bp + 1002]
neg word [bx + 39]
neg byte [bx + si + 5]

; 154
neg word [bp + di - 10044]
neg word [9349]
neg byte [bp]

; 157
cmp bx, cx
cmp dh, [bp + 390]
cmp [bp + 2], si
cmp bl, 20
cmp byte [bx], 34

; 162
cmp ax, 23909

; 163
aas
das

; 165
mul al
mul cx
mul word [bp]
mul byte [bx + di + 500]

; 169
imul ch
imul dx
imul byte [bx]
imul word [9483]

; 173
aam

; 174
div bl
div sp
div byte [bx + si + 2990]
div word [bp + di + 1000]

; 178
idiv ax
idiv si
idiv byte [bp + si]
idiv word [bx + 493]

; 182
aad
cbw
cwd

; 186
not ah
not bl
not sp
not si
not word [bp]

; 191
not byte [bp + 9905]

; 192
shl ah, 1
shr ax, 1
sar bx, 1
rol cx, 1
ror dh, 1

; 197
rcl sp, 1
rcr bp, 1

; 199
shl word [bp + 5], 1
shr byte [bx + si - 199], 1
sar byte [bx + di - 300], 1
rol word [bp], 1
ror word [4938], 1

; 204
rcl byte [3], 1
rcr word [bx], 1

; 206
shl ah, cl
shr ax, cl
sar bx, cl
rol cx, cl
ror dh, cl

; 211
rcl sp, cl
rcr bp, cl

; 213
shl word [bp + 5], cl
shr word [bx + si - 199], cl
sar byte [bx + di - 300], cl
rol byte [bp], cl
ror byte [4938], cl

; 218
rcl byte [3], cl
rcr word [bx], cl

; 220
and al, ah
and ch, cl
and bp, si
and di, sp
and al, 93

; 225
and ax, 20392
and [bp + si + 10], ch
and [bx + di + 1000], dx
and bx, [bp]
and cx, [4384]

; 230
and byte [bp - 39], 239
and word [bx + si - 4332], 10328

; 232
test bx, cx
test dh, [bp + 390]
test [bp + 2], si
test bl, 20
test byte [bx], 34

; 237
test ax, 23909

; 238
or al, ah
or ch, cl
or bp, si
or di, sp
or al, 93

; 243
or ax, 20392
or [bp + si + 10], ch
or [bx + di + 1000], dx
or bx, [bp]
or cx, [4384]

; 248
or byte [bp - 39], 239
or word [bx + si - 4332], 10328

; 250
xor al, ah
xor ch, cl
xor bp, si
xor di, sp
xor al, 93

; 255
xor ax, 20392
xor [bp + si + 10], ch
xor [bx + di + 1000], dx
xor bx, [bp]
xor cx, [4384]

; 230
xor byte [bp - 39], 239
xor word [bx + si - 4332], 10328

; 232
rep movsb
rep cmpsb
rep scasb
rep lodsb
rep movsw

; 237
rep cmpsw
rep scasw
rep lodsw

; NOTE(casey): Special thanks (as always!) to Mārtiņš Možeiko for figuring out why NASM
; wouldn't compile "rep stds" instructions. It was because it was a misprint in the 8086
; manual! It was really just "rep stos", which of course is still in x64, and NASM
; assembles it just fine.

; 240
rep stosb
rep stosw

; 242
call [39201]
call [bp - 100]
call sp
call ax

; 246
jmp ax
jmp di
jmp [12]
jmp [4395]

; 250    
ret -7
ret 500
ret


label:
; 253
je label
jl label
jle label
jb label
jbe label

; 258
jp label
jo label
js label
jne label
jnl label

; 263
jg label
jnb label
ja label
jnp label
jno label

; 268
jns label
loop label
loopz label
loopnz label
jcxz label

; 273    
int 13
int3

; 275
into
iret

; 277
clc
cmc
stc
cld
std

; 283
cli
sti
hlt
wait

; 287
lock not byte [bp + 9905]
lock xchg [100], al

; 289
mov al, cs:[bx + si]
mov bx, ds:[bp + di]
mov dx, es:[bp]
mov ah, ss:[bx + si + 4]

; 293
and ss:[bp + si + 10], ch
or ds:[bx + di + 1000], dx
xor bx, es:[bp]
cmp cx, es:[4384]
test byte cs:[bp - 39], 239

; 298
sbb word cs:[bx + si - 4332], 10328

; 299
lock not byte CS:[bp + 9905]

;
; NOTE(casey): These were not in the original homework set, but have been added since, as people
; found instruction encodings that were not previously covered. Thank you to everyone who
; submitted test cases!
;

; 300
call 123:456
jmp 789:34

; 302
mov [bx+si+59],es

; 303
jmp 2620
call 11804

; 305
retf 17556
ret 17560
retf
ret

; 309
call [bp+si-0x3a]
call far [bp+si-0x3a]
jmp [di]
jmp far [di]

; 313
jmp 21862:30600

;
; TODO(casey): I would like to uncomment this, but as far as I can tell, NASM doesn't recognize the ESC instruction :(
; so even if I just force the assembler to output the bits here, our disasm will fail to assemble because it will (correctly!)
; print the esc instruction and NASM will error because it doesn't know what that is.
;
; esc 938,ax
;

;
; TODO(casey): According to NASM, "rep movsb" is "not lockable". However the 8086 manual seems to think it is, and
; even describes what happens when you you lock a rep: the lock is held for the duration of the rep operation. So...
; yeah. Not sure why this doesn't work in NASM:
;
; lock rep movsb
;
bits 16

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

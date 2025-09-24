; ========================================================================
; 8086 test set — 9 decode groups (10 instructions each)
; Source: listing_0042_completionist_decode.asm
; Dialect: NASM
; ========================================================================

bits 16

; ------------------------------------------------------------------------
; Group 5 — reg encoded in opcode, no trailing data
; (INC/DEC r16, PUSH/POP r16, XCHG AX,r16, PUSH/POP Sreg)
; ------------------------------------------------------------------------

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

pop sp
; hex: 5C
; bin: [0101 1100]

pop di
; hex: 5F
; bin: [0101 1111]

pop si
; hex: 5E
; bin: [0101 1110]

pop ds
; hex: 1F
; bin: [0001 1111]

xchg ax, dx
; hex: 92
; bin: [1001 0010]

xchg ax, si
; hex: 96
; bin: [1001 0110]

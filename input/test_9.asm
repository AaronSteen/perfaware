; ========================================================================
; 8086 test set — 9 decode groups (10 instructions each)
; Source: listing_0042_completionist_decode.asm
; Dialect: NASM
; ========================================================================

bits 16

; ------------------------------------------------------------------------
; Group 9 — control transfer & I/O (Jcc/loop/jcxz, call/jmp/ret/int, in/out)
; ------------------------------------------------------------------------

in al, 200
; hex: E4 C8
; bin: [1110 0100] [1100 1000]
; 

in al, dx
; hex: EC
; bin: [1110 1100]

in ax, dx
; hex: ED
; bin: [1110 1101]

out 44, ax
; hex: E7 2C
; bin: [1110 0111] [0010 1100]

out dx, al
; hex: EE
; bin: [1110 1110]

call [39201]
; hex: FF 16 71 99
; bin: [1111 1111] [0001 0110] [0111 0001] [1001 1001]

jmp [4395]
; hex: FF 26 2B 11
; bin: [1111 1111] [0010 0110] [0010 1011] [0001 0001]

ret
; hex: C3
; bin: [1100 0011]

je label
; hex: 74 00
; bin: [0111 0100] [0000 0000]

loop label
; hex: E2 FE
; bin: [1110 0010] [1111 1110]

label:

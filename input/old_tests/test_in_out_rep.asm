; ========================================================================
; 8086 test set — 9 decode groups (10 instructions each)
; Source: listing_0042_completionist_decode.asm
; Dialect: NASM
; ========================================================================

bits 16

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

; 240
rep stosb
rep stosw

; 51
in al, 200
in al, dx
in ax, dx

; 54
out 44, ax
out dx, al


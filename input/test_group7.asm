; ========================================================================
; 8086 test set — 9 decode groups (10 instructions each)
; Source: listing_0042_completionist_decode.asm
; Dialect: NASM
; ========================================================================

bits 16

; ------------------------------------------------------------------------
; Group 7 — single-byte, no ModRM (misc, prefixes, strings)
; (REP/REPNZ + string ops here; LOCK-prefixed r/m ops counted with their core groups)
; ------------------------------------------------------------------------

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

wait
; hex: 9B
; bin: [1001 1011]

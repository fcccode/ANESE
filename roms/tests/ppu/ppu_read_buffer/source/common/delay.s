; Delays in CPU clocks, milliseconds, etc. All routines are re-entrant
; (no global data). No routines touch X or Y during execution.
; Code generated by macros is relocatable; it contains no JMPs to itself.

zp_byte delay_temp_ ; only written to

; Delays n clocks, from 2 to 16777215
; Preserved: A, X, Y, flags
.macro delay n
    .if (n) < 0 .or (n) = 1 .or (n) > 16777215
	.error "Delay out of range"
    .endif
    delay_ (n)
.endmacro


; Delays n milliseconds (1/1000 second)
; n can range from 0 to 1100.
; Preserved: A, X, Y, flags
.macro delay_msec n
    .if (n) < 0 .or (n) > 1100
	.error "time out of range"
    .endif
    delay ((n)*CLOCK_RATE+500)/1000
.endmacro


; Delays n microseconds (1/1000000 second).
; n can range from 0 to 100000.
; Preserved: A, X, Y, flags
.macro delay_usec n
    .if (n) < 0 .or (n) > 100000
	.error "time out of range"
    .endif
    delay ((n)*((CLOCK_RATE+50)/100)+5000)/10000
.endmacro

.pushseg
.segment "DELAY_65536YXA"

;;;;;;;;;;;;;;;;;;;;;;;;
; Delays Y:X:A clocks+overhead
; Time: 65536*Y+256*X+A+overhead
;;;;;;;;;;;;;;;;;;;;;;;;
delay_65536y_256x_a_35_clocks:
	cpy #0				;+2
	beq delay_256x_a_30_clocks	;+3
	; do 65536 cycles. 4 done so far.
	dey		;2
	pha		;3
	 txa		;2
	 pha		;3
	  ; Total overhead: 27+30. Do (65536-27-30) cycles.
	  ldx #>(65536-27-30)	;2
	  lda #<(65536-27-30)	;2
	  jsr delay_256x_a_30_clocks
	 pla		;4
	 tax		;2
	pla		;4
	jmp delay_65536y_256x_a_35_clocks ;3
;;;;;;;;;;;;;;;;;;;;;;;;
; Delays A:X clocks+overhead
; Time: 256*A+X+34 clocks (including JSR)
; Written by Joel Yliluoma. Clobbers A. Preserves X,Y. Has relocations.
;;;;;;;;;;;;;;;;;;;;;;;;
:	; do 256 cycles.	; 5 cycles done so far. Loop is 2+1+ 2+3+ 1 = 9 bytes.
	sbc #1			; 2 cycles - Carry was set from cmp
	pha			; 3 cycles
	 lda #(256-25-10-2-4)   ; +2
	 jsr delay_a_25_clocks
	pla                     ; 4 cycles
delay_256a_x_33_clocks:
	cmp #1			; +2; 2 cycles overhead
	bcs :-			; +2; 4 cycles overhead
	; 0-255 cycles remain, overhead = 4
	txa 			; +2; 6; +27 = 33
        ; 15 + JSR + RTS overhead for the code below. JSR=6, RTS=6. 15+12=27
        ;          ;    Cycles        Accumulator     Carry flag
        ;          ; 0  1  2  3  4       (hex)        0 1 2 3 4
        sec        ; 0  0  0  0  0   00 01 02 03 04   1 1 1 1 1
:       sbc #5     ; 2  2  2  2  2   FB FC FD FE FF   0 0 0 0 0
        bcs :-     ; 4  4  4  4  4   FB FC FD FE FF   0 0 0 0 0
        lsr a      ; 6  6  6  6  6   7D 7E 7E 7F 7F   1 0 1 0 1
        bcc :+     ; 8  8  8  8  8   7D 7E 7E 7F 7F   1 0 1 0 1
:       sbc #$7E   ;10 11 10 11 10   FF FF 00 00 01   0 0 1 1 1
        bcc :+     ;12 13 12 13 12   FF FF 00 00 01   0 0 1 1 1
        beq :+     ;      14 15 14         00 00 01       1 1 1
        bne :+     ;            16               01           1
:       rts        ;15 16 17 18 19   (thanks to dclxvi for the algorithm) 

;;;;;;;;;;;;;;;;;;;;;;;;
; Delays X:A clocks+overhead
; Time: 256*X+A+30 clocks (including JSR)
; Written by Joel Yliluoma. Clobbers A,X. Preserves Y. Has relocations.
;;;;;;;;;;;;;;;;;;;;;;;;
delay_256x_a_30_clocks:
	cpx #0			; +2
	beq delay_a_25_clocks	; +3  (25+5 = 30 cycles overhead)
	; do 256 cycles.        ;  4 cycles so far. Loop is 1+1+ 2+3+ 1+3 = 11 bytes.
	dex                     ;  2 cycles
	pha                     ;  3 cycles
	 lda #(256-25-9-2-7)    ; +2
	 jsr delay_a_25_clocks
	pla                        ; 4
	jmp delay_256x_a_30_clocks ; 3.
;;;;;;;;;;;;;;;;;;;;;;;;
; Delays A clocks + overhead
; Preserved: X, Y
; Time: A+25 clocks (including JSR)  (13+6+6)
;;;;;;;;;;;;;;;;;;;;;;;;
:       sbc #7          ; carry set by CMP
delay_a_25_clocks:
	cmp #7		;2
	bcs :-          ;2    do multiples of 7
	;               ; Cycles          Accumulator            Carry           Zero
	lsr a           ; 0 0 0 0 0 0 0   00 01 02 03 04 05 06   0 0 0 0 0 0 0   ? ? ? ? ? ? ? 
	bcs :+          ; 2 2 2 2 2 2 2   00 00 01 01 02 02 03   0 1 0 1 0 1 0   1 1 0 0 0 0 0
:       beq @zero       ; 4 5 4 5 4 5 4   00 00 01 01 02 02 03   0 1 0 1 0 1 0   1 1 0 0 0 0 0
	lsr a           ; : : 6 7 6 7 6   :: :: 01 01 02 02 03   : : 0 1 0 1 0   : : 0 0 0 0 0 
	beq :+          ; : : 8 9 8 9 8   :: :: 00 00 01 01 01   : : 1 1 0 0 1   : : 1 1 0 0 0
	bcc :+          ; : : : : A B A   :: :: :: :: 01 01 01   : : : : 0 0 1   : : : : 0 0 0
@zero:  bne :+          ; 7 8 : : : : C   00 01 :: :: :: :: 01   0 1 : : : : 1   1 1 : : : : 0
:       rts             ; 9 A B C D E F   00 01 00 00 01 01 01   0 1 1 1 0 0 1   1 1 1 1 0 0 0 
; ^ (thanks to dclxvi for the algorithm)

.segment "DELAY_256"

; Delays A*256 clocks + overhead
; Preserved: X, Y
; Time: A*256+16 clocks (including JSR)
delay_256a_16_clocks:
	cmp #0
	bne :+
	rts
delay_256a_11_clocks_:
:       pha
	 lda #256-19-22
	 jsr delay_a_25_clocks
	pla
	clc
	adc #-1&$FF
	bne :-
	rts


.segment "DELAY_65536"
; Delays A*65536 clocks + overhead
; Preserved: X, Y
; Time: A*65536+16 clocks (including JSR)
delay_65536a_16_clocks:
	cmp #0
	bne :+
	rts
delay_65536a_11_clocks_:
:       pha
	lda #256-19-22-13
	jsr delay_a_25_clocks
	lda #255
	jsr delay_256a_11_clocks_
	pla
	clc
	adc #-1&$FF
	bne :-
	rts

max_short_delay = 41
	; delay_short_ macro jumps into these
	.res (max_short_delay-12)/2,$EA ; NOP
delay_unrolled_:
	rts
.popseg

;max_small_delay = 10
;.align $40
;	.res (max_small_delay-2), $C9  ; cmp #imm - 2 cycles
;	.byte $C5,$EA                  ; cmp zp   - 3 cycles
;delay_unrolled_small_:
;	rts

.macro delay_short_ n
    .if n < 0 .or n = 1 .or n > max_short_delay
	.error "Internal delay error"
    .endif
    .if n = 0
    	; nothing
    .elseif n = 2
	nop
    .elseif n = 3
	sta <delay_temp_
    .elseif n = 4
	nop
	nop
    .elseif n = 5
	sta <delay_temp_
	nop
    .elseif n = 6
	nop
	nop
	nop
    .elseif n = 7
	php
	plp
    .elseif n = 8
	nop
	nop
	nop
	nop
    .elseif n = 9
	php
	plp
	nop
    .elseif n = 10
	sta <delay_temp_
	php
	plp
    .elseif n = 11
	php
	plp
	nop
	nop
    .elseif n = 13
	php
	plp
	nop
	nop
	nop
    .elseif n & 1
	sta <delay_temp_
	jsr delay_unrolled_-((n-15)/2)
    .else
	jsr delay_unrolled_-((n-12)/2)
    .endif
.endmacro

.macro delay_nosave_ n
    ; 65536+17 = maximum delay using delay_256a_11_clocks_
    ; 255+27   = maximum delay using delay_a_25_clocks
    ; 27       = minimum delay using delay_a_25_clocks
    .if n > 65536+17
	lda #^(n - 15)
	jsr delay_65536a_11_clocks_
	; +2 ensures remaining clocks is never 1
	delay_nosave_ (((n - 15) & $FFFF) + 2)
    .elseif n > 255+27
	lda #>(n - 15)
	jsr delay_256a_11_clocks_
	; +2 ensures remaining clocks is never 1
	delay_nosave_ (<(n - 15) + 2)
    .elseif n >= 27
	lda #<(n - 27)
	jsr delay_a_25_clocks
    .else
	delay_short_ n
    .endif
.endmacro

.macro delay_ n
    .if n > max_short_delay
	php
	pha
	delay_nosave_ (n - 14)
	pla
	plp
    .else
	delay_short_ n
    .endif
.endmacro


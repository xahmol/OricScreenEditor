; ====================================================================================
; oric_core_assembly.s
; Core assembly routines for oric_core.c
;
; Credits for code and inspiration:
; - 6502.org: Practical Memory Move Routines
;   http://6502.org/source/general/memory_move.html
;
; =====================================================================================

; Exports

	.export		_DOSERROR
	.export		_ORIC_HChar_core
	.export		_ORIC_VChar_core
	.export		_ORIC_FillArea_core
	.export		_ORIC_CopyViewPort_core
	.export		_ORIC_Scroll_right_core
	.export		_ORIC_Scroll_left_core
	.export		_ORIC_Scroll_down_core
	.export		_ORIC_Scroll_up_core
    .export     _ORIC_RestoreStandardCharset
    .export     _ORIC_RestoreAlternateCharset
	.export		_ORIC_addrh
	.export		_ORIC_addrl
	.export		_ORIC_desth
	.export		_ORIC_destl
	.export 	_ORIC_strideh
	.export		_ORIC_stridel
	.export		_ORIC_value
	.export		_ORIC_tmp1
	.export		_ORIC_tmp2
    .export		_ORIC_tmp3
	.export		_ORIC_tmp4

; ======================================================================
; Defines of system addresses
; ======================================================================

DOSROM				=	$04F2			; Call to function to disable/enable SEDORIC RAM
_DOSERROR			=	$04FD			; Disk error address
SED_PRINTCHAR		=	$D20F			; SEDORIC print character patch address
SED_XROM			=	$D5D8			; SEDORIC default jump address before patch
ROM_ALTCHARS		=	$F816			; Kernal call to ROM generate alt chars
ROM_COPYROUTINE		=	$F982			; Kernal call to ROM copy routine		

; ======================================================================
; Zero page reservations
; ======================================================================

.segment    "ZEROPAGE"

ZP1:
    .res    1
ZP2:
    .res    1
ZP3:
    .res    1
ZP4:
    .res    1

.segment	"CODE"

; ======================================================================
; Data
; ======================================================================

_ORIC_addrh:
	.res	1
_ORIC_addrl:
	.res	1
_ORIC_desth:
	.res	1
_ORIC_destl:
	.res	1
_ORIC_strideh:
	.res	1
_ORIC_stridel:
	.res	1
_ORIC_value:
	.res	1
_ORIC_tmp1:
	.res	1
_ORIC_tmp2:
	.res	1
_ORIC_tmp3:
	.res	1
_ORIC_tmp4:
	.res	1

; Screen and scrolling core routines

; ------------------------------------------------------------------------------------------
_ORIC_HChar_core:
; Function to draw horizontal line with given character (draws from left to right)
; Input:	ORIC_addrh = high byte of start address in screen memory
;			ORIC_addrl = low byte of start address in screen memory
;			ORIC_tmp1 = character value
;			ORIC_tmp2 = length value
; ------------------------------------------------------------------------------------------

	; Hi-byte of the destination address to ZP1
	lda _ORIC_addrl	        			; Load high byte of start in A
	sta ZP1								; Write to ZP1

	; Lo-byte of the destination address to ZP2
	lda _ORIC_addrh		        		; Load high byte of start in A
	sta ZP2								; Write to ZP2

	; Initialise paint horizontal line
	ldy _ORIC_tmp2						; Set Y counter at number of chars
	dey									; Decrease counter
    lda _ORIC_tmp1                      ; Set charachter value

	; Paint loop
loophchar1:
	sta (ZP1),Y							; Store character at indexed address
	dey									; Decrease counter
	cpy #$ff							; Check for last char
	bne loophchar1						; Loop until last char
    rts

; ------------------------------------------------------------------------------------------
_ORIC_VChar_core:
; Function to draw vertical line with given character (draws from top to bottom)
; Input:	ORIC_addrh = high byte of start address
;			ORIC_addrl = low byte of start address
;			ORIC_tmp1 = character value
;			ORIC_tmp2 = length value
; ------------------------------------------------------------------------------------------

loopvchar:
	; Hi-byte of the destination address to ZP1
	lda _ORIC_addrl	        			; Load high byte of start in A
	sta ZP1								; Write to ZP1

	; Lo-byte of the destination address to ZP2
	lda _ORIC_addrh		        		; Load high byte of start in A
	sta ZP2								; Write to ZP2

	; Initialise paint vertical line
	ldy #$00							; Set Y index at 0
	lda _ORIC_tmp1						; Set screencode
	sta (ZP1),y							; Store character at indexed address

	; Increase start address with 40 for next line
	clc 								; Clear carry
	lda _ORIC_addrl	        			; Load low byte of address to A
	adc #$28    						; Add 40 with carry
	sta _ORIC_addrl			        	; Store result back
	lda _ORIC_addrh	        			; Load high byte of address to A
	adc #$00    						; Add 0 with carry
	sta _ORIC_addrh	        			; Store result back

	; Loop until length reaches zero
	dec _ORIC_tmp2		        		; Decrease length counter
	bne loopvchar		        		; Loop if not zero
    rts

; ------------------------------------------------------------------------------------------
_ORIC_FillArea_core:
; Function to draw area with given character (draws from topleft to bottomright)
; Input:	ORIC_addrh = high byte of start address
;			ORIC_addrl = low byte of start address
;			ORIC_tmp1 = character value
;			ORIC_tmp2 = length value
;			ORIC_tmp4 = number of lines
; ------------------------------------------------------------------------------------------

loopdrawline:
	jsr _ORIC_HChar_core				; Draw line

	; Increase start address with 40 for next line
	clc 								; Clear carry
	lda _ORIC_addrl	        			; Load low byte of address to A
	adc #$28    						; Add 40 with carry
	sta _ORIC_addrl			        	; Store result back
	lda _ORIC_addrh	        			; Load high byte of address to A
	adc #$00    						; Add 0 with carry
	sta _ORIC_addrh	        			; Store result back

	; Decrease line counter and loop until zero
	dec _ORIC_tmp4						; Decrease line counter
	bne loopdrawline					; Continue until counter is zero
	rts

; ------------------------------------------------------------------------------------------
_ORIC_CopyViewPort_core:
; Function to copy viewport from screen memory map to visible screen
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
;			ORIC_desth = high byte of destination address
;			ORIC_destl = low byte of destination address
;			ORIC_strideh = high byte of characters per line in source
;			ORIC_stridel = low byte of characters per line in source
;			ORIC_tmp1 = number lines to copy
;			ORIC_tmp2 = length per line to copy
; ------------------------------------------------------------------------------------------

	; Set source address pointer in zero-page
	lda _ORIC_addrl						; Obtain low byte in A
	sta ZP1								; Store low byte in pointer
	lda _ORIC_addrh						; Obtain high byte in A
	sta ZP2								; Store high byte in pointer

	; Set destination address pointer in zero-page
	lda _ORIC_destl						; Obtain low byte in A
	sta ZP3								; Store low byte in pointer
	lda _ORIC_desth						; Obtain high byte in A
	sta ZP4								; Store high byte in pointer

	; Start of copy loop
outerloopvp:							; Start of outer loop
	ldy _ORIC_tmp2						; Load length of line
	dey									; Decrease counter

	; Read value and store at address
copyloopvp:								; Start of copy loop
	lda (ZP1),Y							; Load source data
	sta (ZP3),Y    						; Save at destination

	; Decrese line counter
	dey									; Decrease line counter
	cpy #$ff							; Check for last character
	bne copyloopvp						; Continue loop if not yet last char

	; Add stride to addresses for next line
	clc									; Clear carry
	lda	ZP1								; Load low byte of source address
	adc _ORIC_stridel					; Add low byte of stride
	sta ZP1								; Store low byte of source
	lda ZP2								; Load high byte of source address
	adc _ORIC_strideh					; Add high byte of stride
	sta ZP2								; Store high byte of source address
	clc									; Clear carry
	lda ZP3								; Load low byte of destination
	adc #$28							; Add 40 characters for next line
	sta ZP3								; Store low byte of destination
	lda ZP4								; Load high byte of destination
	adc #$00							; Add 0 with carry
	sta ZP4								; Store high byte of destination
	dec _ORIC_tmp1						; Decrease counter number of lines
	bne outerloopvp						; Continue outer loop if not yet below zero
    rts


; ------------------------------------------------------------------------------------------
Increase_one_line:
; Increase source pointers by one line
; Input and output: original resp. new address pointers in ZP1/ZP2
; ------------------------------------------------------------------------------------------

	; Increase source addresses by one lines
	clc									; Clear carry
	lda ZP1								; Load color low byte
	adc #$28							; Add 40 with carry for one line
	sta ZP1								; Store back
	lda ZP2								; Load color high byte
	adc #$00							; Add zero with carry
	sta ZP2								; Store back
	clc									; Clear carry
	lda ZP3								; Load screen low byte
	adc #$28							; Add 40 with carry for one line
	sta ZP3								; Store back
	lda ZP4								; Load screen high byte
	adc #$00							; Add zero with carry
	sta ZP4								; Store back
	rts

; ------------------------------------------------------------------------------------------
Decrease_one_line:
; Decrease source pointers by one line
; Input and output: original resp. new address pointers in ZP1/ZP2
; ------------------------------------------------------------------------------------------

	; Decrease source addresses by one lines
	sec									; Set carry
	lda ZP1								; Load color low byte
	sbc #$28							; Subtract 40 with carry for one line
	sta ZP1								; Store back
	lda ZP2								; Load color high byte
	sbc #$00							; Subtract zero with carry
	sta ZP2								; Store back
	sec									; Clear carry
	lda ZP3								; Load screen low byte
	sbc #$28							; Subtract 40 with carry for one line
	sta ZP3								; Store back
	lda ZP4								; Load screen high byte
	sbc #$00							; Subtract zero with carry
	sta ZP4								; Store back
	rts

; ------------------------------------------------------------------------------------------
Store_address_pointers:
; Store the address pointers of the X,Y start location
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
; Output:	Address pointer in ZP1/ZP2
; ------------------------------------------------------------------------------------------

	lda _ORIC_addrl						; Load low byte of memory address
	sta ZP1								; Store in ZP1
	lda _ORIC_addrh						; Load high byte of memory address
	sta ZP2								; Store in ZP2
	rts

; ------------------------------------------------------------------------------------------
_ORIC_Scroll_right_core:
; Function to scroll text screen 1 charachter to the right, no fill
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
;			ORIC_tmp1 = number of lines to copy
;			ORIC_tmp2 = length per line to copy			
; ------------------------------------------------------------------------------------------

	; Set source address pointers
	jsr	Store_address_pointers			; Set source address pointers

	; Move one line right
loop_sr_outer:
	ldy _ORIC_tmp2						; Set Y index for width
	dey									; Decrease by 1 for zero base X coord
	dey

loop_sr_inner:
	lda (ZP1),y							; Load byte
	iny									; Increase index
	sta (ZP1),Y							; Save byte at adress plus 1
	dey									; Decrease index again
	dey									; Decrease index again
	cpy #$ff							; Check for last char
	bne loop_sr_inner					; Loop until index is at 0

	; Increase source addresses by one line
	jsr Increase_one_line				; Get next line

	; Decrease line counter
	dec _ORIC_tmp1						; Decrease line counter
	bne loop_sr_outer					; Loop until counter is zero
	rts

; ------------------------------------------------------------------------------------------
_ORIC_Scroll_left_core:
; Function to scroll text screen 1 charachter to the left, no fill
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
;			ORIC_tmp1 = number of lines to copy
;			ORIC_tmp2 = length per line to copy	
; ------------------------------------------------------------------------------------------

	; Set source address pointers
	jsr	Store_address_pointers			; Set source address pointers

	; Move one line left
loop_sl_outer:
	ldy #$01							; Start with index character 1
loop_sl_inner:
	lda (ZP1),y							; Load byte
	dey									; Decrease index
	sta (ZP1),Y							; Save byte at adress minus 1
	iny									; Increase again
	iny									; Increase again
	cpy _ORIC_tmp2						; Compare with width to check for last char in line
	bne loop_sl_inner					; Loop until index is at last char

	; Increase source addresses by one line
	jsr Increase_one_line				; Get next line

	; Decrease line counter
	dec _ORIC_tmp1						; Decrease line counter
	bne loop_sl_outer					; Loop until counter is zero
	rts

; ------------------------------------------------------------------------------------------
Copy_line:
; Function to copy one line from source to destination for scroll up and down
; Input:	ZP1/ZP2 for source pointer, ZP3/ZP4 for destination pointer
;			ORIC_tmp2 = length per line to copy
; ------------------------------------------------------------------------------------------

	ldy _ORIC_tmp2						; Set Y index for width
	dey									; Decrease by 1 for zero base X coord

loop_cl_inner:
	lda (ZP1),Y							; Load byte from source
	sta (ZP3),Y							; Save byte at destination
	dey									; Decrease index again
	cpy #$ff							; Check for last char
	bne loop_cl_inner					; Loop until index is at 0
	rts

; ------------------------------------------------------------------------------------------
_ORIC_Scroll_down_core:
; Function to scroll text screen 1 charachter down, no fill
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
;			ORIC_tmp1 = number of lines to copy
;			ORIC_tmp2 = length per line to copy
; ------------------------------------------------------------------------------------------
	
	; Set source address pointers
	jsr	Store_address_pointers			; Set source address pointers
    clc                                 ; Clear carry
	lda ZP1								; Load low byte of address
	adc #$28							; Add 40 for next line
	sta ZP3								; Store in ZP3 for destination pointer
	lda ZP2								; Load high byte of address
	adc #$00							; Add carry
	sta ZP4								; Store in ZP4

	; Move one line down
loop_sd:
	jsr Copy_line						; Copy line from source to destination

	; Increase source addresses by one lines
	jsr Decrease_one_line				; Get next line

	; Decrease line counter
	dec _ORIC_tmp1						; Decrease line counter
	bne loop_sd     					; Loop until counter is zero

	rts

; ------------------------------------------------------------------------------------------
_ORIC_Scroll_up_core:
; Function to scroll text screen 1 charachter up, no fill
; Input:	ORIC_addrh = high byte of source address
;			ORIC_addrl = low byte of source address
;			ORIC_tmp1 = number of lines to copy
;			ORIC_tmp2 = length per line to copy
; ------------------------------------------------------------------------------------------
	
	; Set source address pointers color memory
    jsr	Store_address_pointers			; Set source address pointers
	sec									; Clear carry
	lda ZP1								; Load low byte of color memory address
	sbc #$28							; Subtract 40 for next line
	sta ZP3								; Store in ZP3 for destination pointer
	lda ZP2								; Load high byte of color memory address
	sbc #$00							; Subtract carry
	sta ZP4								; Store in ZP4

	; Move one line up
loop_su:
	jsr Copy_line						; Copy line from source to destination

	; Increase source addresses by one lines
	jsr Increase_one_line				; Get next line

	; Decrease line counter
	dec _ORIC_tmp1						; Decrease line counter
	bne loop_su     					; Loop until counter is zero

	rts

; ------------------------------------------------------------------------------------------
_ORIC_RestoreStandardCharset:
; Function to restore ORIC standard charset
; Copy from ORIC Atmos rom RESET routine at $F8D0
; Reference: Oric Advanced User Guide - Leycester Whewell
; https://library.defence-force.org/books/content/oric_advanced_user_guide.pdf
; ------------------------------------------------------------------------------------------

    ldx #$05                            ; Set up right value for lookup table
    jsr ROM_COPYROUTINE                 ; Jump to ROM copy routine
    rts

; ------------------------------------------------------------------------------------------
_ORIC_RestoreAlternateCharset:
; Function to restore ORIC alternate charset
; Call to ORIC Atmos rom Generate alt. char set routine at $F816
; Reference: Oric Advanced User Guide - Leycester Whewell
; https://library.defence-force.org/books/content/oric_advanced_user_guide.pdf
; ------------------------------------------------------------------------------------------

    jsr ROM_ALTCHARS                    ; Jump to ROM routine
    rts

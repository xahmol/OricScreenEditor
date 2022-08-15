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
	.export		_ORIC_DIRParse_start_core
	.export		_ORIC_DIRParse_end
	.export		_ORIC_DIRParse_Xpos
	.export		_ORIC_DIRParse_Ypos
	.export		_ORIC_DIRParse_Ymax
	.export		_ORIC_DIRParse_Xmax
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
_ORIC_DIRParse_Xpos:
	.res	1
_ORIC_DIRParse_Ypos:
	.res	1
_ORIC_DIRParse_Ymax:
	.res	1
_ORIC_DIRParse_Xmax:
	.res	1
DIRParse_Phase:
	.res	1
DIRParse_Count:
	.res	1
DIRParse_PlotChar:
	.res	1
DIRParse_Xnumber:
	.res	1
DIRParse_Xcoord:
	.res	1
DIRParse_Ycoord:
	.res	1

; Disk directory parser routines

; ------------------------------------------------------------------------------------------
_ORIC_DIRParse_start_core:
; Function to enable directory parser and reroute screen output while DIR is executed
; Input:	ORIC_DIRParse_Xpos, ORIC_DIRParse_Ypos: Start co-ordinates
;			ORIC_DIRParse_Ymax: Last Y line to plot
;			ORIC_DIRParse_Xmax: Number of entries per line
; Output:	ORIC_DIRParse_Diskname: Name of disk parsed
;			Output of dir contents to screen at given co-ordinates
; ------------------------------------------------------------------------------------------

	; Enable overlay RAM
	jsr	DOSROM							; Call function to toggle overlay RAM

	; Patch SEDORIC print character function to own routine
	lda #<ORIC_DIRParse_process			; Get low byte of parsing function
	sta SED_PRINTCHAR					; Store at patch address
	lda #>ORIC_DIRParse_process			; Get high byte of parsing function
	sta SED_PRINTCHAR+1					; Store at patch address
	lda #$60							; Load $60 for oppcode RTS
	sta SED_PRINTCHAR+2					; Store RTS at patch location

	; Disable overlay RAM
	jsr	DOSROM							; Call function to toggle overlay RAM

	; Init variables
	lda #$00							; Load 0 for first phase
	sta DIRParse_Phase					; Store in variable
	lda #$28							; Skip 40 bytes in first phase to get to disk name
	sta DIRParse_Count					; Store as counter
	lda #$00							; Load 0 for first entry on line
	sta DIRParse_Xnumber				; Store variable
	lda _ORIC_DIRParse_Xpos				; Load X coord start value
	sta DIRParse_Xcoord					; Store in variable
	lda _ORIC_DIRParse_Ypos				; Load X coord start value
	sta DIRParse_Ycoord					; Store in variable

	rts

; ------------------------------------------------------------------------------------------
DIRParse_nextX:
; Function to increase X and Y co-ords while parsing
; ------------------------------------------------------------------------------------------

	; Increase counter of entries per line
	inc DIRParse_Xnumber				; Increase number
	lda DIRParse_Xnumber				; Load in A
	cmp _ORIC_DIRParse_Xmax				; Compare with maxium number
	bne DIRParse_Incx					; Branch to increase X
	
	; Increase X co-ord by 10 for next entry
	clc									; Clear carry
	lda DIRParse_Xcoord					; Load present xcoord
	adc #$0a							; Add 10 for next entry
	sta DIRParse_Xcoord					; Store again
	rts

DIRParse_Incx:
	; Increase Y co-ord by 1 until max line
	inc DIRParse_Ycoord					; Increase y-coord
	lda DIRParse_Ycoord					; Load Y-coord
	cmp _ORIC_DIRParse_Ymax				; Compare with maximum line
	beq DIRParse_lastlinereached		; Branch to last line reached if equal

	; Reset X values
	lda #$00							; Load 0 for X counter
	sta DIRParse_Xnumber				; Store in variable
	lda _ORIC_DIRParse_Xpos				; Load start X coord
	sta DIRParse_Xcoord					; Store in variable
	rts

DIRParse_lastlinereached:
	lda #$09							; Load 9 for dirparse phase 9
	sta DIRParse_Phase					; Store as variable
	rts

; ------------------------------------------------------------------------------------------
DIRParse_Stormem:
; ------------------------------------------------------------------------------------------

	lda DIRParse_PlotChar

dirparsestormem:
	sta $9000

	inc dirparsestormem+1
	bne dirparsestormemend
	inc dirparsestormem+2

dirparsestormemend:
	rts

; ------------------------------------------------------------------------------------------
_ORIC_DIRParse_end:
; Function to disable directory parser and reroute screen output while DIR is executed
; Input:	
; ------------------------------------------------------------------------------------------

	; Enable overlay RAM
	jsr	DOSROM							; Call function to toggle overlay RAM

	; Patch SEDORIC print character function back to original
	lda #<SED_XROM						; Get low byte of original jump address
	sta SED_PRINTCHAR					; Store at patch address
	lda #>SED_XROM						; Get high byte of original jump address
	sta SED_PRINTCHAR+1					; Store at patch address
	lda #$12							; Load original value
	sta SED_PRINTCHAR+2					; Store at patch location

	; Disable overlay RAM
	jsr	DOSROM							; Call function to toggle overlay RAM

	rts

; ------------------------------------------------------------------------------------------
ORIC_DIRParse_process:
; Function to intercept the system char to screen routine
; Input:	Parses inout character stream via the X register
; ------------------------------------------------------------------------------------------

	; Initialise
	sta DIRParse_PlotChar				; Store present char in variable
	pha									; Save char to be printed
	jsr DIRParse_Stormem
	lda DIRParse_Phase					; Check phase

	; Check for phase 0: skip first header and disk name
	cmp #$00							; Compare for phase 0
	bne DP_phase1						; Go to next check
	jmp DP_skipheader					; Branch to skip header if phase 0

DP_phase1:
	; Check for phase 2: print filename left column
	cmp #$01							; Compare for phase 2
	bne DP_phase2						; Go to next check
	jmp	DP_printleftcolumn				; Branch to read disk name if phase 2

DP_phase2:
	; Check for phase 3: skip end filename left column
	cmp #$02							; Compare for phase 3
	bne DP_phase3						; Go to next check
	jmp DP_skipendleftcolumn			; Branch to read disk name if phase 3
	
DP_phase3:
	; Check for phase 4: skip to right column
	cmp #$03							; Compare for phase 4
	bne DP_phase4						; Go to next check
	jmp DP_skiptoright					; Branch to read disk name if phase 5

DP_phase4:
	; Check for phase 5: print filename right column
	cmp #$04							; Compare for phase 5
	bne DP_phase9						; Go to next check
	jmp DP_printleftcolumn				; Branch to read disk name if phase 5

DP_phase9:
	; Default should be phase 9: end reached, ignore rest
	jmp DP_end							; Jumo to end

DP_endofdir:
	lda #$09							; Set phase as 9 for end reached
	sta DIRParse_Phase					; Set as variable

DP_end:
	pla									; Restore char to be printed

	; Jump to XROM routine to return to SEDORIC gracefully
	jsr SED_XROM						; Jump to XROM routine
	.byte $10,$cd,$10,$cd				; Data for XROM

	rts

DP_skipheader:							; Phase 0: skip header
	dec DIRParse_Count					; Decrease counter
	bne DP_end							; If not yet 0, go to end
	inc DIRParse_Phase					; Next phase
	lda #$09							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jmp DP_end							; Jump to end

DP_printleftcolumn:						; Phase 1: Print a left column filename
	lda DIRParse_PlotChar				; Load char to be printed
	cmp #$2A							; Compare if * for end of dir list
	beq	DP_endofdir						; Branch if last file has been passed
	jsr ORIC_Plot						; Plot character
	dec DIRParse_Count					; Decrease counter
	bne	DP_end							; If not yet 0, go to end
	inc DIRParse_Phase					; Next phase
	lda #$09							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jsr DIRParse_nextX					; Update co-ordinates
	jmp DP_end							; Jump to end

DP_skipendleftcolumn:					; Phase 2: skip end filename left column
	dec DIRParse_Count					; Decrease counter
	bne DP_end							; If not yet 0, go to end
	inc DIRParse_Phase					; Next phase
	lda #$02							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jmp DP_end							; Jump to end

DP_skiptoright:							; Phase 3: skip to right column
	lda DIRParse_PlotChar				; Load char to be printed
	cmp #$2A							; Compare if * for end of dir list
	beq	DP_endofdir						; Branch if last file has been passed
	dec DIRParse_Count					; Decrease counter
	bne DP_end							; If not yet 0, go to end
	inc DIRParse_Phase					; Next phase
	lda #$09							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jmp DP_end							; Jump to end

DP_printrightcolumn:					; Phase 4: Print a right column filename
	lda DIRParse_PlotChar				; Load char to be printed
	cmp #$20							; Compare if right column is empty
	beq	DP_endofdir						; Branch if last file has been passed
	jsr ORIC_Plot						; Plot character
	dec DIRParse_Count					; Decrease counter
	bne	DP_end							; If not yet 0, go to end
	lda #$02							; Set phase 2 again
	sta DIRParse_Phase					; Store in variable
	lda #$09							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jsr DIRParse_nextX					; Update co-ordinates
	jmp DP_end							; Jump to end

DP_skiptoleft:							; Phase 5: skip to left column again
	lda DIRParse_PlotChar				; Load char to be printed
	cmp #$2A							; Compare if * for end of dir list
	beq	DP_endofdir						; Branch if last file has been passed
	dec DIRParse_Count					; Decrease counter
	bne DP_end							; If not yet 0, go to end
	inc DIRParse_Phase					; Next phase
	lda #$09							; Set lenghth
	sta DIRParse_Count					; Set new counter
	jmp DP_end							; Jump to end

; Screen and scrolling core routines

; ------------------------------------------------------------------------------------------
ORIC_Plot:
; Function to plot a char at X resp Y coord
; Input:	A = char to plot
;			_ORIC_DIRParse_Xpos, _ORIC_DIRParse_Ypos = Co-ordinate to plot at
; ------------------------------------------------------------------------------------------

	; Set address at begin of Y line
	ldy DIRParse_Ycoord					; Load Y co-oord as index
	lda OricScreenAddress_lb,Y			; Load corresponding low byte screen address
	sta ORIC_Plot_selfmodify+1			; Self modify address
	lda OricScreenAddress_hb,Y			; Load corresponding high byte screen address
	sta ORIC_Plot_selfmodify+2			; Self modify address

	; Add X coord
	clc									; Clear carry
	lda ORIC_Plot_selfmodify+1			; Load screen address low byte begin line
	adc DIRParse_Xcoord					; Add X co-ord with carry to low byte
	sta ORIC_Plot_selfmodify+1			; Store new value
	lda ORIC_Plot_selfmodify+2			; Load screen address high byte begin line
	adc #$00							; Add zero with carry
	sta ORIC_Plot_selfmodify+2			; Store new value

	; Plot char
	lda DIRParse_PlotChar				; Load char to plot
ORIC_Plot_selfmodify:
	sta $bb80							; Store at calculated self modifying address
	rts

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

; Table for beginning of line screen addresses

OricScreenAddress_lb:
	.byte $80
	.byte $A8
	.byte $D0
	.byte $F8
	.byte $20
	.byte $48
	.byte $70
	.byte $98
	.byte $C0
	.byte $E8
	.byte $10
	.byte $38
	.byte $60
	.byte $88
	.byte $B0
	.byte $D8
	.byte $00
	.byte $28
	.byte $50
	.byte $78
	.byte $A0
	.byte $C8
	.byte $F0
	.byte $18
	.byte $40
	.byte $68
	.byte $90
	.byte $B8

OricScreenAddress_hb:
	.byte $BB
	.byte $BB
	.byte $BB
	.byte $BB
	.byte $BC
	.byte $BC
	.byte $BC
	.byte $BC
	.byte $BC
	.byte $BC
	.byte $BD
	.byte $BD
	.byte $BD
	.byte $BD
	.byte $BD
	.byte $BD
	.byte $BE
	.byte $BE
	.byte $BE
	.byte $BE
	.byte $BE
	.byte $BE
	.byte $BE
	.byte $BF
	.byte $BF
	.byte $BF
	.byte $BF
	.byte $BF

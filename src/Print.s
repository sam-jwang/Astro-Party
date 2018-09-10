; File: Print.s
; Name: Brian Cheung and Sam Wang
; Date: 5/4/18
; Desc: This driver outputs decimal numbers or fixed point numbers
; Usage: Use functions to output numbers in specified fromat to the LCD

; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 
cnt	EQU 0
FP	RN 	11

    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

  

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutDec
	PUSH 	{R0, LR}
	SUB 	SP, #8				;allocation
	MOV 	FP, SP				;frame pointer
	MOV		R1, #0				;init cnt=0
	STR 	R1, [FP, #cnt]
loop	
	MOV		R1, #10				;isolate ones digit
	UDIV 	R2, R0, R1			;R2 = input/10
	MUL		R3, R2, R1
	SUB 	R3, R0, R3			;ASCII = R3 + offset
	ADD		R3, #48
	PUSH	{R3,LR}
	
	LDR		R0, [FP, #cnt]		;increment cnt
	ADD		R0, #1
	STR		R0, [FP, #cnt]
	
	MOV 	R0, R2				;if num!=0, loop
	CMP 	R0, #0
	BNE 	loop
	
print
	POP 	{R0, LR}
	BL 		ST7735_OutChar		;print char
	LDR		R0, [FP, #cnt]		;decrement cnt
	SUBS	R0, #1
	STR		R0, [FP, #cnt]
	BNE		print

	ADD		SP, #8
	POP 	{R0, LR}

    BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000 "
;       R0=3,    then output "0.003 "
;       R0=89,   then output "0.089 "
;       R0=123,  then output "0.123 "
;       R0=9999, then output "9.999 "
;       R0>9999, then output "*.*** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix
	PUSH 	{R0, LR}
	SUB 	SP, #8				;allocation
	MOV 	FP, SP				;frame pointer
	MOV		R1, #0				;init cnt=0
	STR 	R1, [FP, #cnt]
	
	MOV 	R2, #9999
	SUBS	R2, R0, R2
	BHI 	tooBig
	
loop3	
	MOV		R1, #10				;isolate ones digit
	UDIV 	R2, R0, R1			;R2 = input/10
	MUL		R3, R2, R1
	SUB 	R3, R0, R3			;ASCII = R3 + offset
	ADD		R3, #48
	PUSH	{R3,LR}
	
	MOV 	R0, R2				
	LDR		R2, [FP, #cnt]		;increment cnt
	ADD		R2, #1
	STR		R2, [FP, #cnt]
	CMP 	R2, #3
	BNE		check0
	MOV		R1, #46				;push decimal point
	PUSH 	{R1, LR}
	ADD		R2, #1
	STR		R2, [FP, #cnt]

check0
	CMP 	R0, #0				;if num!=0, loop3
	BNE 	loop3
	CMP		R2, #5				;if cnt<5, loop3
	BLO 	loop3
	
print2
	POP 	{R0, LR}
	BL 		ST7735_OutChar		;print char
	LDR		R0, [FP, #cnt]		;decrement cnt
	SUBS	R0, #1
	STR		R0, [FP, #cnt]
	BNE		print2

	ADD		SP, #8
	POP 	{R0, LR}

    BX   LR

tooBig
	MOV		R0, #42
	PUSH 	{R0, LR}

	LDR		R1, [FP, #cnt]		;increment cnt
	ADD		R1, #1
	STR		R1, [FP, #cnt]
	CMP 	R1, #3
	BNE		checkDigits
	MOV		R0, #46				;push '.'
	PUSH 	{R0, LR}
	ADD		R1, #1
	STR		R1, [FP, #cnt]
checkDigits	
	CMP		R1, #5
	BLO		tooBig
	B 		print2
	
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file

; Print.s
; Student names: Alex Koo, Anthony Do
; Last modification date: 4/1
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; SSD1306_OutChar   outputs a single 8-bit ASCII character
; SSD1306_OutString outputs a null-terminated string 

    IMPORT   SSD1306_OutChar
    IMPORT   SSD1306_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix
    PRESERVE8
    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
N EQU 4
M EQU 8
CNT EQU 0
FP RN 11

LCD_OutDec

    PUSH{LR}

    ; Checks if R0 is zero, if not then call function again
    CMP R0, #10
    BLO OutDec_Output

    ADD SP, SP, #-M
    STR R0, [SP, #CNT]
    ;equivalent to PUSH{R0}

    ; R1 = 10 for division
    MOV R1, #10

    ; Divides R0 by 10
    UDIV R0, R1


    BL LCD_OutDec

    ;(1 / 10) % 10 = 0

    ADD SP, SP, #M
    LDR R0, [SP, #-M]
    ;POP{R0}

    ; Makes two copies of R0
    ; R2 gets divided, R3 remains same
    MOV R2, R0
    MOV R3, R0

    ; R1 = 10 for division
    MOV R1, #10

    ; Multiples R2 by 10
    UDIV R2, R1
    MUL R2, R1
    ; Subtracts R2 from R3, stores in R2
    SUB R2, R3, R2

    ; 655 = R2
    ; 655 / 10
    ; 65 = R3

    ; 65 * 10 -> R3 = 650
    ; R2 - R3 = 5

    ; After all function calls, print chars to screen
    MOV R0, R2
OutDec_Output
    ; Converts R0 to ASCII

    ;PUSH{R2}
    ADD R0, #0x30
    BL SSD1306_OutChar

    POP{LR}


      BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.01, range 0.00 to 9.99
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.00 "
;       R0=3,    then output "0.03 "
;       R0=89,   then output "0.89 "
;       R0=123,  then output "1.23 "
;       R0=999,  then output "9.99 "
;       R0>999,  then output "*.** "
; Invariables: This function must not permanently modify registers R4 to R11
LCD_OutFix


	; Allocation Phase
	PUSH{R11}
	STR R0, [SP, #0]
	SUB SP, #4
	MOV FP, SP
	PUSH{LR}
		
	; Store CNT as zero
	MOV R0, #0
	STR R0, [FP, #CNT]
	
	; Loads 10 into register for division
	MOV R1, #10

	; Checks if N is less than 1000
	LDR R0, [FP, #N]
	CMP R0, #1000
	BLO In_Range

	; If N > 1000 print *.**
	MOV R0, #0x2A
	BL SSD1306_OutChar
	MOV R0, #0x2E
	BL SSD1306_OutChar
	MOV R0, #0x2A
	BL SSD1306_OutChar
	MOV R0, #0x2A
	BL SSD1306_OutChar
	B OutFix_End
	
In_Range
	
	; Loads count and adds '1', stores it back
	LDR R0, [FP, #CNT]
	ADD R0, #1
	STR R0, [FP, #CNT]
	
	; Load two copies of 'input'
	LDR R2, [FP, #N]
	LDR R3, [FP, #N]
	
	; Gets the 1's place of the current input
	UDIV R2, R1
	; Stores the cut-off value back into 'N'
	STR R2, [FP, #N]
	MUL R2, R2, R1
	SUB R2, R3, R2
	
	; Stores value onto the stack
	PUSH{R2}
	
	; Check if CNT is less than 3
	LDR R2, [FP, #CNT]
	CMP R2, #3
	BLO In_Range
	
	
	; Pops top value into R2 and adds 0x30 to convert to ASCII
	POP{R2}
	AND R0, R0, #0
	ADD R0, R2
	ADD R0, #0x30
	; Outputs the current character to the screen
	BL SSD1306_OutChar
	
	; Prints a period to the screen
	MOV R0, #0x2E
	BL SSD1306_OutChar
	
	; Pops top value into R2 and adds 0x30 to convert to ASCII
	POP{R2}
	AND R0, R0, #0
	ADD R0, R2
	ADD R0, #0x30
	; Outputs the current character to the screen
	BL SSD1306_OutChar
	
	; Pops top value into R2 and adds 0x30 to convert to ASCII
	POP{R2}
	AND R0, R0, #0
	ADD R0, R2
	ADD R0, #0x30
	; Outputs the current character to the screen
	BL SSD1306_OutChar
	
OutFix_End

	POP{LR}
	ADD SP, #8
	POP{R11}

     BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN          ; make sure the end of this section is aligned
     END            ; end of file

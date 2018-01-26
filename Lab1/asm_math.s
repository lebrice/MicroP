	AREA text, CODE, READONLY
	EXPORT asm_math

; function asm_math
; inputs:
; -------
; R0: testValues pointer
; R1: size
; R3: asm_output pointer




asm_math

	MOV		R6, R0
	MOV		R7, R1



	BX		LR
	
	END
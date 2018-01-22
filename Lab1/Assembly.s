	AREA text, CODE, READONLY
	EXPORT Example_asm

; function Example_asm
; inputs:
; -------
; R0: input


Example_asm

	MOV		R6, R0							
	BX		LR	
	
END
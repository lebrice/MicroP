	AREA text, CODE, READONLY
	EXPORT asm_math

; function asm_math
; inputs:
; -------
; R0: testValues pointer
; R1: size
; R2: asm_output pointer

asm_math
	; set initial value of RMS, max, min, max_index and min_index.
	VLDR.f32 S0, =0  ; initial RMS value is 0.
	VLDR S1, [R0] ; Load in the first value as the initial max.
	VMOV.f32 S2, S1 ; Set the first value as the initial min. (take it from S1 which we just loaded.)
	MOV R3, #0 ; initial max index is 0.
	MOV R4, #0 ; initial min index is 0.
	
	MOV R5, #0 ; R5 is the counter of how many loop iterations we've done.

loop
	VLDM R0, { S5 } ; Read (potentially multiple) floating point values, starting at address in R0.
	; S5: the current latest value.
	
	VMLA.f32 S0, S5, S5 	; S0 = S0 + S5 * S5 (RMS calculation)
	
	ADD R5, R5, #1 ; increment the counter
	CMP R1, R5
	BNE loop
	
	

	BX		LR
	
	END
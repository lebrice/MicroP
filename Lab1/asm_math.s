	AREA text, CODE, READONLY
	EXPORT asm_math
		
; function asm_math
; inputs:
; -------
; R0: testValues pointer
; R1: size
; R2: asm_output pointer

asm_math
	
	; REGISTERS USED:
	; S0 : RMS value
	; S1 : MAX value
	; S2 : MIN value
	
	; R3 : MAX index
	; R4 : MIN index
	
	; S3 : Current value from the array (i.e. "S3 = array[i]")
	; R5 : Loop counter (goes from size to 0, decremented at each loop iteration)
	; S4 : float value of the 'size' (R1), used for the division at the end of the RMS calculation.
	
	; TODO: Since we use registers greater than R3, we need to save them onto the stack, and reset them after, in order not to lose data.
	
	
	
	
	
	; set initial values
	VLDR.f32 S0, =0  ; initial RMS value is 0.
	VLDR.f32 S1, [R0] ; Load in the first value, and set it as the initial max.
	VMOV.f32 S2, S1 ; Set initial min to be the first value in the array. (take it from S1 since we just loaded.)
	MOV R3, #0 ; initial max index is 0.
	MOV R4, #0 ; initial min index is 0.
	MOV R5, #0 ; Loop Counter variable, initiated with the 'size' value. We will decrement it at each iteration.
	
loop
	; S3 is the current latest value of the array.
	VLDR.f32 S3, [R0] ; Load the value pointed to by R0 into S3.
	
	; RMS calculation
	VMLA.f32 S0, S3, S3 	; S0 = S0 + S3 * S3
	
	; MAX calculation.
	VCMP.f32 S3, S1 	;Compare S3 (the latest value) and S1 (the current max) (" if(S3 > S1){ (...) }")
	VMOVGT.f32 S1, S3 	;Move S3 into S1 if the 'GT' (Greater-Than) condition is met, as set in the previous VCMP operation
	VMRSGT APSR_nzcv, FPSCR ; move the conditions from the floating-point unit to the core so we can use conditional operations on non-vector instructions.
	MOVGT R3, R5
	
	; MIN Calculation
	VCMP.f32 S3, S2 	;Compare S3 (the latest value) and S1 (the current max) (" if(S3 > S1){ (...) }")
	VMOVLT.f32 S2, S3 	;Move S3 into S1 if the 'GT' (Greater-Than) condition is met, as set in the previous VCMP operation
	VMRSLT APSR_nzcv, FPSCR ; move the conditions from the floating-point unit to the core so we can use conditional operations on non-vector instructions.
	MOVLT R4, R5
		
	; Increment the array pointer.
	ADD R0, R0, #4
	; Increment the loop counter	
	ADD R5, R5, #1
	
	; Check exit condition.
	CMP R5, R1
	BNE loop
	
	; Divide RMS by the size (as a float), then take its square root.
	
	;VCVT.f32.s32 S4, R1		;put  a float copy of 'size'
	;VDIV.f32 S0, S0, S4 ; (RMS = RMS / float(SIZE)
	VSQRT.f32 S0, S0
	
	; Save all the required values.
	VSTM R2!, {S0, S1, S2}
	STM R2, {R3, R4}
	
	BX		LR
	END
#include <stdio.h>
#include <math.h>
#include "arm_math.h"

int Example_asm(int Input);

void C_math(float inputValues[], int size, float results[]);

float testVals[5] = {5.0, 2.0, 5.0, 7.0, 6.0};

float OutputArray[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex



int main()
{
	int Input = 10;
	printf("Begins Asm\n");
	
	Example_asm(Input);
	C_math(testVals, 5, OutputArray);
	printf("The end!\n");
	
	printf("RMS: %f\n", OutputArray[0]);
	printf("Max Value: %f", OutputArray[1]);
	printf("  at index: %f\n", OutputArray[3]);
	printf("Min Value: %f", OutputArray[2]);
	printf("  at index: %f\n", OutputArray[4]);
	return 0;
}

void C_math(float inputValues[], int size, float results[]){
		int i = 0;
		
		OutputArray[1] = inputValues[0]; //Max val
		OutputArray[2] = inputValues[0]; //Min val
		
		
		
		for(; i < size; i++){
			OutputArray[0] += pow(inputValues[i],2);
			
			if (inputValues[i] > OutputArray[1]){
				OutputArray[1]  = inputValues[i];
				OutputArray[3] = i;
			}				
			if (inputValues[i] < OutputArray[2]){
				OutputArray[2]  = inputValues[i];
				OutputArray[4] = i;
			}
		}
		OutputArray[0] /= size;
		OutputArray[0] = sqrt(OutputArray[0]);

	
}

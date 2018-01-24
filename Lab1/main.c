#include <stdio.h>
#include <math.h>
#include "arm_math.h"


static const int INPUT_SIZE = 10;

//function declarations
int Example_asm(int Input);
void C_math(float inputValues[], int size, float results[]);
void CMSIS_math(float inputValues[], int size, float results[]);
void FIR_C(int Input, float* Output);


// Array of weights
float weights[5] = {0.1, 0.15, 0.5, 0.15, 0.1};
// Buffer that will hold the values as they come in.
int buffer[5];

int head, tail = 0;

float input_array[INPUT_SIZE] = {
	1,1,1,1,1,1,1,1,1,1
};
float filtered_input[INPUT_SIZE];




float testVals[5] = {5.0, 2.0, 5.0, 7.0, 6.0};


float C_mathOutput[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex
float32_t CMSIS_Output[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex


int main()
{
	int i, j;
	int Input = 10;
	float* placer = &filtered_input[0];
	printf("Begins Asm\n");	
	Example_asm(Input);
	
	
	
	// Print out the input array;
	printf("Input Array: [");
	for(i=0; i<10; i++){
		printf("%f,", input_array[i]);
	}
	printf("]\n");
	
	// Print out the filtered array;
	printf("Filtered Array: [");
	for(j=0; j< 10; j++){
		FIR_C(input_array[j],placer++);
		printf("%f,",filtered_input[j]);
	}
	printf("]\n");
	
	printf("The end!\n");
	
	
	C_math(testVals, 5, C_mathOutput);
	CMSIS_math(testVals, 5, CMSIS_Output);
	
	printf("RMS: %f\n", C_mathOutput[0]);
	printf("Max Value: %f", C_mathOutput[1]);
	printf("  at index: %f\n", C_mathOutput[3]);
	printf("Min Value: %f", C_mathOutput[2]);
	printf("  at index: %f\n", C_mathOutput[4]);
	return 0;
}

void CMSIS_math(float inputValues[], int size, float results[]){
	float RMS;
	float maxVal; //Max val
	uint32_t maxIndex;
	float minVal; //Min val
	uint32_t minIndex;
	
	arm_max_f32(inputValues, size, &maxVal, &maxIndex);
	
	printf("CMSIS MAXval: %f\n", maxVal);
	
}
void C_math(float inputValues[], int size, float results[]){
		int i = 0;
		float RMS;
		float maxVal = inputValues[0]; //Max val
		float maxIndex;
		float minVal = inputValues[0]; //Min val
		float minIndex;
		
		
		for(; i < size; i++){
			RMS += pow(inputValues[i],2);
			
			if (inputValues[i] > maxVal){
				maxVal  = inputValues[i];
				maxIndex = i;
			}				
			if (inputValues[i] < minVal){
				minVal  = inputValues[i];
				minIndex = i;
			}
		}
		RMS /= size;
		C_mathOutput[0] = sqrt(RMS);
		C_mathOutput[1] = maxVal;
		C_mathOutput[2] = minVal;
		C_mathOutput[3] = maxIndex;
		C_mathOutput[4] = minIndex;
	
}
	
	

void FIR_C(int Input, float* Output){
	/**
	Idea: use the buffer like a circular queue.
	- Add the element to the buffer, using the head pointer.
	- Update tail accordingly
	- iterate in the buffer, going from head to tail, and add up the results
	*/
	int i;
	float result = 0.f;
	head = (head + 1) % 5; // Update the head.
	if(head == tail){ // Update the tail, if necessary.
		tail = (tail + 1) % 5; 
	}
	buffer[head] = Input; // write the new value in.
	for(i=0; i<5; i++){
		// move backward from 'head' to 'tail', adding up the values.
		result += weights[i] * buffer[(head - i + 5) % 5];
	}
	*Output = result; // place the result at the given location.
}
	

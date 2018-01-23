#include <stdio.h>
#include <math.h>
#include "arm_math.h"

static const int INPUT_SIZE = 10;

int Example_asm(int Input);
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


void C_math(float inputValues[], int size, float results[]);

float testVals[5] = {5.0, 2.0, 5.0, 7.0, 6.0};

float OutputArray[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex



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
	

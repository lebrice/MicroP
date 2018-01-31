#include <stdio.h>
#include <math.h>
#include "arm_math.h"

static const int INPUT_SIZE = 10;

typedef struct {
	float rms;
	float max_value;
	float min_value;
	int max_index;
	int min_index;	
} asm_output;

//function declarations
void C_math(float inputValues[], int size, float results[]);
void CMSIS_math(float inputValues[], int size, float results[]);
void asm_math(float *inputValues, int size, asm_output *results);
void FIR_C(int Input, float* Output);

// Array of weights
float weights[5] = {0.1, 0.15, 0.5, 0.15, 0.1};
// Buffer that will hold the values as they come in.
int buffer[5];

int head, tail = 0;

float input_array[INPUT_SIZE] = {
	5, 2, 5, 7, 6, 9, 1, 8, 2, 7
};
float filtered_input[INPUT_SIZE];


float testVals[5] = {5.0, 2.0, 5.0, 7.0, 6.0};


float C_mathOutput[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex
float CMSIS_Output[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex
//float asm_Output[5]; //RMS, MaxVal, MinVal, MaxIndex, MinIndex
asm_output assembly_output;


int main()
{
	int i, j;
	float* placer = &filtered_input[0];
	
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
	
	
	asm_math(filtered_input, INPUT_SIZE, &assembly_output); //0.00000692
	C_math(filtered_input, INPUT_SIZE, C_mathOutput);//0.00009880
	CMSIS_math(filtered_input, INPUT_SIZE, CMSIS_Output);//0.00001156
	
	printf("\nasm_math output:\n");
	printf("RMS: %f\n", assembly_output.rms);
	printf("Max Value: %f", assembly_output.max_value);
	printf("  at index: %d\n", assembly_output.max_index);
	printf("Min Value: %f", assembly_output.min_value);
	printf("  at index: %d\n\n", assembly_output.min_index);
	
	printf("\nC_math output:\n");
	printf("RMS: %f\n", C_mathOutput[0]);
	printf("Max Value: %f", C_mathOutput[1]);
	printf("  at index: %f\n", C_mathOutput[3]);
	printf("Min Value: %f", C_mathOutput[2]);
	printf("  at index: %f\n\n", C_mathOutput[4]);
	
	printf("CMSIS_math output:\n");
	printf("RMS: %f\n", CMSIS_Output[0]);
	printf("Max Value: %f", CMSIS_Output[1]);
	printf("  at index: %f\n", CMSIS_Output[3]);
	printf("Min Value: %f", CMSIS_Output[2]);
	printf("  at index: %f\n", CMSIS_Output[4]);
	
	return 0;
}

void CMSIS_math(float inputValues[], int size, float results[]){
	float RMS;
	float maxVal; //Max val
	uint32_t maxIndex;
	float minVal; //Min val
	uint32_t minIndex;
	
	arm_max_f32(inputValues, size, &maxVal, &maxIndex);
	arm_min_f32(inputValues, size, &minVal, &minIndex);
	arm_rms_f32(inputValues, size, &RMS);
	CMSIS_Output[0] = RMS;
	CMSIS_Output[1] = maxVal;
	CMSIS_Output[2] = minVal;
	CMSIS_Output[3] = maxIndex;
	CMSIS_Output[4] = minIndex;
	
	
}
void C_math(float inputValues[], int size, float results[]){
		int i = 0;
		float RMS;
		float maxVal = inputValues[0]; //Max val
		float maxIndex;
		float minVal = inputValues[0]; //Min val
		float minIndex;
		int input_value;
		
		
		for(; i < size; i++){
			input_value = inputValues[i];
			RMS += pow(input_value, 2);
			
			if (input_value > maxVal){
				maxVal  = input_value;
				maxIndex = i;
			}				
			if (input_value < minVal){
				minVal  = input_value;
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
	

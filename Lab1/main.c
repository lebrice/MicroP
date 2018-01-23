#include <stdio.h>
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
	return 0;
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
		result = result + weights[i] * buffer[(head - i + 5) % 5];
	}
	*Output = result; // place the result at the given location.
}
	

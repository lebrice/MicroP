#include <stdio.h>
#include "arm_math.h"

int Example_asm(int Input);

int main()
{
	
	int Input = 10;
	printf("Begins Asm\n");
	
	Example_asm(Input);

	printf("The end!\n");
	
	return 0;
}

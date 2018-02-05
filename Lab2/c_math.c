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
#include "heads_up_display.h"

short display_mode = DISPLAY_RMS;

void set_display_mode(short new_display_mode){
	display_mode = new_display_mode;
}

short get_display_mode(){
	return display_mode;
}

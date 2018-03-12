#ifndef __display_thread_h
#define __display_thread_h
#endif

#ifndef bool
#include <stdbool.h>
#endif


// Function used to refresh the display.
void refresh_display(void);

void stop_display(void);
void start_display(void);

static bool display_on;

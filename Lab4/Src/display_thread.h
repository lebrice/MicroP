#ifndef __display_thread_h
#define __display_thread_h
#endif

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __CMSIS_OS_H
#include "cmsis_os.h"
#endif


void StartDisplayTask(void const * arguments);

// Function used to refresh the display.
void refresh_display(void);

void stop_display(void);
void start_display(void);

static bool display_on;

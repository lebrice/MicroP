#ifndef __display_thread_h
#define __display_thread_h
#endif

#ifndef __stdint_h
#include <stdint.h>
#endif

#ifndef __stdio_h
#include <stdio.h>
#endif

#ifndef bool
#include <stdbool.h>
#endif

#ifndef __CMSIS_OS_H
#include "cmsis_os.h"
#endif

#ifndef __main_h
#include "main.h"
#endif

#ifndef __segment_display_h
#include "segment_display.h"
#endif

#ifndef __math_h
#include "math.h"
#endif

void StartDisplayTask(void const * arguments);

// Function used to refresh the display.
void refresh_display(void);

void stop_display(void);
void start_display(void);


/*=============================================================================
 * beep_handle.h - Buzzer application layer interface
 *   beep_handle()          : periodic buzzer service, call every BEEP_TASK_TIME ms
 *===========================================================================*/#ifndef BEEP_HANDLE_H
#define BEEP_HANDLE_H
#define BEEP_TASK_TIME 10
#include "Inf_beep.h"
void beep_handle(void);
#endif
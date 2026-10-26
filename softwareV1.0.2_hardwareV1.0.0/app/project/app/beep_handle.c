/*=============================================================================
 * beep_handle.c - Buzzer application layer
 *
 * Periodic buzzer task (called every BEEP_TASK_TIME ms from the app
 * scheduler). Delegates all pattern generation to the low level driver
 * BeepProc() in Inf_beep.c. Higher level code only needs to call
 * BeepSetStatus() to request a beep pattern.
 *===========================================================================*/#include "beep_handle.h"

void beep_handle(void)
{
    BeepProc(&sbeep);
}
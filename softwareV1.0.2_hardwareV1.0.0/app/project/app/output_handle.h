/*=============================================================================
 * output_handle.h - Output control interface
 *   fan_output()        : cooling fan control
 *   check_amt()         : current range validation
 *   set_amt_range()     : apply current range setting
 *   check_output_state(): output enable/disable state machine
 *===========================================================================*/ \
#ifndef _OUTPUT_HANDLE_H
#define _OUTPUT_HANDLE_H
#include "CZ01.h"
#include "pid_operation.h"
void output_handle(CZ01_t *this);
#endif
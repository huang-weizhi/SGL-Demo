/*=============================================================================
 * ina226_handle.h - INA226 power monitor application layer interface
 *   ina226_handle() : poll both sensors and update spa45 measurements
 *===========================================================================*/#ifndef __INA226_HANDLE_H
#define __INA226_HANDLE_H

#include "Inf_ina226.h"

void ina226_handle(void);
#endif
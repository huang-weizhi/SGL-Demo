/*=============================================================================
 * touch_handle.h - Touch gesture handling interface
 *   touch_handle() : periodic gesture dispatch, call every TOUCH_TASK_TIME ms
 *===========================================================================*/ \
#ifndef __TOUCH_HANDLE_H
#define __TOUCH_HANDLE_H

#include "Inf_touch.h"
#include "PA45.h"
#define TOUCH_TASK_TIME 5

typedef enum
{

    TOUCH_ADD_PAGE,
    TOUCH_REDUCE_PAGE,

    TOUCH_SET_INA226_DATA_STYLE,

    TOUCH_SET_PAGE_CURVE,
    TOUCH_SET_PAGE_VALUE,
    TOUCH_SET_PAGE_POINTER,

    TOUCH_SET_PAGE_MULTIMETER,
    TOUCH_SET_PAGE_CHARGE,
    TOUCH_SET_PAGE_SET,

    TOUCH_CURVE_PAGE_VOLTAGE_FLOATING,
    TOUCH_CURVE_PAGE_RECORD,
    TOUCH_CURVE_PAGE_REPLAY,
    TOUCH_CURVE_PAGE_AI_SCAN,

    TOUCH_CURVE_PAGE_WAVE_PLAY_STATE,
    TOUCH_CURVE_PAGE_WAVE_REPLAY_STATE,

    TOUCH_POINTER_PAGE_CHANGE_LEVEL,

    TOUCH_MULTIMETER_SELECT_DC_OR_AC,
    TOUCH_MULTIMETER_SELECT_R,
    TOUCH_MULTIMETER_SELECT_C,
    TOUCH_MULTIMETER_SELECT_BEEP_OR_DIO,
    TOUCH_MULTIMETER_SELECT_ADD,
    TOUCH_MULTIMETER_SELECT_REDUCE,
    TOUCH_MULTIMETER_PAGE_WAVE_PLAY_STATE,

    TOUCH_EVENT_NONE,

} touch_event_e;
void touch_handle(void);
uint8_t touch_get_last_info(cst340_touch_info_t *out);
#endif
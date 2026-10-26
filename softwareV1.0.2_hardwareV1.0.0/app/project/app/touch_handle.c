/*=============================================================================
 * touch_handle.c - Touch gesture to UI action mapping
 *
 * Consumes gestures produced by Inf_touch.c and dispatches UI actions:
 *   - swipe left/right : switch pages (disabled while replay dialog active)
 *   - short press      : bottom page bar / on-page buttons / wave keys
 *   - double click     : play/pause waveform replay
 * Also drives text box dragging on the multimeter page via ui_text_box.
 * touch_handle() is the periodic entry point (TOUCH_TASK_TIME ms).
 *===========================================================================*/ \
#include "touch_handle.h"
#include "beep_handle.h"
#include "ui_handle.h"
#include "ui_wave_dialog.h"
void touch_event_handle(PA45_Global_t *this);
void get_touch_event(PA45_Global_t *this);

#define MULTI_DRAG_BOX_X0 68
#define MULTI_DRAG_BOX_Y0 62
#define MULTI_DRAG_BOX_X1 665
#define MULTI_DRAG_BOX_Y1 325

/***************************** bottom area touch key **************************/
/* area ID */
typedef enum
{
    RECT_BOTTON_INA226_DATA = 0,
    RECT_BOTTON_MULTIMETER,
    RECT_BOTTON_CHARGE,
    RECT_BOTTON_SET,
    RECT_BOTTON_COUNT
} bottom_id_e;

/* area define */

const touch_rect_t bottom_id[RECT_BOTTON_COUNT] = {
    {21, 360, 186, 120}, // RECT_BUTTON_CURVE
    {228, 410, 186, 70}, // RECT_BUTTON_VALUE
    {438, 410, 186, 70}, // RECT_BUTTON_POINTER
    {647, 410, 186, 70}, // RECT_BUTTON_MAIN
};

/*************************** curve page area touch key ************************/
/* area ID */
typedef enum
{
    CURVE_PAGE_SWITCH_INA226_DATA_STYLE = 0,
    CURVE_PAGE_CURVE_STATE_KEY, // stop or pause or play curve
    CURVE_PAGE_VOLTAGE_FLOATING_KEY,
    CURVE_PAGE_RECORD_KEY,
    CURVE_PAGE_REPLAY_KEY,
    CURVE_PAGE_AI_KEY,
    CURVE_PAGE_KEY_COUNT
} curve_page_key_id_e;

/* area define */
const touch_rect_t curve_page_key[CURVE_PAGE_KEY_COUNT] = {
    {353, 5, 167, 34 + 34},
    {60, 111, 653, 279},
    {558, 5, 167, 34 + 34},
    {746, 63, 108, 103},
    {746, 172, 108, 103},
    {746, 287, 109, 103},
};
/*************************** value page area touch key ************************/
/* area ID */
typedef enum
{
    VALUE_PAGE_SWITCH_INA226_DATA_STYLE = 0,
    VALUE_PAGE_KEY_COUNT
} value_page_key_id_e;
/* area define */
const touch_rect_t value_page_key[VALUE_PAGE_KEY_COUNT] = {
    {353, 5, 167, 34 + 34},
};

/*************************** pointer page area touch key ************************/
/* area ID */
typedef enum
{
    POINTER_PAGE_SWITCH_INA226_DATA_STYLE = 0,
    POINTER_PAGE_LEVEL_KEY, // change pointer current level
    POINTER_PAGE_KEY_COUNT
} pointer_page_key_id_e;
/* area define */
const touch_rect_t pointer_page_key[POINTER_PAGE_KEY_COUNT] = {
    {353, 5, 167, 34 + 34},
    {660, 80, 100, 100},
};

/************************ multimeter page area touch key **********************/
/* area ID */
typedef enum
{
    MULTIMETER_PAGE_SELECT_BEEP_OR_DIO = 0,
    MULTIMETER_PAGE_SELECT_DC_OR_AC,
    MULTIMETER_PAGE_SELECT_R,
    MULTIMETER_PAGE_SELECT_C,
    MULTIMETER_PAGE_SELECT_CHANGE,
    MULTIMETER_PAGE_WAVE_STATE_KEY,
    MULTIMETER_PAGE_KEY_COUNT
} multimeter_page_key_id_e;
/* area define */
const touch_rect_t multimeter_page_key[MULTIMETER_PAGE_KEY_COUNT] = {
    {53, 76, 105, 70},
    {53, 155, 105, 70},
    {53, 234, 105, 70},
    {53, 313, 105, 70},
    {38, 63, 132, 332},
    {199, 239, 616, 146},

};
/************************** charge page area touch key ************************/
/* area ID */
/* area define */

/************************** charge page area touch key ************************/
/* area ID */
/* area define */

/**************************** set page area touch key *************************/
/* area ID */

/* area define */

static touch_event_e touch_event = TOUCH_EVENT_NONE;

static cst340_touch_info_t s_last_touch_info;
static uint8_t s_last_touch_valid = 0;

uint8_t touch_get_last_info(cst340_touch_info_t *out)
{
    if (out == NULL)
        return 0;
    *out = s_last_touch_info;
    return s_last_touch_valid;
}

void get_touch_event(PA45_Global_t *this)
{

    if (cst340_read_touch_info(&s_last_touch_info) == 0)
    {
        s_last_touch_valid = 1;
        cst340_gesture_process(&s_last_touch_info);
    }
    else
    {
        s_last_touch_valid = 0;
    }
    gesture_t g = cst340_get_gesture();

    if (wave_dialog_is_open())
    {
        touch_event = TOUCH_EVENT_NONE;
        return;
    }

    if (g == GESTURE_NONE || this->refreshing_page)
    {
        touch_event = TOUCH_EVENT_NONE;
        return;
    }
    /* Retrieve the coordinates associated with this gesture */
    uint16_t x, y;
    cst340_get_last_gesture_pos(&x, &y);

    /* Find which region was touched */
    int8_t bottom_region = -1;
    int8_t curve_page_region = -1;
    int8_t value_page_region = -1;
    int8_t pointer_page_region = -1;
    int8_t multimeter_page_region = -1;

    for (uint8_t i = 0; i < RECT_BOTTON_COUNT; i++)
    {
        if (cst340_is_point_in_rect(x, y, &bottom_id[i]))
        {
            bottom_region = i;
            break;
        }
    }
    /* get page region */
    switch (this->page)
    {
    case PAGE_CURVE:
        for (uint8_t i = 0; i < CURVE_PAGE_KEY_COUNT; i++)
        {
            if (cst340_is_point_in_rect(x, y, &curve_page_key[i]))
            {
                curve_page_region = i;
                break;
            }
        }
        break;

    case PAGE_VALUE:
        for (uint8_t i = 0; i < VALUE_PAGE_KEY_COUNT; i++)
        {
            if (cst340_is_point_in_rect(x, y, &value_page_key[i]))
            {
                value_page_region = i;
                break;
            }
        }
        break;

    case PAGE_POINTER:
        for (uint8_t i = 0; i < POINTER_PAGE_KEY_COUNT; i++)
        {
            if (cst340_is_point_in_rect(x, y, &pointer_page_key[i]))
            {
                pointer_page_region = i;
                break;
            }
        }
        break;

    case PAGE_MULTIMETER:
        for (uint8_t i = 0; i < MULTIMETER_PAGE_KEY_COUNT; i++)
        {
            if (cst340_is_point_in_rect(x, y, &multimeter_page_key[i]))
            {
                multimeter_page_region = i;
                break;
            }
        }
        break;

    default:
        break;
    }

    /* get key status */
    switch (g)
    {
    case GESTURE_SHORT_PRESS:
#if 1
        /* botton area */
        switch (bottom_region)
        {
        case RECT_BOTTON_INA226_DATA:
            switch (this->ina226_data_show_style)
            {
            case INA226_SHOW_STYLE_0:
                touch_event = TOUCH_SET_PAGE_CURVE;
                break;

            case INA226_SHOW_STYLE_1:
                touch_event = TOUCH_SET_PAGE_VALUE;
                break;

            case INA226_SHOW_STYLE_2:
                touch_event = TOUCH_SET_PAGE_POINTER;
                break;

            default:
                break;
            }
            break;
        case RECT_BOTTON_MULTIMETER:
            touch_event = TOUCH_SET_PAGE_MULTIMETER;
            break;
        case RECT_BOTTON_CHARGE:
            touch_event = TOUCH_SET_PAGE_CHARGE;
            break;
        case RECT_BOTTON_SET:
            touch_event = TOUCH_SET_PAGE_SET;
            break;
        }

        /* curve page area */
        if (curve_page_region == CURVE_PAGE_SWITCH_INA226_DATA_STYLE)
            touch_event = TOUCH_SET_INA226_DATA_STYLE;
        else if (curve_page_region == CURVE_PAGE_VOLTAGE_FLOATING_KEY)
            touch_event = TOUCH_CURVE_PAGE_VOLTAGE_FLOATING;
        else if (curve_page_region == CURVE_PAGE_RECORD_KEY)
            touch_event = TOUCH_CURVE_PAGE_RECORD;
        else if (curve_page_region == CURVE_PAGE_REPLAY_KEY)
            touch_event = TOUCH_CURVE_PAGE_REPLAY;
        else if (curve_page_region == TOUCH_CURVE_PAGE_AI_SCAN)
            touch_event = TOUCH_CURVE_PAGE_AI_SCAN;
        /* value page area */
        if (value_page_region == VALUE_PAGE_SWITCH_INA226_DATA_STYLE)
            touch_event = TOUCH_SET_INA226_DATA_STYLE;
        /* pointer page area */
        if (pointer_page_region == POINTER_PAGE_SWITCH_INA226_DATA_STYLE)
            touch_event = TOUCH_SET_INA226_DATA_STYLE;
        else if (pointer_page_region == POINTER_PAGE_LEVEL_KEY)
            touch_event = TOUCH_POINTER_PAGE_CHANGE_LEVEL;
        /* multimeier page area */
        if (multimeter_page_region == MULTIMETER_PAGE_SELECT_DC_OR_AC)
            touch_event = TOUCH_MULTIMETER_SELECT_DC_OR_AC;
        else if (multimeter_page_region == MULTIMETER_PAGE_SELECT_R)
            touch_event = TOUCH_MULTIMETER_SELECT_R;
        else if (multimeter_page_region == MULTIMETER_PAGE_SELECT_C)
            touch_event = TOUCH_MULTIMETER_SELECT_C;
        else if (multimeter_page_region == MULTIMETER_PAGE_SELECT_BEEP_OR_DIO)
            touch_event = TOUCH_MULTIMETER_SELECT_BEEP_OR_DIO;

#endif

        break;

    case GESTURE_LONG_PRESS:

        printf("Long press\n");
        break;

    case GESTURE_DOUBLE_CLICK:
        /* curve page area */

        if (curve_page_region == CURVE_PAGE_CURVE_STATE_KEY)
        {

            if (this->wave_replay_state != STATE_WAVE_REPLAY_STOP)
            {
                touch_event = TOUCH_CURVE_PAGE_WAVE_REPLAY_STATE;
            }
            else if (this->wave_play_state == STATE_WAVE_PLAY ||
                     this->wave_play_state == STATE_WAVE_PAUSE)
            {
                touch_event = TOUCH_CURVE_PAGE_WAVE_PLAY_STATE;
            }
        }

        if (multimeter_page_region == MULTIMETER_PAGE_WAVE_STATE_KEY)
        {
            if (this->multimeter_current_setting == SELECT_AC ||
                this->multimeter_current_setting == SELECT_DC)
            {
                touch_event = TOUCH_MULTIMETER_PAGE_WAVE_PLAY_STATE;
            }
        }
        break;

    case GESTURE_SWIPE_LEFT:

        /* swipe-to-switch-pages works on every page; it is blocked ONLY
           when the curve page is showing a PAUSED waveform replay
           (swipe is used for the replay there) */
        if (!(this->page == PAGE_CURVE &&
              this->wave_replay_state == STATE_WAVE_REPLAY_PAUSE))
        {
            touch_event = TOUCH_ADD_PAGE;
        }
        break;

    case GESTURE_SWIPE_RIGHT:

        /* same rule as left swipe */
        if (!(this->page == PAGE_CURVE &&
              this->wave_replay_state == STATE_WAVE_REPLAY_PAUSE))
        {
            touch_event = TOUCH_REDUCE_PAGE;
        }
        break;

    case GESTURE_SWIPE_UP:

        /* multimeier page area */
        // if (multimeter_page_region == MULTIMETER_PAGE_SELECT_CHANGE)

        if (this->page == PAGE_MULTIMETER)
            touch_event = TOUCH_MULTIMETER_SELECT_ADD;
        break;

    case GESTURE_SWIPE_DOWN:
        /* multimeier page area */
        // if (multimeter_page_region == MULTIMETER_PAGE_SELECT_CHANGE)
        if (this->page == PAGE_MULTIMETER)
            touch_event = TOUCH_MULTIMETER_SELECT_REDUCE;
        break;

    default:
        // touch_event = TOUCH_EVENT_NONE;
        break;
    }
#if 0
    if (
        bottom_region == -1 &&
        curve_page_region == -1 &&
        value_page_region == -1 &&
        charge_page_region == -1 &&
        setting_page_region == -1 &&
        set_work_mode_page_region == -1 &&
        set_speaker_mode_page_region == -1 &&
        set_language_mode_page_region == -1 &&
        set_outputkey_mode_page_region == -1 &&
        g == GESTURE_SHORT_PRESS)
    {
          
    }
#endif
}

void touch_event_handle(PA45_Global_t *this)
{
    static bool select_dc = true;

    if (this->multimeter_current_setting == SELECT_AC)
        select_dc = false;
    else if (this->multimeter_current_setting == SELECT_DC)
        select_dc = true;

    switch (touch_event)
    {
        /* bottom */
    case TOUCH_ADD_PAGE:
        if (this->page == PAGE_CURVE ||
            this->page == PAGE_VALUE ||
            this->page == PAGE_POINTER)
        {
            this->page = PAGE_MULTIMETER;
        }
        else if (++this->page > PAGE_SET)
        {
            switch (this->ina226_data_show_style)
            {
            case INA226_SHOW_STYLE_0:
                this->page = PAGE_CURVE;
                break;

            case INA226_SHOW_STYLE_1:
                this->page = PAGE_VALUE;
                break;

            case INA226_SHOW_STYLE_2:
                this->page = PAGE_POINTER;
                break;

            default:
                break;
            }
        }
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_REDUCE_PAGE:
        if (this->page == PAGE_CURVE ||
            this->page == PAGE_VALUE ||
            this->page == PAGE_POINTER)
        {
            this->page = PAGE_SET;
        }

        else if (--this->page < PAGE_MULTIMETER)
        {
            switch (this->ina226_data_show_style)
            {
            case INA226_SHOW_STYLE_0:
                this->page = PAGE_CURVE;
                break;

            case INA226_SHOW_STYLE_1:
                this->page = PAGE_VALUE;
                break;

            case INA226_SHOW_STYLE_2:
                this->page = PAGE_POINTER;
                break;

            default:
                break;
            }
        }
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_CURVE:
        this->page = PAGE_CURVE;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_VALUE:
        this->page = PAGE_VALUE;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_POINTER:
        this->page = PAGE_POINTER;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_MULTIMETER:
        this->page = PAGE_MULTIMETER;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_CHARGE:
        this->page = PAGE_CHARGE;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_SET_PAGE_SET:
        this->page = PAGE_SET;
        sbeep.status = BEEP_SHORT;
        break;

        /* curve page */
    case TOUCH_SET_INA226_DATA_STYLE:
        if (++this->ina226_data_show_style > INA226_SHOW_STYLE_2)
            this->ina226_data_show_style = INA226_SHOW_STYLE_0;
        switch (this->ina226_data_show_style)
        {
        case INA226_SHOW_STYLE_0:
            this->page = PAGE_CURVE;
            break;
        case INA226_SHOW_STYLE_1:
            this->page = PAGE_VALUE;
            break;
        case INA226_SHOW_STYLE_2:
            this->page = PAGE_POINTER;
            break;

        default:
            break;
        }
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_VOLTAGE_FLOATING:
        this->Voltage_floating_state = !this->Voltage_floating_state;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_RECORD:
        if (this->wave_replay_state != STATE_WAVE_REPLAY_STOP)
            return; // can't record when replaying
        this->wave_record_state = !this->wave_record_state;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_REPLAY:
        if (this->wave_record_state)
            return; // can't replay when recording

        if (this->wave_replay_state == STATE_WAVE_REPLAY_STOP)
        {
            wave_dialog_open_replay(this);
            sbeep.status = BEEP_SHORT;
            break;
        }
        if (this->wave_replay_state == STATE_WAVE_REPLAY_PLAY ||
            this->wave_replay_state == STATE_WAVE_REPLAY_PAUSE)
            this->wave_replay_state = STATE_WAVE_REPLAY_STOP;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_AI_SCAN:
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_WAVE_REPLAY_STATE:

        if (this->wave_replay_state == STATE_WAVE_REPLAY_PLAY)
        {
            this->wave_replay_state = STATE_WAVE_REPLAY_PAUSE;
        }
        else if (this->wave_replay_state == STATE_WAVE_REPLAY_PAUSE)
        {
            this->wave_replay_state = STATE_WAVE_REPLAY_PLAY;
        }
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_CURVE_PAGE_WAVE_PLAY_STATE:
        if (this->wave_play_state == STATE_WAVE_PLAY)
        {
            this->wave_play_state = STATE_WAVE_PAUSE;
        }
        else if (this->wave_play_state == STATE_WAVE_PAUSE)
        {
            this->wave_play_state = STATE_WAVE_PLAY;
        }
        sbeep.status = BEEP_SHORT;
        break;
    /* pointer page event */
    case TOUCH_POINTER_PAGE_CHANGE_LEVEL:
        if (++this->pointer_show_level > POINTER_PAGE_SHOW_LEVEL_4)
        {
            this->pointer_show_level = POINTER_PAGE_SHOW_LEVEL_0;
        }

        sbeep.status = BEEP_SHORT;
        break;

        /* multimeter page event */
    case TOUCH_MULTIMETER_SELECT_DC_OR_AC:

        if (this->multimeter_current_setting != SELECT_DC &&
            this->multimeter_current_setting != SELECT_AC)
        {
            if (select_dc)
                this->multimeter_current_setting = SELECT_DC;
            else
                this->multimeter_current_setting = SELECT_AC;
        }
        else if (this->multimeter_current_setting == SELECT_DC)
        {
            this->multimeter_current_setting = SELECT_AC;
        }
        else if (this->multimeter_current_setting == SELECT_AC)
        {
            this->multimeter_current_setting = SELECT_DC;
        }

        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_SELECT_R:
        this->multimeter_current_setting = SELECT_R;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_SELECT_C:
        this->multimeter_current_setting = SELECT_C;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_SELECT_BEEP_OR_DIO:
        this->multimeter_current_setting = SELECT_BEEP_OR_DIO;
        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_SELECT_ADD:

        if (this->multimeter_current_setting == SELECT_BEEP_OR_DIO)
        {
            if (select_dc)
                this->multimeter_current_setting = SELECT_DC;
            else
                this->multimeter_current_setting = SELECT_AC;
        }
        else
        {
            if (this->multimeter_current_setting == SELECT_DC ||
                this->multimeter_current_setting == SELECT_AC)
            {
                this->multimeter_current_setting = SELECT_R;
            }
            else
            {
                this->multimeter_current_setting++;
            }
        }

        if (this->multimeter_current_setting > SELECT_C)
        {
            this->multimeter_current_setting = SELECT_BEEP_OR_DIO;
        }

        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_SELECT_REDUCE:

        if (this->multimeter_current_setting == SELECT_BEEP_OR_DIO)
        {
            this->multimeter_current_setting = SELECT_C;
        }
        else
        {
            if (this->multimeter_current_setting == SELECT_DC ||
                this->multimeter_current_setting == SELECT_AC)
            {
                this->multimeter_current_setting = SELECT_BEEP_OR_DIO;
            }

            else if (this->multimeter_current_setting == SELECT_R)
            {
                if (select_dc)
                    this->multimeter_current_setting = SELECT_DC;
                else
                    this->multimeter_current_setting = SELECT_AC;
            }
            else
            {
                this->multimeter_current_setting--;
            }
        }

        if (this->multimeter_current_setting < SELECT_BEEP_OR_DIO)
        {
            this->multimeter_current_setting = SELECT_C;
        }

        sbeep.status = BEEP_SHORT;
        break;

    case TOUCH_MULTIMETER_PAGE_WAVE_PLAY_STATE:
        this->multimeter_wave_play_state = !this->multimeter_wave_play_state;
        sbeep.status = BEEP_SHORT;
        break;
    case TOUCH_EVENT_NONE:
        break;

    default:
        break;
    }
}

void touch_handle(void)
{

    get_touch_event(&spa45); /* keep the gesture FSM fed even while swallowing */
    if (wave_dialog_touch_swallowed())
    {
        cst340_touch_info_t info;
        if (!(touch_get_last_info(&info) && info.touch_count > 0))
            wave_dialog_clear_touch_swallow(); /* finger lifted: resume normal handling */
        return;                                /* ignore actions from the press that closed a dialog */
    }
    touch_event_handle(&spa45); /* ignore actions from the press that closed a dialog */
}
#include "output_handle.h"

void beep_handle(void)
{
    BeepProc(&sbeep);
}

void heatting_handle(CZ01_t *this)
{
    static uint32_t last_refresh_tick = 0;
    uint32_t now;
    float pid_valud;
    now = xTaskGetTickCount();

    if ((uint32_t)(now - last_refresh_tick) < (uint32_t)pdMS_TO_TICKS(3000))
    {
        return;
    }

    last_refresh_tick = now;

    if (this->function.heat == HEATTING_START)
    {
        if ((this->actual_temp_c - this->cal_temp) >=  this->set_temp_c + 2)
        {
            pid_valud = 0;
        }
        else if ((this->actual_temp_c - this->cal_temp) <= this->set_temp_c - 20)
        {
            pid_valud = MAX_PWM_OUT; 
        }
        else
        {
            pid_valud = PID_Position_Calc(&handle_pid,
                                          this->set_temp_c,
                                          (this->actual_temp_c - this->cal_temp));
        }
    }
    else
    {
        pid_valud = 0;
    }

    if (pid_valud <= 0)
    {
        pid_valud = 0;
    }

    this->pwm_out = (uint32_t)pid_valud;
    tmr_channel_value_set(TMR2, TMR_SELECT_CHANNEL_4, this->pwm_out);
}


static void check_heat_state(CZ01_t *this)
{
    static uint16_t error_time = 0;

    if (this->function.heat == HEATTING_START)
    {
        if (this->hetting_error_state == HEATTING_OK)
        {
            if (this->actual_temp_c >= MAX_SET_TEMP_C + 50 ||
                    this->actual_temp_c <= MIN_SET_TEMP_C + 20)
            {
                error_time++;

                if (error_time > 1500) /* 1500ms * 20ms = 30S  */
                {
                    /* low temp */
                    if (this->actual_temp_c <= MIN_SET_TEMP_C)
                    {
                        this->function.heat = HEATTING_STOP;
                        this->hetting_error_state = HEATTING_LOW_TEMP_ERROR;
                        error_time = 0;
                    }
                    /* over temp */
                    else if (this->actual_temp_c >= MAX_SET_TEMP_C + 50)
                    {
                        this->function.heat = HEATTING_STOP;
                        this->hetting_error_state = HEATTING_OVER_TEMP_ERROR;
                        error_time = 0;
                    }
                }
            }
            else
            {
                error_time = 0;
            }
        }
    }
}


static void beep_alarm(CZ01_t *this)
{
    static uint16_t alarm_time = 0;

    if (this->hetting_error_state == HEATTING_OVER_TEMP_ERROR ||
            this->hetting_error_state == HEATTING_LOW_TEMP_ERROR)
    {
        alarm_time++;

        if (alarm_time >= 80)
        {
            sbeep.cmd = BEEP_LONG;
            alarm_time = 0;
        }
    }
    else
    {
        alarm_time = 0;
    }
}


static void beep_countdown(CZ01_t *this)
{
	static int16_t time_count;
	static bool beep_flag = false;
	
    if (this->function.heat == HEATTING_START &&
            this->countdown_time &&
            //! this->setting_temp_flag &&
            !this->setting_time_flag)
    {
        time_count++;
        beep_flag = true;

        if (time_count >= 50)
        {
            if (this->countdown_time <= 0)
            {
                this->countdown_time = 0;
            }
            else
            {
                this->countdown_time--;
            }

            time_count = 0;
        }
    }
    else
    {
        time_count = 0;
    }

    if (this->countdown_time == 0 && beep_flag)
    {
        
		if(this->function.countdown == BEEP_SOUDS)
		{
		    
		}
		else
		{
		    this->function.heat = HEATTING_STOP;
		}
		
        sbeep.cmd = BEEP_LONG;
        beep_flag = false;
    }
}
void output_handle(CZ01_t *this)
{
    beep_alarm(this);
    check_heat_state(this);
    heatting_handle(this);
    beep_countdown(this);
    if (this->function.speaker == OFF)
    {
        sbeep.off();
    }
    else
    {
        beep_handle();
    }
}
/*=============================================================================
 * ina226_handle.c - INA226 power monitor application layer
 *
 * Periodically polls both INA226 sensors (input / output side), converts raw
 * registers into volts / amps and publishes the results into the global
 * PA45_Global_t (spa45) structure used by the whole application.
 *===========================================================================*/ \
#include "ina226_handle.h"
#include "at32f403a_407_wk_config.h"
#include "PA45.h"
#include "ui_wave.h"
//
// void ina226_handle(void)
//{
// #if 1
//     static uint8_t state = 0;
//     static uint16_t reset_time = 0;
//     volatile static float last_input_current = 0;

//
//    ina226.BusVoltage = INA226_GetBusVoltage();
//    spa45.input_ina226_voltage = ina226.BusVoltage;

//
//    last_input_current = ina226.Current;
//    ina226.Current = INA226_GetCurrent() / 1000;
//    spa45.input_ina226_current = ina226.Current;

//    if (ina226.Current == -0.25f)
//    {
//        reset_time = 0;
//        ina226.Current = 0;
//    }
//    if (ina226.Current < 0.0f)
//    {
//        reset_time = 0;
//        ina226.Current = 0;
//    }

//    if (last_input_current == ina226.Current)
//    {
//        reset_time++;
//    }
//    else
//    {
//        reset_time = 0;
//    }

//    if (reset_time >= 200)
//    {
//        reset_time = 0;
//        INA226_Reset();
//    }

//    // wave_push_sample(spa45.input_ina226_voltage, spa45.input_ina226_current * 1000.0f);

// #endif
// }

void ina226_handle(void)
{
#if 1
    static uint8_t state = 0;
    static uint16_t reset_time = 0;
    volatile static float last_input_current = 0;
    switch (state)
    {
    case 0:
        // taskENTER_CRITICAL();
        ina226.BusVoltage = INA226_GetBusVoltage();
        spa45.input_ina226_voltage = ina226.BusVoltage;
        state = 1;
        // taskEXIT_CRITICAL();
        break;

    case 1:
        // taskENTER_CRITICAL();

        ina226.BusVoltage = INA226_GetBusVoltage();
        spa45.input_ina226_voltage = ina226.BusVoltage;

        spa45.input_hct6801_ac = spa45.input_hct6801_dc = spa45.input_ina226_voltage;

        last_input_current = spa45.input_ina226_current;
        ina226.Current = INA226_GetCurrent() / 1000;

        last_input_current = spa45.input_ina226_current;

        // if (ina226.Current == 0.25f || ina226.Current == -0.25f)

        if (ina226.Current == -0.25f)
        {
            reset_time = 0;
            ina226.Current = 0;
        }
        if (ina226.Current < 0.0f)
        {
            reset_time = 0;
            ina226.Current = 0;
        }

        spa45.input_ina226_current = ina226.Current;

        if (last_input_current == spa45.input_ina226_current)
        {
            reset_time++;
        }
        else
        {
            reset_time = 0;
        }

        if (reset_time >= 200)
        {
            reset_time = 0;
            INA226_Reset();
        }

        state = 1;
        break;

    default:
        state = 0;
        break;
    }

#endif
}
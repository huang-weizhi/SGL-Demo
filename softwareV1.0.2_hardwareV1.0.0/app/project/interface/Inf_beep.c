#include "inf_beep.h"

static void beep_on(void)
{
    gpio_bits_set(BEEP_PORT, BEEP_PIN);
}

static void beep_off(void)
{
    gpio_bits_reset(BEEP_PORT, BEEP_PIN);
}

beep_t sbeep =
    {
        .name = "beep",
        .status = BEEP_IDLE,
        .cmd = BEEP_STOP,
        .on = beep_on,
        .off = beep_off,
};

void BeepProc(beep_t *beep)
{
    static uint8_t s_BeepStartTime = 0;
    static uint8_t on_flag = 0;

    switch (beep->status)
    {
    case BEEP_IDLE:
        beep->off();

        if (beep->cmd == BEEP_SHORT)
        {
            beep->status = BEEP_STEP1;
            s_BeepStartTime = BEEP_SHORT_TIME;
            break;
        }
        else if (beep->cmd == BEEP_LONG)
        {
            beep->status = BEEP_STEP1;
            s_BeepStartTime = BEEP_LONG_TIME;
            break;
        }

        break;

    case BEEP_STEP1:
        beep->on();

        if (--s_BeepStartTime == 0)
        {
            beep->status = BEEP_STEP2;

            if (beep->cmd == BEEP_SHORT)
            {
                beep->status = BEEP_STEP2;
                s_BeepStartTime = BEEP_SHORT_TIME;
                break;
            }
            else if (beep->cmd == BEEP_LONG)
            {
                beep->status = BEEP_STEP2;
                s_BeepStartTime = BEEP_LONG_TIME;
                break;
            }
        }
        break;
    case BEEP_STEP2:
        beep->off();

        if (--s_BeepStartTime == 0)
        {
            beep->status = BEEP_IDLE;
            beep->cmd = BEEP_STOP;
            s_BeepStartTime = 0;
            break;
        }
        break;
    }
}

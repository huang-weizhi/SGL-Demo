#ifndef _INF_BEEP_H
#define _INF_BEEP_H

#include <stdio.h>
#include "at32f415.h"

/* output */
#define BEEP_PORT GPIOB
#define BEEP_PIN GPIO_PINS_10

#define BEEP_LONG_TIME 25
#define BEEP_SHORT_TIME 3
typedef enum
{
    BEEP_IDLE,
    BEEP_STEP1,
    BEEP_STEP2,
} beep_run_e;

typedef enum
{
    BEEP_SHORT,
    BEEP_LONG,
    BEEP_STOP,
} beep_ctr_e;

typedef struct
{
    char *name;
    beep_run_e status;
    beep_ctr_e cmd;
    void (*on)(void);
    void (*off)(void);
} beep_t;
extern beep_t sbeep;
void BeepProc(beep_t *beep);
#endif
#ifndef __INF_KEY_H
#define __INF_KEY_H

#include "at32f415.h"

/* Key events reported by the state machine */
typedef enum
{
    KEY_NONE = 0,            /* no event */
    KEY_PRESS,               /* instant event on press */
    KEY_SHORT_PRESS,         /* short press, reported after release */
    KEY_LONG_PRESS,          /* long press, reported when threshold reached */
    KEY_LONG_PRESS_CONTINUE, /* long press held, repeats periodically */
    KEY_DOUBLE_CLICK         /* double click */
} key_event_t;

/* Internal state machine states, not for user use */
typedef enum
{
    STATE_RELEASED = 0, /* released */
    STATE_PRESSED,      /* pressed and confirmed */
    STATE_DEBOUNCE,     /* debouncing */
    STATE_WAIT_DOUBLE,  /* waiting for a possible second click */
    STATE_LONG_PRESSED  /* long pressed */
} key_state_t;

/* Timing parameters, all in milliseconds */
typedef struct
{
    uint16_t debounce_ms;                     /* debounce time (ms) */
    uint16_t long_press_ms;                   /* long press threshold time (ms) */
    uint16_t double_click_interval_ms;        /* double click window time (ms) */
    uint16_t long_press_continue_interval_ms; /* long press repeat interval (ms) */
} key_timing_config_t;

/* Key object */
typedef struct
{
    /* hardware config, set by the user */
    gpio_type *port;      /* GPIO port pointer, e.g. GPIOC */
    uint16_t pin;         /* GPIO pin, e.g. GPIO_PINS_13 */
    uint8_t active_level; /* active level: 0 = active low, 1 = active high */

    /* state variables, internal use */
    key_state_t state;
    uint16_t press_count;               /* press counter (unit: scan period) */
    uint16_t release_count;             /* release counter (unit: scan period) */
    uint16_t long_press_continue_count; /* long press repeat counter */
    uint8_t click_count;                /* click counter */
    uint8_t long_press_triggered;       /* long press already reported flag */

    /* timing thresholds, internal use, unit: scan periods */
    uint8_t debounce_count;               /* debounce threshold (scan periods) */
    uint8_t long_press_count;             /* long press threshold (scan periods) */
    uint8_t double_click_count;           /* double click window (scan periods) */
    uint8_t long_press_continue_interval; /* long press repeat interval (scan periods) */
} key_obj;

/**
 * @brief  Define and initialize a key object
 * @param  name        key object name
 * @param  port        GPIO port, e.g. GPIOC
 * @param  pin         GPIO pin, e.g. GPIO_PINS_13
 * @param  active_lvl  active level: 0 = active low, 1 = active high
 */
#define KEY_DEFINE(name, port, pin, active_lvl)         \
    key_obj name = {                                    \
        port, pin, active_lvl,        /* hardware config */ \
        STATE_RELEASED, 0, 0, 0, 0, 0, /* state variables */ \
        1, 50, 15, 10                 /* default timing */   \
    }

/**
 * @brief  Set the key scan period, must be called before any timing setup
 * @param  scan_interval_ms  scan interval in ms
 */
void key_set_scan_interval(uint16_t scan_interval_ms);

/**
 * @brief  Set the key timing parameters (ms based)
 * @param  key     key object pointer
 * @param  timing  timing config struct
 */
void key_set_timing(key_obj *key, key_timing_config_t *timing);

/**
 * @brief  Set the key timing parameters (simplified version)
 * @param  key                             key object pointer
 * @param  debounce_ms                     debounce time (ms)
 * @param  long_press_ms                   long press threshold (ms)
 * @param  double_click_interval_ms        double click window (ms)
 * @param  long_press_continue_interval_ms long press repeat interval (ms), 0 = disabled
 */
void key_set_timing_simple(key_obj *key,
                           uint16_t debounce_ms,
                           uint16_t long_press_ms,
                           uint16_t double_click_interval_ms,
                           uint16_t long_press_continue_interval_ms);

/**
 * @brief  Key event detection, call periodically (once per scan period)
 * @param  key  key object pointer
 * @return detected event, one of key_event_t
 */
key_event_t key_event_check(key_obj *key);

/**
 * @brief  Check whether the key is currently held down
 * @param  key  key object
 * @return 1 = pressed, 0 = released
 */
uint8_t key_is_pressed(key_obj *key);

/**
 * @brief  Check whether the key is in long press state
 * @param  key  key object
 * @return 1 = long pressed, 0 = not long pressed
 */
uint8_t key_is_long_pressed(key_obj *key);

#endif
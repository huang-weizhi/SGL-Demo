#include "flash_handle.h"
/* Optional debug output */
// #define DEBUG_FLASH
#ifdef DEBUG_FLASH
#include <stdio.h>
#define FLASH_DEBUG(fmt, ...) printf("[FLASH] " fmt "\r\n", ##__VA_ARGS__)
#else
#define FLASH_DEBUG(...)
#endif

/* --------------------------- State machine definition --------------------------- */
typedef enum
{
    FLASH_STATE_IDLE = 0,    /* idle, waiting for data change */
    FLASH_STATE_START,       /* prepare to save */
    FLASH_STATE_UNLOCK,      /* unlock Flash */
    FLASH_STATE_ERASE_START, /* start sector erase */
    FLASH_STATE_ERASE_WAIT,  /* wait for erase to complete */
    FLASH_STATE_PROGRAM,     /* program data */
    FLASH_STATE_VERIFY,      /* verify magic word */
    FLASH_STATE_FINISH,      /* done, lock Flash */
    FLASH_STATE_ERROR        /* error, abort save */
} FlashState_t;

/* Copy of the last saved parameters (used to detect changes) */
static FlashParams_t s_lastSaved;

/* Next bank to write (0: Bank1, 1: Bank2) */
static uint8_t s_nextBank = 0;

/* Current sequence number (used on the next save) */
static uint32_t s_currentSequence = 0;

/* State machine variables */
static FlashState_t s_state = FLASH_STATE_IDLE;
static FlashParams_t s_writeData; /* data to be written */
static uint32_t s_targetAddr;     /* address currently being erased/written */
static uint8_t s_retryCount = 0;

/* --------------------------- Internal function declarations --------------------------- */
static bool ReadBank(uint32_t addr, FlashParams_t *data);
static void ValidateParams(FlashParams_t *data);
static bool DataHasChanged(void);
static void UpdateLastSaved(void);
static void SyncToCz01(const FlashParams_t *data);
static void SyncFromCz01(FlashParams_t *data);
static void SetDefaultParams(FlashParams_t *data);

/* Non-blocking erase helper functions */
static void Flash_StartErase(uint32_t addr);
static bool Flash_IsEraseDone(void);
static bool Flash_EraseHasError(void);
static void Flash_ClearErrorFlags(void);

/* --------------------------- Public function implementation --------------------------- */

void flash_init(void)
{
    FlashParams_t bank1, bank2;
    bool valid1 = ReadBank(FLASH_BANK1_ADDR, &bank1);
    bool valid2 = ReadBank(FLASH_BANK2_ADDR, &bank2);

    FlashParams_t *latest = NULL;
    uint32_t seq1 = valid1 ? bank1.sequence : 0;
    uint32_t seq2 = valid2 ? bank2.sequence : 0;

    FLASH_DEBUG("Init: bank1 valid=%d seq=%lu, bank2 valid=%d seq=%lu", valid1, seq1, valid2, seq2);

    if (valid1 && valid2)
    {
        if (seq1 >= seq2)
        {
            latest = &bank1;
            s_nextBank = 1; /* next write goes to bank2 */
            FLASH_DEBUG("Using bank1, seq=%lu", seq1);
        }
        else
        {
            latest = &bank2;
            s_nextBank = 0; /* next write goes to bank1 */
            FLASH_DEBUG("Using bank2, seq=%lu", seq2);
        }
        s_currentSequence = (seq1 > seq2 ? seq1 : seq2) + 1;
    }
    else if (valid1)
    {
        latest = &bank1;
        s_nextBank = 1;
        s_currentSequence = seq1 + 1;
        FLASH_DEBUG("Only bank1 valid, seq=%lu", seq1);
    }
    else if (valid2)
    {
        latest = &bank2;
        s_nextBank = 0;
        s_currentSequence = seq2 + 1;
        FLASH_DEBUG("Only bank2 valid, seq=%lu", seq2);
    }
    else
    {
        /* Both banks invalid, restore factory defaults */
        static FlashParams_t defaultParams;
        SetDefaultParams(&defaultParams);
        latest = &defaultParams;
        s_nextBank = 0; /* next write goes to bank1 */
        s_currentSequence = 1;
        FLASH_DEBUG("No valid bank, use defaults, seq=1");
    }

    /* Clamp parameter ranges and sync to global variables */
    ValidateParams(latest);
    SyncToCz01(latest);
    UpdateLastSaved();
    scz01.init_flag = true;

    FLASH_DEBUG("Init done, next bank=%d, next seq=%lu", s_nextBank, s_currentSequence);
}

void flash_save(void)
{
    switch (s_state)
    {
    case FLASH_STATE_IDLE:
        if (DataHasChanged())
        {
            FLASH_DEBUG("Data changed, start save");
            s_state = FLASH_STATE_START;
            s_retryCount = 0;
        }
        break;

    case FLASH_STATE_START:
        /* Wait until setting is finished */
        if (scz01.key_setting_flag)
            break;
        /* Prepare the data to write */
        SyncFromCz01(&s_writeData);
        s_writeData.magic = FLASH_MAGIC_WORD;
        s_writeData.sequence = s_currentSequence;
        s_targetAddr = (s_nextBank == 0) ? FLASH_BANK1_ADDR : FLASH_BANK2_ADDR;
        FLASH_DEBUG("Save to addr 0x%08lX, seq=%lu", s_targetAddr, s_currentSequence);
        s_state = FLASH_STATE_UNLOCK;
        break;

    case FLASH_STATE_UNLOCK:
        flash_unlock();
        s_state = FLASH_STATE_ERASE_START;
        break;

    case FLASH_STATE_ERASE_START:
        Flash_StartErase(s_targetAddr);
        s_state = FLASH_STATE_ERASE_WAIT;
        break;

    case FLASH_STATE_ERASE_WAIT:
        if (!Flash_IsEraseDone())
        {
            break; /* erase not finished yet */
        }
        if (Flash_EraseHasError())
        {
            FLASH_DEBUG("Erase error, retry %d", s_retryCount);
            Flash_ClearErrorFlags();
            FLASH->ctrl_bit.secers = FALSE;
            if (++s_retryCount < 3)
            {
                s_state = FLASH_STATE_ERASE_START; /* retry erase */
            }
            else
            {
                s_state = FLASH_STATE_ERROR;
            }
        }
        else
        {
            FLASH->ctrl_bit.secers = FALSE;
            s_state = FLASH_STATE_PROGRAM;
        }
        break;

    case FLASH_STATE_PROGRAM:
    {
        const uint16_t *src = (const uint16_t *)&s_writeData;
        uint32_t dst = s_targetAddr;
        uint32_t words = sizeof(FlashParams_t) / 2;
        for (uint32_t i = 0; i < words; i++)
        {
            flash_halfword_program(dst, src[i]);
            dst += 2;
        }
        s_state = FLASH_STATE_VERIFY;
    }
    break;

    case FLASH_STATE_VERIFY:
    {
        FlashParams_t *p = (FlashParams_t *)s_targetAddr;
        if (p->magic == FLASH_MAGIC_WORD)
        {
            FLASH_DEBUG("Write verify OK");
            s_state = FLASH_STATE_FINISH;
        }
        else
        {
            FLASH_DEBUG("Verify failed, magic=0x%08lX", p->magic);
            if (++s_retryCount < 3)
            {
                s_state = FLASH_STATE_ERASE_START; /* re-erase and retry */
            }
            else
            {
                s_state = FLASH_STATE_ERROR;
            }
        }
    }
    break;

    case FLASH_STATE_FINISH:
        flash_lock();
        /* Update state */
        s_nextBank = !s_nextBank;
        s_currentSequence++;
        UpdateLastSaved(); /* update the copy to avoid duplicate saves */
        s_state = FLASH_STATE_IDLE;
        FLASH_DEBUG("Save completed, next bank=%d, next seq=%lu", s_nextBank, s_currentSequence);
        break;

    case FLASH_STATE_ERROR:
        flash_lock();
        FLASH_DEBUG("Save failed after retries");
        s_state = FLASH_STATE_IDLE;
        break;

    default:
        s_state = FLASH_STATE_IDLE;
        break;
    }
}

/* --------------------------- Internal function implementation --------------------------- */

/* Read parameters from the given address, verify the magic word, return true on success */
static bool ReadBank(uint32_t addr, FlashParams_t *data)
{
    FlashParams_t *p = (FlashParams_t *)addr;
    if (p->magic != FLASH_MAGIC_WORD)
    {
        return false;
    }
    memcpy(data, p, sizeof(FlashParams_t));
    return true;
}

/* Clamp parameter ranges */
static void ValidateParams(FlashParams_t *data)
{
    /* Temperature: only Celsius is stored, range [MIN_SET_TEMP_C, MAX_SET_TEMP_C] */
    if (data->set_temp_c > MAX_SET_TEMP_C || data->set_temp_c < MIN_SET_TEMP_C)
        data->set_temp_c = DEFAULT_SET_TEMP;

    if (data->ch1_set_temp_c > MAX_SET_TEMP_C || data->ch1_set_temp_c < MIN_SET_TEMP_C)
        data->ch1_set_temp_c = DEFAULT_CH1_SET_TEMP;
    if (data->ch2_set_temp_c > MAX_SET_TEMP_C || data->ch2_set_temp_c < MIN_SET_TEMP_C)
        data->ch2_set_temp_c = DEFAULT_CH2_SET_TEMP;
    if (data->ch3_set_temp_c > MAX_SET_TEMP_C || data->ch3_set_temp_c < MIN_SET_TEMP_C)
        data->ch3_set_temp_c = DEFAULT_CH3_SET_TEMP;

    /* Calibration temperature */
    if (data->cal_temp > MAX_CAL_TEMP || data->cal_temp < MIN_CAL_TEMP)
        data->cal_temp = DEFAULT_CAL_TEMP;

    /* Set time */
    if (data->set_time > MAX_SET_TIME || data->set_time < MIN_SET_TIME)
        data->set_time = DEFAULT_SET_TIME;

    /* Enum types: invalid -> default */
    if (data->show_mode > 1 || data->show_mode < 0)
        data->show_mode = 0; /* SHOW_CURVE */
    if (data->language > 1 || data->language < 0)
        data->language = 0; /* CN */
    if (data->unit > 1 || data->unit < 0)
        data->unit = 0; /* CELSIUS */
    if (data->beep > 1 || data->beep < 0)
        data->beep = 1; /* force ON when invalid */
}

/* Check whether the current parameters differ from the last saved copy */
static bool DataHasChanged(void)
{
    return ((int16_t)scz01.set_temp_c != s_lastSaved.set_temp_c) ||
           ((int16_t)scz01.cal_temp != s_lastSaved.cal_temp) ||
           ((int16_t)scz01.set_time != s_lastSaved.set_time) ||

           ((int16_t)scz01.ch1_set_temp_c != s_lastSaved.ch1_set_temp_c) ||
           ((int16_t)scz01.ch2_set_temp_c != s_lastSaved.ch2_set_temp_c) ||
           ((int16_t)scz01.ch3_set_temp_c != s_lastSaved.ch3_set_temp_c) ||

           ((int16_t)scz01.function.show != s_lastSaved.show_mode) ||
           ((int16_t)scz01.function.language != s_lastSaved.language) ||
           ((int16_t)scz01.function.unit != s_lastSaved.unit) ||
           ((int16_t)scz01.function.speaker != s_lastSaved.beep);
}

/* Update the last saved copy to match the current parameters */
static void UpdateLastSaved(void)
{
    s_lastSaved.set_temp_c = scz01.set_temp_c;
    s_lastSaved.cal_temp = scz01.cal_temp;
    s_lastSaved.set_time = scz01.set_time;

    s_lastSaved.ch1_set_temp_c = scz01.ch1_set_temp_c;
    s_lastSaved.ch2_set_temp_c = scz01.ch2_set_temp_c;
    s_lastSaved.ch3_set_temp_c = scz01.ch3_set_temp_c;

    s_lastSaved.show_mode = (int16_t)scz01.function.show;
    s_lastSaved.language = (int16_t)scz01.function.language;
    s_lastSaved.unit = (int16_t)scz01.function.unit;
    s_lastSaved.beep = (int16_t)scz01.function.speaker;
}

/* Sync Flash parameters to the scz01 global variable */
static void SyncToCz01(const FlashParams_t *data)
{
    scz01.set_temp_c = data->set_temp_c;
    scz01.cal_temp = data->cal_temp;
    scz01.set_time = data->set_time;

    scz01.ch1_set_temp_c = data->ch1_set_temp_c;
    scz01.ch2_set_temp_c = data->ch2_set_temp_c;
    scz01.ch3_set_temp_c = data->ch3_set_temp_c;

    scz01.function.show = (Show_e)data->show_mode;
    scz01.function.language = (Language_e)data->language;
    scz01.function.unit = (Unit_e)data->unit;
    scz01.function.speaker = (Speaker_e)data->beep;
	
	scz01.set_temp_f = 9 * scz01.set_temp_c / 5 + 32;
	scz01.ch1_set_temp_f = 9 * scz01.ch1_set_temp_c / 5 + 32;
	scz01.ch2_set_temp_f = 9 * scz01.ch2_set_temp_c / 5 + 32;
	scz01.ch3_set_temp_f = 9 * scz01.ch3_set_temp_c / 5 + 32;
}

/* Sync from the scz01 global variable to the structure to be written */
static void SyncFromCz01(FlashParams_t *data)
{
    data->set_temp_c = scz01.set_temp_c;
    data->cal_temp = scz01.cal_temp;
    data->set_time = scz01.set_time;

    data->ch1_set_temp_c = scz01.ch1_set_temp_c;
    data->ch2_set_temp_c = scz01.ch2_set_temp_c;
    data->ch3_set_temp_c = scz01.ch3_set_temp_c;

    data->show_mode = (int16_t)scz01.function.show;
    data->language = (int16_t)scz01.function.language;
    data->unit = (int16_t)scz01.function.unit;
    data->beep = (int16_t)scz01.function.speaker;
}

/* Set factory default parameters */
static void SetDefaultParams(FlashParams_t *data)
{
    memset(data, 0, sizeof(FlashParams_t));
    data->magic = FLASH_MAGIC_WORD;
    data->sequence = 0;

    data->set_temp_c = DEFAULT_SET_TEMP; /* 210 */
    data->cal_temp = DEFAULT_CAL_TEMP;   /* 0   */
    data->set_time = DEFAULT_SET_TIME;   /* 30  */

    data->ch1_set_temp_c = DEFAULT_CH1_SET_TEMP; /* 120 */
    data->ch2_set_temp_c = DEFAULT_CH2_SET_TEMP; /* 240 */
    data->ch3_set_temp_c = DEFAULT_CH3_SET_TEMP; /* 320 */

    data->show_mode = SHOW_CURVE; /* 0 */
    data->language = CN;          /* 0 */
    data->unit = CELSIUS;         /* 0 */
    data->beep = ON;              /* 1 */
}

/* --------------------------- Non-blocking erase helper functions --------------------------- */

static void Flash_StartErase(uint32_t addr)
{
    FLASH->ctrl_bit.secers = 1; /* sector erase mode */
    FLASH->addr = addr;         /* any address within the sector is fine */
    FLASH->ctrl_bit.erstr = 1;  /* start erase */
}

static bool Flash_IsEraseDone(void)
{
    return (FLASH->sts_bit.obf == 0);
}

static bool Flash_EraseHasError(void)
{
    return (FLASH->sts_bit.prgmerr || FLASH->sts_bit.epperr);
}

static void Flash_ClearErrorFlags(void)
{
    if (FLASH->sts_bit.prgmerr)
        FLASH->sts_bit.prgmerr = 1;
    if (FLASH->sts_bit.epperr)
        FLASH->sts_bit.epperr = 1;
}
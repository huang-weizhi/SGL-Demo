#ifndef __FLASH_HANDLE_H
#define __FLASH_HANDLE_H

#include "CZ01.h"

#define FLASH_TASK_TIME 10

/* --------------------------- Flash address configuration --------------------------- */
/* AT32F415CBT7: 128KB Flash, sector size 1KB, use the last two sectors (124 and 125) */
#define FLASH_SECTOR_SIZE 1024
#define FLASH_BANK1_ADDR (0x08000000 + FLASH_SECTOR_SIZE * 124) /* sector 124 */
#define FLASH_BANK2_ADDR (0x08000000 + FLASH_SECTOR_SIZE * 125) /* sector 125 */
#define FLASH_MAGIC_WORD 0xA5A5A5A5								/* magic word, used to validate data */

/* --------------------------- Parameter structure --------------------------- */
#pragma pack(1)
typedef struct
{
	uint32_t magic;	   /* magic word */
	uint32_t sequence; /* sequence number (incremented on each save) */

	int16_t set_temp_c; /* set temperature (Celsius) */
	int16_t cal_temp;	/* temperature calibration value */
	int16_t set_time;	/* set time */

	int16_t ch1_set_temp_c; /* channel 1 set temperature (Celsius) */
	int16_t ch2_set_temp_c; /* channel 2 set temperature (Celsius) */
	int16_t ch3_set_temp_c; /* channel 3 set temperature (Celsius) */

	int16_t show_mode; /* display mode (curve / value) */
	int16_t language;  /* language */
	int16_t unit;	   /* temperature unit */
	int16_t beep;	   /* buzzer */

	uint32_t reserved; /* reserved for alignment */
} FlashParams_t;
#pragma pack()

/* --------------------------- Public interface --------------------------- */
void flash_init(void); /* power-on init, load latest params from Flash */
void flash_save(void); /* non-blocking save, call periodically (e.g. every 20ms) */

#endif
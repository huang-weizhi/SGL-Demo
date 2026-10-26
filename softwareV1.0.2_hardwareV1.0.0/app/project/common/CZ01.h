#ifndef __CZ01_H__
#define __CZ01_H__

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#include "at32f415.h"
#include "at32f415_wk_config.h"
#include "wk_system.h"



#include "iap.h"
#include "Inf_key.h"
#include "Inf_beep.h"
#include "Inf_w25qxxx.h"
#include "Inf_lcd.h"
#include "Inf_uart.h"

#include "FreeRTOS.h"
#include "task.h"

#define SOFTWARE_VERSTION "V1.0.2"
#define HARDWARE_VERSTION "V1.0.0"

#define LOCAL_DEVICE_ID_1 0x00
#define LOCAL_DEVICE_ID_2 0x00

#define DEFAULT_SET_TEMP 210
#define DEFAULT_CAL_TEMP 0
#define DEFAULT_SET_TIME 30

#define DEFAULT_CH1_SET_TEMP 120
#define DEFAULT_CH2_SET_TEMP 180
#define DEFAULT_CH3_SET_TEMP 240

#define MAX_SET_TEMP_C 300
#define MAX_SET_TEMP_F 572

#define MIN_SET_TEMP_C 50
#define MIN_SET_TEMP_F 122

#define MAX_CAL_TEMP 100
#define MIN_CAL_TEMP -100

#define MAX_SET_TIME 999
#define MIN_SET_TIME 0

#define MAX_PWM_OUT 1124

#define SETTING_TEMP_TIME 150
#define SETTING_TIME_TIME 150

#define RESET_TIME 150

typedef enum
{
	PAGE_LOGO = 0,
	PAGE_CURVE,
	PAGE_VALUE,
	/* MENU LIST 1 */
	PAGE_MENU_SELECT_SHOW_STYLE_CN,
	PAGE_MENU_UNSELECT_SHOW_STYLE_CN,
	PAGE_MENU_SELECT_SHOW_STYLE_EN,
	PAGE_MENU_UNSELECT_SHOW_STYLE_EN,
	PAGE_MENU_SELECT_UNIT_CN,
	PAGE_MENU_UNSELECT_UNIT_CN,
	PAGE_MENU_SELECT_UNIT_EN,
	PAGE_MENU_UNSELECT_UNIT_EN,
	PAGE_MENU_SELECT_CALTEMP_CN,
	PAGE_MENU_UNSELECT_CALTEMP_CN,
	PAGE_MENU_SELECT_CALTEMP_EN,
	PAGE_MENU_UNSELECT_CALTEMP_EN,
	PAGE_MENU_SELECT_LANGUAGE_CN,
	PAGE_MENU_UNSELECT_LANGUAGE_CN,
	PAGE_MENU_SELECT_LANGUAGE_EN,
	PAGE_MENU_UNSELECT_LANGUAGE_EN,
	PAGE_MENU_SELECT_SPEAKER_CN,
	PAGE_MENU_UNSELECT_SPEAKER_CN,
	PAGE_MENU_SELECT_SPEAKER_EN,
	PAGE_MENU_UNSELECT_SPEAKER_EN,
	/* MENU LIST 2 */
	PAGE_MENU_SELECT_COUNTDOWN_MODE_CN,
	PAGE_MENU_UNSELECT_COUNTDOWN_MODE_CN,
	PAGE_MENU_SELECT_COUNTDOWN_MODE_EN,
	PAGE_MENU_UNSELECT_COUNTDOWN_MODE_EN,
	PAGE_MENU_SELECT_VERSTION_CN,
	PAGE_MENU_UNSELECT_VERSTION_CN,
	PAGE_MENU_SELECT_VERSTION_EN,
	PAGE_MENU_UNSELECT_VERSTION_EN,
	PAGE_MENU_SELECT_RESET_CN,
	PAGE_MENU_UNSELECT_RESET_CN,
	PAGE_MENU_SELECT_RESET_EN,
	PAGE_MENU_UNSELECT_RESET_EN,
	PAGE_MENU_SELECT_SUPPORT_CN,
	PAGE_MENU_UNSELECT_SUPPORT_CN,
	PAGE_MENU_SELECT_SUPPORT_EN,
	PAGE_MENU_UNSELECT_SUPPORT_EN,
	PAGE_MENU_SELECT_RETURN_MENU_CN,
	PAGE_MENU_UNSELECT_RETURN_MENU_CN,
	PAGE_MENU_SELECT_RETURN_MENU_EN,
	PAGE_MENU_UNSELECT_RETURN_MENU_EN,
	/* SETTING */
	PAGE_SETTING_SHOW_STYLE_CN,
	PAGE_SETTING_SHOW_STYLE_EN,
	PAGE_SETTING_UNIT_CN,
	PAGE_SETTING_UNIT_EN,
	PAGE_SETTING_CALTEMP_CN,
	PAGE_SETTING_CALTEMP_EN,
	PAGE_SETTING_CN,
	PAGE_SETTING_EN,
	PAGE_SETTING_SPEAKER_CN,
	PAGE_SETTING_SPEAKER_EN,
	PAGE_SETTING_COUNTDOWN_MODE_CN,
	PAGE_SETTING_COUNTDOWN_MODE_EN,
	PAGE_SETTING_VERSTION_CN,
	PAGE_SETTING_VERSTION_EN,
	PAGE_SETTING_UNSELECT_RESET_CN,
	PAGE_SETTING_UNSELECT_RESET_EN,
	PAGE_SETTING_SELECT_RESET_CN,
	PAGE_SETTING_SELECT_RESET_EN,
	PAGE_SETTING_RESET_DONE_CN,
	PAGE_SETTING_RESET_DONE_EN,
	PAGE_SETTING_SUPPORT_CN,
	PAGE_SETTING_SUPPORT_EN,
} Page_e;

typedef enum
{
	MENU_SELECT_PAGE_MODE,
	MENU_SELECT_UNIT,
	MENU_SELECT_CAL_TEMP,
	MENU_SELECT_LANGUAGE,
	MENU_SELECT_SPEAKER,
	MENU_SELECT_COUNTDOWN_MODE,
	MENU_SELECT_VERSTION,
	MENU_SELECT_RESET,
	MENU_SELECT_SUPPORT,
	MENU_SELECT_EXIT_MENU,
} Menu_e;

typedef enum
{
	/* !!! one-to-one with lcd_picture_table[] in Inf_lcd_picture.h !!!
	 * !!! the enum value IS the picture id, do not insert in the middle !!! */

	/* 0 */
	ICON_LOGO = 0,

	/* USE IN CURVE PAGE, picture 1-9 */
	CURVE_PAGE_ICON_STOP_HEATTING_WITH_SETTING_TIME,	/* 1 */
	CURVE_PAGE_ICON_START_HEATTING_WITH_SETTING_TIME,	/* 2 */
	CURVE_PAGE_ICON_STOP_HEATTING_WITH_UNSETTING_TIME,	/* 3 */
	CURVE_PAGE_ICON_START_HEATTING_WITH_UNSETTING_TIME, /* 4 */

	CURVE_PAGE_ICON_START_SETTING_TEMP, /* 5 */
	CURVE_PAGE_ICON_STOP_SETTING_TEMP,	/* 6 */
	CURVE_PAGE_ICON_CELSIUS,			/* 7 */
	CURVE_PAGE_ICON_FAHRENHEIT,			/* 8 */
	CURVE_PAGE_ICON_CURVE_SCALE,		/* 9 */

	/* USE IN VALUE PAGE, picture 10-17 */
	VALUE_PAGE_ICON_STOP_HEATTING_WITH_SETTING_TIME,	/* 10 */
	VALUE_PAGE_ICON_START_HEATTING_WITH_SETTING_TIME,	/* 11 */
	VALUE_PAGE_ICON_STOP_HEATTING_WITH_UNSETTING_TIME,	/* 12 */
	VALUE_PAGE_ICON_START_HEATTING_WITH_UNSETTING_TIME, /* 13 */

	VALUE_PAGE_ICON_START_SETTING_TEMP, /* 14 */
	VALUE_PAGE_ICON_STOP_SETTING_TEMP,	/* 15 */
	VALUE_PAGE_ICON_CELSIUS,			/* 16 */
	VALUE_PAGE_ICON_FAHRENHEIT,			/* 17 */

	/* USE IN MENU 1 PAGE, picture 18-27 */
	MENU_1_PAGE_ICON_SELECT_SHOW_STYLE_CN,	 /* 18 */
	MENU_1_PAGE_ICON_UNSELECT_SHOW_STYLE_CN, /* 19 */

	MENU_1_PAGE_ICON_SELECT_UNIT_CN,   /* 20 */
	MENU_1_PAGE_ICON_UNSELECT_UNIT_CN, /* 21 */

	MENU_1_PAGE_ICON_SELECT_CALTEMP_CN,	  /* 22 */
	MENU_1_PAGE_ICON_UNSELECT_CALTEMP_CN, /* 23 */

	MENU_1_PAGE_ICON_SELECT_LANGUAGE_CN,   /* 24 */
	MENU_1_PAGE_ICON_UNSELECT_LANGUAGE_CN, /* 25 */

	MENU_1_PAGE_ICON_SELECT_SPEAKER_CN,	  /* 26 */
	MENU_1_PAGE_ICON_UNSELECT_SPEAKER_CN, /* 27 */

	/* USE IN MENU 2 PAGE, picture 28-37 */
	MENU_2_PAGE_ICON_SELECT_COUNTDOWN_MODE_CN,   /* 28 */
	MENU_2_PAGE_ICON_UNSELECT_COUNTDOWN_MODE_CN, /* 29 */
	
	MENU_2_PAGE_ICON_SELECT_VERSTION_CN,   /* 30 */
	MENU_2_PAGE_ICON_UNSELECT_VERSTION_CN, /* 31 */

	MENU_2_PAGE_ICON_SELECT_RESET_CN,	/* 32 */
	MENU_2_PAGE_ICON_UNSELECT_RESET_CN, /* 33 */

	MENU_2_PAGE_ICON_SELECT_SUPPORT_CN,	  /* 34 */
	MENU_2_PAGE_ICON_UNSELECT_SUPPORT_CN, /* 35 */

	MENU_2_PAGE_ICON_SELECT_EXIT_CN,   /* 36 */
	MENU_2_PAGE_ICON_UNSELECT_EXIT_CN, /* 37 */

	/* USE IN SETTING PAGES, picture 38-55 */
	SETTING_PAGE_ICON_SHOW_STYLE_CN,		  /* 38 */
	SETTING_PAGE_ICON_UNIT_CN,				  /* 39 */
	SETTING_PAGE_ICON_CALTEMP_CN,			  /* 40 */
	SETTING_PAGE_ICON_LANGUAGE_EN,			  /* 41 */
	SETTING_PAGE_ICON_LANGUAGE_CN,			  /* 42 */
	SETTING_PAGE_ICON_SPEAKER_CN,			  /* 43 */
	SETTING_PAGE_ICON_COUNTDOWN_MODE_CN,			  /* 44 */
	SETTING_PAGE_ICON_VERSTION_CN,			  /* 45 */
	SETTING_PAGE_ICON_RESET_UNSELECT_CN,	  /* 46 */
	SETTING_PAGE_ICON_RESET_SELECT_CN,		  /* 47 */
	SETTING_PAGE_ICON_RESET_DONE_CN,		  /* 48 */
	SETTING_PAGE_ICON_SUPPORT_CN,			  /* 49 */
	SETTING_PAGE_ICON_RESET_BACK_SELECT_CN,	  /* 50 */
	SETTING_PAGE_ICON_RESET_BACK_UNSELECT_CN, /* 51 */
	SETTING_PAGE_ICON_SELECT_OPTION,		  /* 52 */
	SETTING_PAGE_ICON_UNSELECT_OPTION,		  /* 53 */
	SETTING_PAGE_ICON_CALTEMP_CELSIUS,		  /* 54 */
	SETTING_PAGE_ICON_CALTEMP_FAHRENHEIT,	  /* 55 */


	/* COMMON ICONS, used in several pages, picture 49-58 */
	ICON_STATUS_BAR_TOP,	/* 56 */
	ICON_STATUS_BAR_BOTTOM_CN, /* 57 */
	ICON_VOLUME_ON,			/* 58 */
	ICON_VOLUME_OFF,		/* 59 */
	ICON_CH1_SELECT,		/* 60 */
	ICON_CH1_UNSELECT,		/* 61 */
	ICON_CH2_SELECT,		/* 62 */
	ICON_CH2_UNSELECT,		/* 63 */
	ICON_CH3_SELECT,		/* 64 */
	ICON_CH3_UNSELECT,		/* 65 */

} Icon_e;

typedef enum
{
	SHOW_CURVE,
	SHOW_VALUE,
} Show_e;

typedef enum
{
	CELSIUS,
	FAHRENHEIT,
} Unit_e;

typedef enum
{
	CN,
	EN,
} Language_e;

typedef enum
{
	OFF,
	ON,
} Speaker_e;

typedef enum
{
	SELECT_NONE,
	SELECT_CHANNEL_1,
	SELECT_CHANNEL_2,
	SELECT_CHANNEL_3,

} Channel_e;

typedef enum
{
	HEATTING_STOP,
	HEATTING_START,
} Heatting_e;

typedef enum
{
	BEEP_SOUDS,
	BEEP_SOUDS_AND_HEATING_STOP,
} Countdown_e;

typedef struct
{
	Show_e show;
	Language_e language;
	Unit_e unit;
	Speaker_e speaker;
	Channel_e channel;
	Heatting_e heat;
	Countdown_e countdown;
	
} Function_t;

typedef enum
{
	SELECT_SETTING_TEMP,
	SELECT_SETTING_TIME,
} KeySettingSelect_e;

typedef enum
{
    HEATTING_OK = 0,
    HEATTING_OVER_TEMP_ERROR,
    HEATTING_LOW_TEMP_ERROR,
} HettingErrorState_e;

typedef struct
{

	/* CZ01 data */
	int16_t set_temp_c;
	int16_t last_set_temp_c;

	int16_t set_temp_f;
	int16_t last_set_temp_f;

	volatile int16_t actual_temp_c; /* written by DMA1_Channel1_IRQHandler */
	int16_t last_actual_temp_c;

	int16_t actual_temp_f;
	int16_t last_actual_temp_f;

	int16_t show_actual_temp;
	int16_t last_show_actual_temp;

	int16_t show_set_temp;
	int16_t last_show_set_temp;

	int16_t cal_temp;
	int16_t show_cal_temp_f;
	int16_t show_cal_temp_c;

	int16_t set_time;
	int16_t last_set_time;
	int16_t countdown_time;

	int16_t ch1_set_temp_c;
	int16_t ch2_set_temp_c;
	int16_t ch3_set_temp_c;

	int16_t ch1_set_temp_f;
	int16_t ch2_set_temp_f;
	int16_t ch3_set_temp_f;

	int16_t show_ch1_set_temp;
	int16_t show_ch2_set_temp;
	int16_t show_ch3_set_temp;

	uint16_t ad_value_buff[20]; /* DMA ring: 20 NTC samples = 20 ms window, processed by DMA1_Channel1_IRQHandler */

	/* CZ01 setting */
	Function_t function;
	Function_t last_function;
	bool key_setting_flag;
	uint8_t key_setting_time;
	bool flash_updata;
	/* CZ01 info */
	uint8_t sw_ver_major;
	uint8_t sw_ver_minor;
	uint8_t sw_ver_patch;
	uint8_t hw_ver_major;
	uint8_t hw_ver_minor;
	uint8_t hw_ver_patch;
	bool reset_flag;
	uint16_t reset_time;
	bool init_flag;
	/* CZ01 page */
	Page_e page;
	Page_e last_page;
	Menu_e menu_select;
	Menu_e last_menu_select;
	bool refreshing_page;

	/* CZ01 key status */
	bool setting_temp_flag;
	uint8_t setting_temp_time;
	bool setting_time_flag;
	uint8_t setting_time_time;
	KeySettingSelect_e key_setting_select;
	/* mcu info */
	volatile int16_t mcu_temp_c; /* written by DMA1_Channel1_IRQHandler */
	uint16_t mcu_temp_f;
	uint32_t mcu_size;
	uint32_t uid_value;
	uint8_t mcu_uid_buff[12];
	
	/* heatting info */
	HettingErrorState_e hetting_error_state;
	uint32_t pwm_out;
	
	
 

} CZ01_t;
extern CZ01_t scz01;

uint32_t convert_data(const uint8_t *input_array, uint32_t *output_array,
					  uint32_t start_index, uint32_t end_index);
void get_cz01_version(CZ01_t *this);
uint32_t get_mcu_info(CZ01_t *this);

#endif

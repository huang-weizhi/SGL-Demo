#ifndef __INF_LCD_DEV_H__
#define __INF_LCD_DEV_H__

#include "stdint.h"
#include "at32f415.h"
#include "Inf_lcd_port.h"

/*******************************************************************************
 * LCD controller layer (ST7789V family)
 *  - command/data writes, address window setup, initialization sequence
 ******************************************************************************/

#define USE_HORIZONTAL 0 /* 0/1 = portrait, 2/3 = landscape */

#if (USE_HORIZONTAL == 0 || USE_HORIZONTAL == 1)
#define LCD_W 240
#define LCD_H 320
#else
#define LCD_W 320
#define LCD_H 240
#endif

/* Run hardware reset + full register initialization sequence */
void lcd_dev_init(void);

/* Write one command byte (DC = low) */
void lcd_dev_write_cmd(uint8_t cmd);

/* Write one data byte (DC = high) */
void lcd_dev_write_data8(uint8_t dat);

/* Write one 16-bit data word, big-endian on the wire (DC = high) */
void lcd_dev_write_data16(uint16_t dat);

/* Set RAM address window [x1..x2] x [y1..y2], ends with 0x2C so pixel data follows */
void lcd_dev_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

#endif /* __INF_LCD_DEV_H__ */
#ifndef __INF_LCD_H__
#define __INF_LCD_H__

#include "stdint.h"
#include "Inf_lcd_dev.h"

/*******************************************************************************
 * LCD graphics/text public API (the only header the application needs to include)
 *
 *  color : RGB565
 *  text  : ASCII, heights 12/16/24/32 (width = height / 2)
 *  mode  : 0 = fill background, 1 = transparent background (strokes only)
 *
 *  picture: lcd_show_flash_picture(), the external flash read function
 *           is injected by the caller, no storage chip code is required
 ******************************************************************************/

/* common RGB565 colors */
#define WHITE           0xFFFF
#define BLACK           0x0000
#define BLUE            0x001F
#define BRED            0XF81F
#define GRED            0XFFE0
#define GBLUE           0X07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define GREEN           0x07E0
#define CYAN            0x7FFF
#define YELLOW          0xFFE0
#define CRIMSON         0XD8A7
#define TROPICAL_ORANGE 0XFC06
#define BROWN           0XBC40
#define BRRED           0XFC07
#define GRAY            0X8430
#define DARKBLUE        0X01CF
#define LIGHTBLUE       0X7D7C
#define GRAYBLUE        0X5458
#define LIGHTGREEN      0X841F
#define LGRAY           0XC618
#define LGRAYBLUE       0XA651
#define LBBLUE          0X2B12

/* external flash read function prototype: read(buf, flash_addr, len) */
typedef void (*lcd_flash_read_fn)(uint8_t *buf, uint32_t flash_addr, uint32_t len);

/* lifecycle */
void lcd_init(void);
void lcd_set_backlight(uint8_t on);

/* area / pixel */
void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_clear(uint16_t color);
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);

/* graphics primitives */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

/* text */
void lcd_show_char(uint16_t x, uint16_t y, uint8_t ch, uint16_t fc, uint16_t bc,
                   uint8_t size, uint8_t mode);
void lcd_show_string(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc,
                     uint8_t size, uint8_t mode);
void lcd_show_int(uint16_t x, uint16_t y, int32_t num, uint8_t len, uint16_t fc,
                  uint16_t bc, uint8_t size);
void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t int_len, uint8_t dec_len,
                    uint16_t fc, uint16_t bc, uint8_t size);

/* external flash RGB565 picture */
void lcd_show_flash_picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                            uint32_t flash_addr, lcd_flash_read_fn read_fn);

#endif /* __INF_LCD_H__ */

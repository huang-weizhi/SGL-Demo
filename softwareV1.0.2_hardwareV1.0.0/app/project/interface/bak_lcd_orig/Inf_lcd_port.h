#ifndef __INF_LCD_PORT_H__
#define __INF_LCD_PORT_H__

#include "at32f415_wk_config.h"

/*******************************************************************************
 * LCD hardware abstraction layer (HAL)
 *  - This file is the ONLY place allowed to touch SPI / DMA / GPIO pins
 *  - Pin definitions must match wk_gpio.c / at32f415_wk_config.h:
 *    CS  : chip select, active low
 *    DC  : high = data, low = command
 *    BL  : high = backlight on
 *    RST : low = controller reset
 *    SCK/MOSI/MISO : SPI2
 ******************************************************************************/

#define LCD_SPI                     SPI2

/* SPI2 transmit request mapped to DMA1 channel 4 */
#define LCD_DMA_TX_CHANNEL          DMA1_CHANNEL4
#define LCD_DMA_TX_FDT_FLAG         DMA1_FDT4_FLAG

#define LCD_CS_GPIO_PORT            OUT_LCD_CS_L_GPIO_PORT
#define LCD_CS_PIN                  OUT_LCD_CS_L_PIN
#define LCD_DC_GPIO_PORT            OUT_LCD_DC_HL_GPIO_PORT
#define LCD_DC_PIN                  OUT_LCD_DC_HL_PIN
#define LCD_RST_GPIO_PORT           OUT_LCD_RST_H_GPIO_PORT
#define LCD_RST_PIN                 OUT_LCD_RST_H_PIN
#define LCD_BL_GPIO_PORT            OUT_LCD_BL_H_GPIO_PORT
#define LCD_BL_PIN                  OUT_LCD_BL_H_PIN

/* Fill threshold: at or below this pixel count use polled SPI writes, larger areas use DMA bulk transfer */
#define LCD_DMA_THRESHOLD           32U

/* Chip select control: 0 = asserted (low), 1 = released (high) */
void lcd_port_cs(uint8_t select);

/* Data/command line control: 0 = command, 1 = data */
void lcd_port_dc(uint8_t is_data);

/* Hardware reset line control: 0 = asserted, 1 = released */
void lcd_port_rst(uint8_t level);

/* Backlight control: 0 = off, 1 = on */
void lcd_port_backlight(uint8_t on);

/* Blocking single byte write over SPI (does not touch CS/DC) */
void lcd_port_send_byte(uint8_t dat);

/* Blocking bulk write over DMA, up to 65535 bytes per call (does not touch CS/DC) */
void lcd_port_send_bulk(const uint8_t *buf, uint32_t len);

#endif /* __INF_LCD_PORT_H__ */
#include "Inf_lcd_dev.h"
#include "wk_system.h"

/*******************************************************************************
 * LCD controller layer (ST7789V) implementation
 *  - hardware reset, command/data writes, address window setup
 ******************************************************************************/

/*******************************************************************************
 * Basic command/data access
 ******************************************************************************/

void lcd_dev_write_cmd(uint8_t cmd)
{
    lcd_port_dc(0);
    lcd_port_send_byte(cmd);
}

void lcd_dev_write_data8(uint8_t dat)
{
    lcd_port_dc(1);
    lcd_port_send_byte(dat);
}

void lcd_dev_write_data16(uint16_t dat)
{
    lcd_port_dc(1);
    lcd_port_send_byte((uint8_t)(dat >> 8));
    lcd_port_send_byte((uint8_t)dat);
}

/*******************************************************************************
 * Address window (supports all 4 orientations)
 ******************************************************************************/
void lcd_dev_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_dev_write_cmd(0x2A); /* column address set */

#if (USE_HORIZONTAL == 3)
    lcd_dev_write_data16(x1 + 80);
    lcd_dev_write_data16(x2 + 80);
#else
    lcd_dev_write_data16(x1);
    lcd_dev_write_data16(x2);
#endif

    lcd_dev_write_cmd(0x2B); /* row address set */

#if (USE_HORIZONTAL == 1)
    lcd_dev_write_data16(y1 + 80);
    lcd_dev_write_data16(y2 + 80);
#else
    lcd_dev_write_data16(y1);
    lcd_dev_write_data16(y2);
#endif

    lcd_dev_write_cmd(0x2C); /* memory write, window now ready for pixel data */
}

/*******************************************************************************
 * Initialization sequence (ST7789V)
 *  - hardware reset first, then controller register configuration
 ******************************************************************************/
void lcd_dev_init(void)
{
    /* hardware reset: pull low then release, wait for controller ready */
    lcd_port_rst(0);
    wk_delay_ms(200);
    lcd_port_rst(1);
    wk_delay_ms(120);

    lcd_port_backlight(0); /* keep backlight off while configuring */
    wk_delay_ms(50);

    lcd_dev_write_cmd(0x11); /* sleep out */
    wk_delay_ms(120);

    lcd_dev_write_cmd(0x28); /* display off (avoid white flash) */
    wk_delay_ms(20);

    lcd_dev_write_cmd(0x36); /* memory access control (orientation) */
#if (USE_HORIZONTAL == 0)
    lcd_dev_write_data8(0x00);
#elif (USE_HORIZONTAL == 1)
    lcd_dev_write_data8(0xC0);
#elif (USE_HORIZONTAL == 2)
    lcd_dev_write_data8(0x70);
#else
    lcd_dev_write_data8(0xA0);
#endif

    lcd_dev_write_cmd(0x3A); /* pixel format: 16 bits per pixel RGB565 */
    lcd_dev_write_data8(0x05);

    lcd_dev_write_cmd(0xB2); /* porch control (front/back porch) */
    lcd_dev_write_data8(0x0C);
    lcd_dev_write_data8(0x0C);
    lcd_dev_write_data8(0x00);
    lcd_dev_write_data8(0x33);
    lcd_dev_write_data8(0x33);

    lcd_dev_write_cmd(0xB7); /* gate control */
    lcd_dev_write_data8(0x35);

    lcd_dev_write_cmd(0xBB); /* VCOM setting */
    lcd_dev_write_data8(0x19);

    lcd_dev_write_cmd(0xC0); /* LCM control */
    lcd_dev_write_data8(0x2C);

    lcd_dev_write_cmd(0xC2); /* source control */
    lcd_dev_write_data8(0x01);

    lcd_dev_write_cmd(0xC3); /* VRH / GVDD voltage: 4.2V */
    lcd_dev_write_data8(0x0D);

    lcd_dev_write_cmd(0xC4); /* door control */
    lcd_dev_write_data8(0x20);

    lcd_dev_write_cmd(0xC6); /* frame rate: 60Hz */
    lcd_dev_write_data8(0x0F);

    lcd_dev_write_cmd(0xD0); /* LCM control: power settings */
    lcd_dev_write_data8(0xA4);
    lcd_dev_write_data8(0xA1);

    lcd_dev_write_cmd(0xE0); /* gamma correction, positive polarity */
    lcd_dev_write_data8(0xD0);
    lcd_dev_write_data8(0x00);
    lcd_dev_write_data8(0x05);
    lcd_dev_write_data8(0x0F);
    lcd_dev_write_data8(0x10);
    lcd_dev_write_data8(0x28);
    lcd_dev_write_data8(0x34);
    lcd_dev_write_data8(0x50);
    lcd_dev_write_data8(0x44);
    lcd_dev_write_data8(0x3A);
    lcd_dev_write_data8(0x0B);
    lcd_dev_write_data8(0x06);
    lcd_dev_write_data8(0x11);
    lcd_dev_write_data8(0x20);

    lcd_dev_write_cmd(0xE1); /* gamma correction, negative polarity */
    lcd_dev_write_data8(0xD0);
    lcd_dev_write_data8(0x00);
    lcd_dev_write_data8(0x05);
    lcd_dev_write_data8(0x0A);
    lcd_dev_write_data8(0x0B);
    lcd_dev_write_data8(0x16);
    lcd_dev_write_data8(0x32);
    lcd_dev_write_data8(0x40);
    lcd_dev_write_data8(0x4A);
    lcd_dev_write_data8(0x2B);
    lcd_dev_write_data8(0x1B);
    lcd_dev_write_data8(0x1C);
    lcd_dev_write_data8(0x22);
    lcd_dev_write_data8(0x1F);

    lcd_dev_write_cmd(0x21); /* display inversion on */

    /* note: 0x29 (display on) is sent by lcd_init() AFTER clearing the panel,
     * matching the reference driver's LCD_Init() order. */
}	
#include "Inf_lcd.h"
#include "Inf_lcd_font.h"

/*******************************************************************************
 * Lifecycle management
 ******************************************************************************/
/* Initialize the LCD: hardware reset + register configuration via controller layer */
void lcd_init(void)
{
    lcd_dev_init();
    /* mirror reference LCD_Init(): clear panel while display is off,
     * then turn the display on */
    lcd_clear(BLACK);
    lcd_dev_write_cmd(0x29); /* display on */
}

/* Turn backlight on/off: 0 = off, 1 = on */
void lcd_set_backlight(uint8_t on)
{
    lcd_port_backlight(on);
}

/* Set the RAM address window, pixel data written after this call fills [x1..x2]x[y1..y2] */
void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_dev_set_window(x1, y1, x2, y2);
}

/*******************************************************************************
 * Area fill (screen refresh)
 *  - small area : direct byte-by-byte SPI writes, low overhead
 *  - large area : small internal buffer moved via DMA1 channel 4
 *  note: DMA counter is 16-bit (max 65535 bytes per transfer), so loop in chunks
 ******************************************************************************/
#define LCD_FILL_BUF_PIXELS 256U    /* 256 pixels = 512 bytes */

void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    static uint8_t fill_buf[LCD_FILL_BUF_PIXELS * 2];
    uint32_t total, remain;
    uint32_t chunk_bytes;
    uint32_t i;

    lcd_set_window(x1, y1, x2, y2);
    total = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);
    lcd_port_cs(0);

    if (total <= LCD_DMA_THRESHOLD)
    {
        for (i = 0; i < total; i++)
        {
            lcd_dev_write_data16(color);
        }

        lcd_port_cs(1);
        return;
    }

    /* pre-fill the color buffer (big-endian: high byte first) */
    for (i = 0; i < LCD_FILL_BUF_PIXELS; i++)
    {
        fill_buf[i * 2]     = (uint8_t)(color >> 8);
        fill_buf[i * 2 + 1] = (uint8_t)color;
    }

    chunk_bytes = sizeof(fill_buf);

    while (total)
    {
        remain = total * 2;

        if (remain > chunk_bytes)
        {
            remain = chunk_bytes;
        }

        lcd_port_send_bulk(fill_buf, remain);
        total -= remain / 2;
    }

    lcd_port_cs(1);
}

/* Clear the whole screen with one color */
void lcd_clear(uint16_t color)
{
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, color);
}

/* Draw one pixel */
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_window(x, y, x, y);
    lcd_dev_write_data16(color);
}

/*******************************************************************************
 * Graphics primitives
 ******************************************************************************/
/* Draw a line using the Bresenham algorithm, works in all directions */
void lcd_draw_line(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, uRow, uCol;
    delta_x = x2 - x1;
    delta_y = y2 - y1;
    uRow = x1;
    uCol = y1;

    if (delta_x > 0)
    {
        incx = 1;
    }
    else if (delta_x == 0)
    {
        incx = 0;
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0;
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    distance = (delta_x > delta_y) ? delta_x : delta_y;

    for (t = 0; t < distance + 1; t++)
    {
        lcd_draw_point(uRow, uCol, color);
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            uRow += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            uCol += incy;
        }
    }
}

/* Draw a rectangle outline (4 lines) */
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(x1, y1, x2, y1, color);
    lcd_draw_line(x1, y1, x1, y2, color);
    lcd_draw_line(x1, y2, x2, y2, color);
    lcd_draw_line(x2, y1, x2, y2, color);
}

/* Draw a circle outline using the midpoint algorithm, radius in pixels */
void lcd_draw_circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a, b;
    a = 0;
    b = r;

    while (a <= b)
    {
        lcd_draw_point(x0 - b, y0 - a, color);
        lcd_draw_point(x0 + b, y0 - a, color);
        lcd_draw_point(x0 - a, y0 + b, color);
        lcd_draw_point(x0 - a, y0 - b, color);
        lcd_draw_point(x0 + b, y0 + a, color);
        lcd_draw_point(x0 + a, y0 - b, color);
        lcd_draw_point(x0 + a, y0 + b, color);
        lcd_draw_point(x0 - b, y0 + a, color);
        a++;

        if ((a * a + b * b) > (r * r))
        {
            b--;
        }
    }
}

/*******************************************************************************
 * ASCII text
 ******************************************************************************/
/* Get the bitmap of one char for the given size, width in pixels, 0 if unsupported size */
static const unsigned char *lcd_font_get(uint8_t size, uint8_t ch, uint8_t *width)
{
    ch = ch - ' ';

    switch (size)
    {
        case 12:
            *width = 6;
            return ascii_1206[ch];
        case 16:
            *width = 8;
            return ascii_1608[ch];
        case 24:
            *width = 12;
            return ascii_2412[ch];
        case 32:
            *width = 16;
            return ascii_3216[ch];
        default:
            *width = 0;
            return 0;
    }
}

/* Show one ASCII char; mode: 0 = fill background, 1 = transparent background */
void lcd_show_char(uint16_t x, uint16_t y, uint8_t ch, uint16_t fc, uint16_t bc,
                   uint8_t size, uint8_t mode)
{
    const unsigned char *font;
    uint8_t sizex, t;
    uint16_t i, type_num;
    uint8_t m = 0;
    uint16_t x0 = x;

    font = lcd_font_get(size, ch, &sizex);

    if (font == 0)
    {
        return;
    }

    type_num = ((sizex / 8) + ((sizex % 8) ? 1 : 0)) * size;    /* bitmap bytes per char */
    lcd_set_window(x, y, x + sizex - 1, y + size - 1);

    for (i = 0; i < type_num; i++)
    {
        for (t = 0; t < 8; t++)
        {
            if (!mode)
            {
                /* background mode: write every pixel including background */
                lcd_dev_write_data16((font[i] & (0x01 << t)) ? fc : bc);
                m++;

                if (m % sizex == 0)
                {
                    m = 0;
                    break;
                }
            }
            else
            {
                /* transparent mode: draw only foreground pixels */
                if (font[i] & (0x01 << t))
                {
                    lcd_draw_point(x, y, fc);
                }

                x++;

                if ((x - x0) == sizex)
                {
                    x = x0;
                    y++;
                    break;
                }
            }
        }
    }
}

/* Show a null-terminated ASCII string, cursor advances by one char width */
void lcd_show_string(uint16_t x, uint16_t y, const char *str, uint16_t fc, uint16_t bc,
                     uint8_t size, uint8_t mode)
{
    uint8_t sizex = size / 2;

    while (*str != '\0')
    {
        lcd_show_char(x, y, (uint8_t)*str, fc, bc, size, mode);
        x += sizex;
        str++;
    }
}

/*******************************************************************************
 * Numeric display
 ******************************************************************************/
/* 10^n helper without floating point */
static uint32_t lcd_pow10(uint8_t n)
{
    uint32_t r = 1;

    while (n--)
    {
        r *= 10;
    }

    return r;
}

/* Fixed-length integer display, leading spaces for shorter values; '-' takes one digit */
void lcd_show_int(uint16_t x, uint16_t y, int32_t num, uint8_t len, uint16_t fc,
                  uint16_t bc, uint8_t size)
{
    uint8_t t, temp, sizex;
    uint8_t enshow = 0;
    uint32_t unum;
    uint16_t cx = x;

    sizex = size / 2;

    if (len == 0)
    {
        return;
    }

    if (num < 0)
    {
        lcd_show_char(x, y, '-', fc, bc, size, 0);
        cx += sizex;
        unum = (uint32_t)(-num);
        len--;

        if (len == 0)
        {
            return;
        }
    }
    else
    {
        unum = (uint32_t)num;
    }

    for (t = 0; t < len; t++)
    {
        temp = (uint8_t)((unum / lcd_pow10(len - t - 1)) % 10);

        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                lcd_show_char(cx + t * sizex, y, ' ', fc, bc, size, 0);
                continue;
            }
            else
            {
                enshow = 1;
            }
        }

        lcd_show_char(cx + t * sizex, y, temp + '0', fc, bc, size, 0);
    }
}

/* Fixed-point display: int_len integer digits + dec_len decimal digits, negative supported */
void lcd_show_float(uint16_t x, uint16_t y, float num, uint8_t int_len, uint8_t dec_len,
                    uint16_t fc, uint16_t bc, uint8_t size)
{
    uint8_t t, temp, sizex, display_pos = 0;
    uint8_t is_negative = 0;
    uint32_t multiplier = 1;
    int32_t num1;

    sizex = size / 2;

    if (num < 0)
    {
        is_negative = 1;
        num = -num;
    }

    for (t = 0; t < dec_len; t++)
    {
        multiplier *= 10;
    }

    num1 = (int32_t)(num * multiplier + 0.5f);  /* round half up */

    if (is_negative)
    {
        lcd_show_char(x, y, '-', fc, bc, size, 0);
        display_pos++;
    }

    for (t = 0; t < int_len; t++)
    {
        temp = (uint8_t)((num1 / (int32_t)lcd_pow10(int_len + dec_len - t - 1)) % 10);
        lcd_show_char(x + (display_pos + t) * sizex, y, temp + '0', fc, bc, size, 0);
    }

    if (dec_len > 0)
    {
        lcd_show_char(x + (display_pos + int_len) * sizex, y, '.', fc, bc, size, 0);

        for (t = 0; t < dec_len; t++)
        {
            temp = (uint8_t)((num1 / (int32_t)lcd_pow10(dec_len - t - 1)) % 10);
            lcd_show_char(x + (display_pos + int_len + 1 + t) * sizex, y, temp + '0',
                          fc, bc, size, 0);
        }
    }
}

/*******************************************************************************
 * External flash RGB565 picture
 *  - data is row-major big-endian RGB565, the byte stream is written
 *    to the screen directly
 *  - read_fn is injected by the caller (e.g. spiflash_read of w25qxxx)
 ******************************************************************************/
#define LCD_PIC_CHUNK   512U    /* bytes read from flash per iteration */

void lcd_show_flash_picture(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
                            uint32_t flash_addr, lcd_flash_read_fn read_fn)
{
    static uint8_t chunk[LCD_PIC_CHUNK];
    uint32_t total, once;
    uint32_t addr = flash_addr;

    if (read_fn == 0)
    {
        return;
    }

    lcd_set_window(x, y, x + width - 1, y + height - 1);
    total = (uint32_t)width * height * 2;
    lcd_port_cs(0);

    while (total)
    {
        once = (total > LCD_PIC_CHUNK) ? LCD_PIC_CHUNK : total;
        read_fn(chunk, addr, once);
        lcd_port_send_bulk(chunk, once);
        addr += once;
        total -= once;
    }

    lcd_port_cs(1);
}
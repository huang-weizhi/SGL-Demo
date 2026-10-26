#include "Inf_lcd.h"
#include "Inf_lcd_font.h"
#include "Inf_lcd_font_num.h"
#include "Inf_lcd_picture.h"

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
 * Area fill (screen refresh) - mirrors the reference driver
 *  - set the window, assert CS once, then repeat the same 16-bit color via
 *    half-word DMA, release CS at the end
 ******************************************************************************/
void lcd_fill(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint32_t total;

    lcd_set_window(x1, y1, x2, y2);
    total = (uint32_t)(x2 - x1 + 1) * (y2 - y1 + 1);
    lcd_port_cs(0);                      /* keep CS asserted for the whole fill */
    lcd_port_send_halfwords(&color, total);
    lcd_port_cs(1);
}

/* Clear the whole screen with one color */
void lcd_clear(uint16_t color)
{
    lcd_fill(0, 0, LCD_W - 1, LCD_H - 1, color);
}

/*******************************************************************************
 * Atomic block blit for double-buffered widgets (waveform, gauges, ...)
 *  - lcd_blit_start(): set the window, assert CS, kick the byte DMA and
 *    return immediately; the caller prepares the next block meanwhile
 *  - lcd_blit_wait(): block until the transfer finished, release CS
 *  - pixels: raw big-endian RGB565 byte stream, row-major, w*h*2 bytes
 ******************************************************************************/
void lcd_blit_start(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                    const uint8_t *pixels)
{
    lcd_port_send_bulk_wait();              /* finish a previous transfer */
    lcd_set_window(x, y, x + w - 1, y + h - 1);
    lcd_port_cs(0);
    lcd_port_send_bulk_start(pixels, (uint32_t)w * h * 2);
}

void lcd_blit_wait(void)
{
    lcd_port_send_bulk_wait();
    lcd_port_cs(1);
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
/* Map a character to its index in the compact digit-only tables of
 * Inf_lcd_font_num.h, or 0xFF when the glyph is not kept there.
 * Compact layout: 0 = space, 1..10 = '0'..'9', 11 = '-', 12 = '.' */
static uint8_t lcd_numfont_index(uint8_t ch)
{
    if ((ch >= '0') && (ch <= '9'))
    {
        return (uint8_t)(1 + (ch - '0'));
    }
    if (ch == '-')
    {
        return 11;
    }
    if (ch == '.')
    {
        return 12;
    }
    if (ch == ' ')
    {
        return 0;
    }
    return 0xFF;
}

/* Get the bitmap of one char for the given size, width in pixels, 0 if unsupported size */
static const unsigned char *lcd_font_get(uint8_t size, uint8_t ch, uint8_t *width)
{
    uint8_t ni;

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
		case 40:
			*width = 20;
            return ascii_4020[ch];
        case 56:                                    /* 28x56, 224 bytes/char */
            ni = lcd_numfont_index((uint8_t)(ch + ' '));
            if (ni == 0xFF)
            {
                break;  /* not in the compact table: fall back to the 12 px font */
            }
            *width = 28;
            return ascii_5628_num[ni];
		case 72:
            ni = lcd_numfont_index((uint8_t)(ch + ' '));
            if (ni == 0xFF)
            {
                break;
            }
			*width = 36;
            return ascii_7236_num[ni];
        default:
            break;
    }

    *width = 6;
    return ascii_1206[ch < 95U ? ch : 0U];
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

/* Fixed-length integer display: always len digit slots, a negative value
 * uses '-' as one of the slots (the rest of the digits follow).
 *   zero_fill = 1 : pad shorter values with leading zeros, e.g. len 3 and
 *                   value 0 -> "000", value -1 -> "-01"
 *   zero_fill = 0 : pad with spaces so the value stays right aligned,
 *                   e.g. len 3 and value 0 -> "  0"
 * len must stay <= 10 (32-bit arithmetic limits the number of digits). */
void lcd_show_int(uint16_t x, uint16_t y, int32_t num, uint8_t len, uint8_t zero_fill,
                  uint16_t fc, uint16_t bc, uint8_t size)
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

        if (zero_fill == 0 && enshow == 0 && t < (len - 1))
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

/* Show a picture by its id in the resource table (Inf_lcd_picture.h) */
void lcd_show_picture(uint16_t x, uint16_t y, uint16_t id, lcd_flash_read_fn read_fn)
{
    if (id >= LCD_PICTURE_NUM || read_fn == 0)
    {
        return;
    }

    lcd_show_flash_picture(x, y, lcd_picture_table[id].width,
                           lcd_picture_table[id].height,
                           lcd_picture_table[id].addr, read_fn);
}
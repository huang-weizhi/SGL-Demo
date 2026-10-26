/*=============================================================================
 *                              Header Includes
 *============================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "at32f415.h"
#include "at32f415_wk_config.h"
#include "wk_system.h"
#include "lcd.h"
#include "lcd_font.h"
#include "HA01_HANDLE.h"
#include "at32_spiflash.h"

/*=============================================================================
 *                            Static Variables
 *============================================================================*/
static dma_init_type dma_init_struct;

/*=============================================================================
 *                      Static Helper Functions (DMA Configuration)
 *============================================================================*/

/**
 * @brief Configure DMA channel for SPI transfer (common setup)
 * @param channel DMA channel to configure
 * @param dir Transfer direction (memory to peripheral or peripheral to memory)
 * @param addr Memory address (source or destination)
 * @param inc Memory increment enable flag
 * @param width Data width (byte/halfword)
 * @param length Number of transfers
 */
static void spi_dma_config(dma_channel_type *channel, uint8_t dir, uint32_t addr,
                           uint8_t inc, uint8_t width, uint32_t length)
{
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.buffer_size = length;
    dma_init_struct.direction = dir;
    dma_init_struct.memory_base_addr = addr;
    dma_init_struct.memory_data_width = width;
    dma_init_struct.memory_inc_enable = inc;
    dma_init_struct.peripheral_base_addr = (uint32_t)(&LCD_SPI->dt);
    dma_init_struct.peripheral_data_width = width;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority = LCD_SPI_DMA_PRIORITY;
    dma_init_struct.loop_mode_enable = FALSE;
    dma_init(channel, &dma_init_struct);
}

/*=============================================================================
 *                         SPI DMA Transfer Functions
 *============================================================================*/

/**
 * @brief Send/receive bytes over SPI using DMA (8-bit mode)
 * @param pbuffer Pointer to data buffer to send
 * @param length Number of bytes to transfer
 */
void spi_dma_write_bytes(uint8_t *pbuffer, uint32_t length)
{
    volatile uint16_t dummy_data;
    /* Reset DMA channels */
    dma_reset(SPI_DMA_RX_CHANNEL);
    dma_reset(SPI_DMA_TX_CHANNEL);
    /* Configure MISO (receive) channel - discard received data */
    spi_dma_config(SPI_DMA_RX_CHANNEL, DMA_DIR_PERIPHERAL_TO_MEMORY,
                   (uint32_t)&dummy_data, FALSE, DMA_MEMORY_DATA_WIDTH_BYTE, length);
    /* Configure MOSI (transmit) channel */
    spi_dma_config(SPI_DMA_TX_CHANNEL, DMA_DIR_MEMORY_TO_PERIPHERAL,
                   (uint32_t)pbuffer, TRUE, DMA_MEMORY_DATA_WIDTH_BYTE, length);
    /* Enable SPI DMA requests */
    spi_i2s_dma_transmitter_enable(LCD_SPI, TRUE);
    spi_i2s_dma_receiver_enable(LCD_SPI, TRUE);
    /* Start DMA channels */
    dma_channel_enable(SPI_DMA_RX_CHANNEL, TRUE);
    dma_channel_enable(SPI_DMA_TX_CHANNEL, TRUE);

    /* Wait for receive complete (implies transmit complete) */
    while (dma_flag_get(SPI_DMA_RX_FLAG) == RESET);

    dma_flag_clear(SPI_DMA_RX_FLAG);
    /* Stop DMA channels */
    dma_channel_enable(SPI_DMA_RX_CHANNEL, FALSE);
    dma_channel_enable(SPI_DMA_TX_CHANNEL, FALSE);
    /* Disable SPI DMA requests */
    spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
    spi_i2s_dma_receiver_enable(LCD_SPI, FALSE);
}

/**
 * @brief Send/receive half-words over SPI using DMA (16-bit mode)
 * @note memory_inc_enable is FALSE because we usually send the same color value repeatedly.
 *       For different data, set memory_inc_enable to TRUE in caller.
 * @param pbuffer Pointer to data buffer (16-bit aligned)
 * @param length Number of half-words to transfer
 */
void spi_dma_write_halfwords(uint16_t *pbuffer, uint32_t length)
{
    volatile uint16_t dummy_data;
    /* Switch SPI to 16-bit frame */
    spi_frame_bit_num_set(LCD_SPI, SPI_FRAME_16BIT);
    dma_reset(SPI_DMA_RX_CHANNEL);
    dma_reset(SPI_DMA_TX_CHANNEL);
    /* Configure receive channel (discard) */
    spi_dma_config(SPI_DMA_RX_CHANNEL, DMA_DIR_PERIPHERAL_TO_MEMORY,
                   (uint32_t)&dummy_data, FALSE, DMA_MEMORY_DATA_WIDTH_HALFWORD, length);
    /* Configure transmit channel (no memory increment for constant color fill) */
    spi_dma_config(SPI_DMA_TX_CHANNEL, DMA_DIR_MEMORY_TO_PERIPHERAL,
                   (uint32_t)pbuffer, FALSE, DMA_MEMORY_DATA_WIDTH_HALFWORD, length);
    spi_i2s_dma_transmitter_enable(LCD_SPI, TRUE);
    spi_i2s_dma_receiver_enable(LCD_SPI, TRUE);
    dma_channel_enable(SPI_DMA_RX_CHANNEL, TRUE);
    dma_channel_enable(SPI_DMA_TX_CHANNEL, TRUE);

    while (dma_flag_get(SPI_DMA_RX_FLAG) == RESET);

    dma_flag_clear(SPI_DMA_RX_FLAG);
    dma_channel_enable(SPI_DMA_RX_CHANNEL, FALSE);
    dma_channel_enable(SPI_DMA_TX_CHANNEL, FALSE);
    spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
    spi_i2s_dma_receiver_enable(LCD_SPI, FALSE);
    /* Restore 8-bit frame mode */
    spi_frame_bit_num_set(LCD_SPI, SPI_FRAME_8BIT);
}

/**
 * @brief Write one byte over SPI and read the response
 * @param data Byte to send
 * @return Received byte, or 0 on timeout
 */
uint8_t spi_write_read_byte(uint8_t data)
{
    uint8_t retry = 0;
    /* Disable DMA for this transaction */
    spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
    spi_i2s_dma_receiver_enable(LCD_SPI, FALSE);

    /* Wait for TX buffer empty */
    while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_TDBE_FLAG) == RESET)
    {
        if (++retry > SPI_TIMEOUT_MS)
        {
            return 0;
        }
    }

    spi_i2s_data_transmit(LCD_SPI, data);
    /* Wait for RX buffer not empty */
    retry = 0;

    while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_RDBF_FLAG) == RESET)
    {
        if (++retry > SPI_TIMEOUT_MS)
        {
            return 0;
        }
    }

    return spi_i2s_data_receive(LCD_SPI);
}

/*=============================================================================
 *                    LCD Low-Level Communication Functions
 *============================================================================*/

/**
 * @brief Write one byte over SPI bus with CS control
 * @param dat Byte to send
 */
void LCD_Write_Bus(uint8_t dat)
{
    LCD_CS_Clr();
    spi_write_read_byte(dat);
    LCD_CS_Set();
}

/**
 * @brief Write 8-bit data (data mode)
 * @param dat 8-bit data
 */
void LCD_WR_DATA8(uint8_t dat)
{
    LCD_Write_Bus(dat);
}

/**
 * @brief Write 16-bit data (data mode, big-endian order)
 * @param dat 16-bit data
 */
void LCD_WR_DATA(uint16_t dat)
{
    LCD_DC_Set();
    LCD_Write_Bus(dat >> 8);
    LCD_Write_Bus(dat);
}

/**
 * @brief Write 8-bit command (command mode)
 * @param dat Command byte
 */
void LCD_WR_REG(uint8_t dat)
{
    LCD_DC_Clr();
    LCD_Write_Bus(dat);
    LCD_DC_Set();
}

/**
 * @brief Set LCD display window (address range)
 * @param x1 Start column
 * @param y1 Start row
 * @param x2 End column
 * @param y2 End row
 */
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    int16_t x_offset = 0;
    int16_t y_offset = 0;

    /* Apply offset for specific orientations */
    if (USE_HORIZONTAL == 1)
    {
        y_offset = 80;
    }
    else if (USE_HORIZONTAL == 3)
    {
        x_offset = 80;
    }

    LCD_WR_REG(0x2A);                       // Column address set
    LCD_WR_DATA(x1 + x_offset);
    LCD_WR_DATA(x2 + x_offset);
    LCD_WR_REG(0x2B);                       // Row address set
    LCD_WR_DATA(y1 + y_offset);
    LCD_WR_DATA(y2 + y_offset);
    LCD_WR_REG(0x2C);                       // Memory write
}

/**
 * @brief LCD initialization sequence
 */
void LCD_Init(void)
{
    /* Hardware reset */
    LCD_RES_Clr();
    wk_delay_ms(200);
    LCD_RES_Set();
    wk_delay_ms(120);
    /* Exit sleep mode */
    LCD_WR_REG(0x11);
    wk_delay_ms(120);
    /* Turn off display to avoid white flash */
    LCD_WR_REG(0x28);
    wk_delay_ms(20);
    /* Set memory access control (orientation) */
    LCD_SET_DIRECTION(LCD_CURRENT_DIR);
    /* Set pixel format to 16-bit (RGB565) */
    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);
    /* Register configuration (refer to ST7789 datasheet) */
    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);
    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);
    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x19);
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x0D);     // GVDD = 4.2V
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);
    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F);     // Frame rate = 60Hz
    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);
    /* Positive voltage gamma correction */
    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x0F);
    LCD_WR_DATA8(0x10);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x34);
    LCD_WR_DATA8(0x50);
    LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x3A);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x06);
    LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x20);
    /* Negative voltage gamma correction */
    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x0A);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x16);
    LCD_WR_DATA8(0x32);
    LCD_WR_DATA8(0x40);
    LCD_WR_DATA8(0x4A);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x1B);
    LCD_WR_DATA8(0x1C);
    LCD_WR_DATA8(0x22);
    LCD_WR_DATA8(0x1F);
    /* Display inversion (some panels require 0x21 or 0x20) */
    LCD_WR_REG(0x21);
    /* Clear screen to black to avoid random content */
    LCD_Clear(BLACK);
    /* Turn on display */
    LCD_WR_REG(0x29);
}

/*=============================================================================
 *                         High-Level Drawing Functions
 *============================================================================*/

/**
 * @brief Fill a rectangular area with a solid color
 * @param xsta Start X coordinate
 * @param ysta Start Y coordinate
 * @param xend End X coordinate
 * @param yend End Y coordinate
 * @param color Fill color
 */
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint16_t color_buf[1] = {color};
    uint32_t total_pixels = (xend - xsta) * (yend - ysta);
    uint32_t chunk;
    uint8_t first = 1;
    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1);
    LCD_CS_Clr();

    /* Split large transfers into chunks (max 65534 half-words) */
    while (first || total_pixels > 0)
    {
        if (total_pixels > 65534)
        {
            chunk = 65534;
            total_pixels -= 65534;
        }
        else
        {
            chunk = total_pixels;
            total_pixels = 0;
        }

        if (first)
        {
            first = 0;
        }

        spi_dma_write_halfwords(color_buf, chunk);
    }

    LCD_CS_Set();
}

/**
 * @brief Clear entire screen with a single color
 * @param color Fill color
 */
void LCD_Clear(uint16_t color)
{
    LCD_Fill(0, 0, LCD_W, LCD_H, color);
}

/**
 * @brief Draw a single pixel
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Pixel color
 */
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)
{
    LCD_Address_Set(x, y, x, y);
    LCD_WR_DATA(color);
}

/**
 * @brief Draw a line using Bresenham algorithm
 * @param x1 Start X
 * @param y1 Start Y
 * @param x2 End X
 * @param y2 End Y
 * @param color Line color
 */
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int16_t dx = x2 - x1;
    int16_t dy = y2 - y1;
    int16_t x = x1, y = y1;
    int16_t incx, incy;
    int16_t distance, xerr = 0, yerr = 0;

    /* Determine step direction */
    if (dx > 0)
    {
        incx = 1;
    }
    else if (dx == 0)
    {
        incx = 0;
    }
    else
    {
        incx = -1;
        dx = -dx;
    }

    if (dy > 0)
    {
        incy = 1;
    }
    else if (dy == 0)
    {
        incy = 0;
    }
    else
    {
        incy = -1;
        dy = -dy;
    }

    distance = (dx > dy) ? dx : dy;

    for (uint16_t t = 0; t <= distance; t++)
    {
        LCD_DrawPoint(x, y, color);
        xerr += dx;
        yerr += dy;

        if (xerr > distance)
        {
            xerr -= distance;
            x += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            y += incy;
        }
    }
}

/**
 * @brief Draw a rectangle outline
 * @param x1 Top-left X
 * @param y1 Top-left Y
 * @param x2 Bottom-right X
 * @param y2 Bottom-right Y
 * @param color Rectangle color
 */
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    LCD_DrawLine(x1, y1, x2, y1, color);
    LCD_DrawLine(x1, y1, x1, y2, color);
    LCD_DrawLine(x1, y2, x2, y2, color);
    LCD_DrawLine(x2, y1, x2, y2, color);
}

/**
 * @brief Draw a circle using midpoint algorithm
 * @param x0 Center X
 * @param y0 Center Y
 * @param r Radius
 * @param color Circle color
 */
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int16_t a = 0, b = r;

    while (a <= b)
    {
        LCD_DrawPoint(x0 - b, y0 - a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        a++;

        if ((a * a + b * b) > (r * r))
        {
            b--;
        }
    }
}

/*=============================================================================
 *                         Chinese Character Display
 *============================================================================*/

/* The following functions remain functionally identical to the original.
   Only comments have been translated to English. For brevity, only the
   first function is shown with full comments; others follow the same pattern. */

void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    while (*s != 0)
    {
        if (sizey == 12)
        {
            LCD_ShowChinese12x12(x, y, s, fc, bc, sizey, mode);
        }
        else if (sizey == 16)
        {
            LCD_ShowChinese16x16(x, y, s, fc, bc, sizey, mode);
        }
        else if (sizey == 24)
        {
            LCD_ShowChinese24x24(x, y, s, fc, bc, sizey, mode);
        }
        else if (sizey == 32)
        {
            LCD_ShowChinese32x32(x, y, s, fc, bc, sizey, mode);
        }
        else
        {
            return;
        }

        s += 2;
        x += sizey;
    }
}

void LCD_ShowChinese12x12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j, m = 0;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont12) / sizeof(typFNT_GB12);
    uint16_t TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
    uint16_t x0 = x;

    for (k = 0; k < HZnum; k++)
    {
        if ((tfont12[k].Index[0] == *(s)) && (tfont12[k].Index[1] == *(s + 1)))
        {
            LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);

            for (i = 0; i < TypefaceNum; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if (!mode)   // Block fill mode
                    {
                        if (tfont12[k].Msk[i] & (0x01 << j))
                        {
                            LCD_WR_DATA(fc);
                        }
                        else
                        {
                            LCD_WR_DATA(bc);
                        }

                        m++;

                        if (m % sizey == 0)
                        {
                            m = 0;
                            break;
                        }
                    }
                    else         // Point-by-point mode
                    {
                        if (tfont12[k].Msk[i] & (0x01 << j))
                        {
                            LCD_DrawPoint(x, y, fc);
                        }

                        x++;

                        if ((x - x0) == sizey)
                        {
                            x = x0;
                            y++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void LCD_ShowChinese16x16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j, m = 0;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont16) / sizeof(typFNT_GB16);
    uint16_t TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
    uint16_t x0 = x;

    for (k = 0; k < HZnum; k++)
    {
        if ((tfont16[k].Index[0] == *(s)) && (tfont16[k].Index[1] == *(s + 1)))
        {
            LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);

            for (i = 0; i < TypefaceNum; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if (!mode)
                    {
                        if (tfont16[k].Msk[i] & (0x01 << j))
                        {
                            LCD_WR_DATA(fc);
                        }
                        else
                        {
                            LCD_WR_DATA(bc);
                        }

                        m++;

                        if (m % sizey == 0)
                        {
                            m = 0;
                            break;
                        }
                    }
                    else
                    {
                        if (tfont16[k].Msk[i] & (0x01 << j))
                        {
                            LCD_DrawPoint(x, y, fc);
                        }

                        x++;

                        if ((x - x0) == sizey)
                        {
                            x = x0;
                            y++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void LCD_ShowChinese24x24(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j, m = 0;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont24) / sizeof(typFNT_GB24);
    uint16_t TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
    uint16_t x0 = x;

    for (k = 0; k < HZnum; k++)
    {
        if ((tfont24[k].Index[0] == *(s)) && (tfont24[k].Index[1] == *(s + 1)))
        {
            LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);

            for (i = 0; i < TypefaceNum; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if (!mode)
                    {
                        if (tfont24[k].Msk[i] & (0x01 << j))
                        {
                            LCD_WR_DATA(fc);
                        }
                        else
                        {
                            LCD_WR_DATA(bc);
                        }

                        m++;

                        if (m % sizey == 0)
                        {
                            m = 0;
                            break;
                        }
                    }
                    else
                    {
                        if (tfont24[k].Msk[i] & (0x01 << j))
                        {
                            LCD_DrawPoint(x, y, fc);
                        }

                        x++;

                        if ((x - x0) == sizey)
                        {
                            x = x0;
                            y++;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void LCD_ShowChinese32x32(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t i, j, m = 0;
    uint16_t k;
    uint16_t HZnum = sizeof(tfont32) / sizeof(typFNT_GB32);
    uint16_t TypefaceNum = (sizey / 8 + ((sizey % 8) ? 1 : 0)) * sizey;
    uint16_t x0 = x;

    for (k = 0; k < HZnum; k++)
    {
        if ((tfont32[k].Index[0] == *(s)) && (tfont32[k].Index[1] == *(s + 1)))
        {
            LCD_Address_Set(x, y, x + sizey - 1, y + sizey - 1);

            for (i = 0; i < TypefaceNum; i++)
            {
                for (j = 0; j < 8; j++)
                {
                    if (!mode)
                    {
                        if (tfont32[k].Msk[i] & (0x01 << j))
                        {
                            LCD_WR_DATA(fc);
                        }
                        else
                        {
                            LCD_WR_DATA(bc);
                        }

                        m++;

                        if (m % sizey == 0)
                        {
                            m = 0;
                            break;
                        }
                    }
                    else
                    {
                        if (tfont32[k].Msk[i] & (0x01 << j))
                        {
                            LCD_DrawPoint(x, y, fc);
                        }

                        x++;

                        if ((x - x0) == sizey)
                        {
                            x = x0;
                            y++;
                            break;
                        }
                    }
                }
            }
        }
    }
}



/*=============================================================================
 *                         ASCII Character Display
 *============================================================================*/

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t temp, sizex = sizey / 2, t, m = 0;
    uint16_t i, TypefaceNum = (sizex / 8 + ((sizex % 8) ? 1 : 0)) * sizey;
    uint16_t x0 = x;
    num = num - ' ';
    LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);

    for (i = 0; i < TypefaceNum; i++)
    {
        /* Select font table based on size */
        if (sizey == 12)
        {
            temp = ascii_1206[num][i];
        }
        else if (sizey == 16)
        {
            temp = ascii_1608[num][i];
        }
        else if (sizey == 24)
        {
            temp = ascii_2412[num][i];
        }
        else if (sizey == 32)
        {
            temp = ascii_3216[num][i];
        }
 
		else if (sizey == 72)
        {
            temp = ascii_7236[num][i];
        }
        else
        {
            return;
        }

        for (t = 0; t < 8; t++)
        {
            if (!mode)
            {
                if (temp & (0x01 << t))
                {
                    LCD_WR_DATA(fc);
                }
                else
                {
                    LCD_WR_DATA(bc);
                }

                m++;

                if (m % sizex == 0)
                {
                    m = 0;
                    break;
                }
            }
            else
            {
                if (temp & (0x01 << t))
                {
                    LCD_DrawPoint(x, y, fc);
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

void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    while (*p != '\0')
    {
        LCD_ShowChar(x, y, *p, fc, bc, sizey, mode);
        x += sizey / 2;
        p++;
    }
}

/*=============================================================================
 *                         Numeric Display Utilities
 *============================================================================*/



/**
 * @brief Integer power function
 */
uint32_t mypow(uint8_t m, uint8_t n)
{
    uint32_t result = 1;

    while (n--)
    {
        result *= m;
    }

    return result;
}

/**
 * @brief Display integer with fixed length (leading zeros replaced by spaces)
 */
void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint8_t t, temp;
    uint8_t sizex = sizey / 2;

    for (t = 0; t < len; t++)
    {
        temp = (num / mypow(10, len - t - 1)) % 10;
        LCD_ShowChar(x + t * sizex, y, temp + '0', fc, bc, sizey, 0);
    }
}


/*=============================================================================
 *                         Image Display Functions
 *============================================================================*/

/**
 * @brief Display image from memory buffer
 */
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[])
{
    uint32_t total_bytes = length * width * 2;
    uint32_t chunk;
    uint8_t first = 1;
    LCD_Address_Set(x, y, x + length - 1, y + width - 1);
    LCD_CS_Clr();

    while (first || total_bytes > 0)
    {
        if (total_bytes > 65534)
        {
            chunk = 65534;
            total_bytes -= 65534;
        }
        else
        {
            chunk = total_bytes;
            total_bytes = 0;
        }

        if (first)
        {
            first = 0;
        }

        spi_dma_write_bytes((uint8_t *)pic, chunk);
        pic += chunk;
    }

    LCD_CS_Set();
}

/**
 * @brief Picture address table for external flash
 */
const PicInfo g_picInfo[] =
{
    /*   0 */ {0x00000000, 222, 39},   // PIC_LOGO
    /*   1 */ {0x000043A4, 320, 217},  // PIC_MAIN_INTERFACE
    /*   2 */ {0x00026224, 320, 217},  // PIC_CURVE_SCREEN
    /*   3 */ {0x000480A4, 144, 32},   // PIC_MENU_SELECTED_WORK_MODE_CN
    /*   4 */ {0x0004A4A4, 144, 32},   // PIC_MENU_UNSELECTED_WORK_MODE_CN
    /*   5 */ {0x0004C8A4, 144, 32},   // PIC_MENU_SELECTED_WORK_MODE_EN
    /*   6 */ {0x0004ECA4, 144, 32},   // PIC_MENU_UNSELECTED_WORK_MODE_EN
    /*   7 */ {0x000510A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_UNIT_CN
    /*   8 */ {0x000534A4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_UNIT_CN
    /*   9 */ {0x000558A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_UNIT_EN
    /*  10 */ {0x00057CA4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_UNIT_EN
    /*  11 */ {0x0005A0A4, 144, 32},   // PIC_MENU_SELECTED_SOUND_SETTING_CN
    /*  12 */ {0x0005C4A4, 144, 32},   // PIC_MENU_UNSELECTED_SOUND_SETTING_CN
    /*  13 */ {0x0005E8A4, 144, 32},   // PIC_MENU_SELECTED_SOUND_SETTING_EN
    /*  14 */ {0x00060CA4, 144, 32},   // PIC_MENU_UNSELECTED_SOUND_SETTING_EN
    /*  15 */ {0x000630A4, 144, 32},   // PIC_MENU_SELECTED_LANGUAGE_SETTING_CN
    /*  16 */ {0x000654A4, 144, 32},   // PIC_MENU_UNSELECTED_LANGUAGE_SETTING_CN
    /*  17 */ {0x000678A4, 144, 32},   // PIC_MENU_SELECTED_LANGUAGE_SETTING_EN
    /*  18 */ {0x00069CA4, 144, 32},   // PIC_MENU_UNSELECTED_LANGUAGE_SETTING_EN
    /*  19 */ {0x0006C0A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_CALIB_CN
    /*  20 */ {0x0006E4A4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_CALIB_CN
    /*  21 */ {0x000708A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_CALIB_EN
    /*  22 */ {0x00072CA4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_CALIB_EN
    /*  23 */ {0x000750A4, 144, 32},   // PIC_MENU_SELECTED_SLEEP_SETTING_CN
    /*  24 */ {0x000774A4, 144, 32},   // PIC_MENU_UNSELECTED_SLEEP_SETTING_CN
    /*  25 */ {0x000798A4, 144, 32},   // PIC_MENU_SELECTED_SLEEP_SETTING_EN
    /*  26 */ {0x0007BCA4, 144, 32},   // PIC_MENU_UNSELECTED_SLEEP_SETTING_EN
    /*  27 */ {0x0007E0A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_DISPLAY_CN
    /*  28 */ {0x000804A4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_DISPLAY_CN
    /*  29 */ {0x000828A4, 144, 32},   // PIC_MENU_SELECTED_TEMP_DISPLAY_EN
    /*  30 */ {0x00084CA4, 144, 32},   // PIC_MENU_UNSELECTED_TEMP_DISPLAY_EN
    /*  31 */ {0x000870A4, 144, 32},   // PIC_MENU_SELECTED_VERSION_CN
    /*  32 */ {0x000894A4, 144, 32},   // PIC_MENU_UNSELECTED_VERSION_CN
    /*  33 */ {0x0008B8A4, 144, 32},   // PIC_MENU_SELECTED_VERSION_EN
    /*  34 */ {0x0008DCA4, 144, 32},   // PIC_MENU_UNSELECTED_VERSION_EN
    /*  35 */ {0x000900A4, 144, 32},   // PIC_MENU_SELECTED_FACTORY_RESET_CN
    /*  36 */ {0x000924A4, 144, 32},   // PIC_MENU_UNSELECTED_FACTORY_RESET_CN
    /*  37 */ {0x000948A4, 144, 32},   // PIC_MENU_SELECTED_FACTORY_RESET_EN
    /*  38 */ {0x00096CA4, 144, 32},   // PIC_MENU_UNSELECTED_FACTORY_RESET_EN
    /*  39 */ {0x000990A4, 144, 32},   // PIC_MENU_SELECTED_TECH_SUPPORT_CN
    /*  40 */ {0x0009B4A4, 144, 32},   // PIC_MENU_UNSELECTED_TECH_SUPPORT_CN
    /*  41 */ {0x0009D8A4, 144, 32},   // PIC_MENU_SELECTED_TECH_SUPPORT_EN
    /*  42 */ {0x0009FCA4, 144, 32},   // PIC_MENU_UNSELECTED_TECH_SUPPORT_EN
    /*  43 */ {0x000A20A4, 239, 122},   // PIC_SETTING_WORK_MODE_CN
    /*  44 */ {0x000B0470, 239, 122},   // PIC_SETTING_WORK_MODE_EN
    /*  45 */ {0x000BE83C, 239, 122},   // PIC_SETTING_TEMP_UNIT_CN
    /*  46 */ {0x000CCC08, 239, 122},   // PIC_SETTING_TEMP_UNIT_EN
    /*  47 */ {0x000DAFD4, 239, 122},   // PIC_SETTING_SOUND_SETTING_CN
    /*  48 */ {0x000E93A0, 239, 122},   // PIC_SETTING_SOUND_SETTING_EN
    /*  49 */ {0x000F776C, 240, 93},    // PIC_SETTING_CN
    /*  50 */ {0x001025CC, 240, 93},    // PIC_SETTING_EN
    /*  51 */ {0x0010D42C, 239, 122},   // PIC_SETTING_TEMP_CALIB_CN
    /*  52 */ {0x0011B7F8, 239, 122},   // PIC_SETTING_TEMP_CALIB_EN
    /*  53 */ {0x00129BC4, 239, 122},   // PIC_SETTING_SLEEP_MODE_CN
    /*  54 */ {0x00137F90, 239, 122},   // PIC_SETTING_SLEEP_MODE_EN
    /*  55 */ {0x0014635C, 239, 122},   // PIC_SETTING_TEMP_DISPLAY_CN
    /*  56 */ {0x00154728, 239, 122},   // PIC_SETTING_TEMP_DISPLAY_EN
    /*  57 */ {0x00162AF4, 239, 134},   // PIC_SETTING_VERSION_CN
    /*  58 */ {0x00172528, 239, 134},   // PIC_SETTING_VERSION_EN
    /*  59 */ {0x00181F5C, 239, 122},   // PIC_SETTING_UNSELECT_FACTORY_RESET_CN
    /*  60 */ {0x00190328, 239, 122},   // PIC_SETTING_UNSELECT_FACTORY_RESET_EN
    /*  61 */ {0x0019E6F4, 239, 122},   // PIC_SETTING_SELECT_FACTORY_RESET_CN
    /*  62 */ {0x001ACAC0, 239, 122},   // PIC_SETTING_SELECT_FACTORY_RESET_EN
    /*  63 */ {0x001BAE8C, 239, 122},   // PIC_SETTING_FACTORY_RESET_DONE_CN
    /*  64 */ {0x001C9258, 239, 122},   // PIC_SETTING_FACTORY_RESET_DONE_EN
    /*  65 */ {0x001D7624, 310, 98},   // PIC_SETTING_TECH_HA01_SUPPORT_CN
    /*  66 */ {0x001E637C, 310, 98},   // PIC_SETTING_TECH_HA01_SUPPORT_EN
    /*  67 */ {0x001F50D4, 310, 98},   // PIC_SETTING_TECH_HA02_SUPPORT_CN
    /*  68 */ {0x00203E2C, 310, 98},   // PIC_SETTING_TECH_HA02_SUPPORT_EN
    /*  69 */ {0x00212B84, 310, 98},   // PIC_SETTING_TECH_HA03_SUPPORT_CN
    /*  70 */ {0x002218DC, 310, 98},   // PIC_SETTING_TECH_HA03_SUPPORT_EN
    /*  71 */ {0x00230634, 66, 35},   // PIC_SELECT_RETURN_ICON_CN
    /*  72 */ {0x00231840, 66, 35},   // PIC_SELECT_RETURN_ICON_EN
    /*  73 */ {0x00232A4C, 66, 35},   // PIC_UNSELECT_RETURN_ICON_CN
    /*  74 */ {0x00233C58, 66, 35},   // PIC_UNSELECT_RETURN_ICON_EN
    /*  75 */ {0x00234E64, 88, 34},   // PIC_SELECT_EXIT_ICON_CN
    /*  76 */ {0x002365C4, 88, 34},   // PIC_SELECT_EXIT_ICON_EN
    /*  77 */ {0x00237D24, 88, 34},   // PIC_UNSELECT_EXIT_ICON_CN
    /*  78 */ {0x00239484, 88, 34},   // PIC_UNSELECT_EXIT_ICON_EN
    /*  79 */ {0x0023ABE4, 320, 23},   // PIC_HA01_NAVIGATION_BAR_ICON
    /*  80 */ {0x0023E564, 320, 23},   // PIC_HA02_NAVIGATION_BAR_ICON
    /*  81 */ {0x00241EE4, 320, 23},   // PIC_HA03_NAVIGATION_BAR_ICON
    /*  82 */ {0x00245864, 103, 52},   // PIC_STANDBY_ICON_CN
    /*  83 */ {0x0024823C, 103, 52},   // PIC_STANDBY_ICON_EN
    /*  84 */ {0x0024AC14, 103, 52},   // PIC_CANCEL_STANDBY_ICON
    /*  85 */ {0x0024D5EC, 156, 75},   // PIC_LOW_TEMP_ALARM_CN
    /*  86 */ {0x00253154, 156, 75},   // PIC_LOW_TEMP_ALARM_EN
    /*  87 */ {0x00258CBC, 156, 75},   // PIC_HIGH_TEMP_ALARM_CN
    /*  88 */ {0x0025E824, 156, 75},   // PIC_HIGH_TEMP_ALARM_EN
    /*  89 */ {0x0026438C, 156, 75},   // PIC_FAN_ABNORMAL_CN
    /*  90 */ {0x00269EF4, 156, 75},   // PIC_FAN_ABNORMAL_EN
    /*  91 */ {0x0026FA5C, 64, 33},   // PIC_UNSELECT_CHANNEL1_ICON
    /*  92 */ {0x00270ADC, 64, 33},   // PIC_UNSELECT_CHANNEL2_ICON
    /*  93 */ {0x00271B5C, 64, 33},   // PIC_UNSELECT_CHANNEL3_ICON
    /*  94 */ {0x00272BDC, 64, 33},   // PIC_SELECT_CHANNEL1_ICON
    /*  95 */ {0x00273C5C, 64, 33},   // PIC_SELECT_CHANNEL2_ICON
    /*  96 */ {0x00274CDC, 64, 33},   // PIC_SELECT_CHANNEL3_ICON
    /*  97 */ {0x00275D5C, 26, 27},   // PIC_FAN1_ICON
    /*  98 */ {0x002762D8, 26, 27},   // PIC_FAN2_ICON
    /*  99 */ {0x00276854, 26, 27},   // PIC_FAN3_ICON
    /* 100 */ {0x00276DD0, 26, 27},   // PIC_FAN4_ICON
    /* 101 */ {0x0027734C, 117, 26},   // PIC_NORMAL_MODE_ICON
    /* 102 */ {0x00278B10, 117, 26},   // PIC_COOL_MODE_ICON_CN
    /* 103 */ {0x0027A2D4, 117, 26},   // PIC_COOL_MODE_ICON_EN
    /* 104 */ {0x0027BA98, 117, 26},   // PIC_QUICK_RELEASE_MODE_CN
    /* 105 */ {0x0027D25C, 117, 26},   // PIC_QUICK_RELEASE_MODE_EN
    /* 106 */ {0x0027EA20, 99, 23},   // PIC_SET_TEMP_ICON_CN
    /* 107 */ {0x0027FBEA, 99, 23},   // PIC_SET_TEMP_ICON_EN
    /* 108 */ {0x00280DB4, 99, 23},   // PIC_ACTUAL_TEMP_ICON_CN
    /* 109 */ {0x00281F7E, 99, 23},   // PIC_ACTUAL_TEMP_ICON_EN
    /* 110 */ {0x00283148, 99, 23},   // PIC_SET_FAN_SPEED_ICON_CN
    /* 111 */ {0x00284312, 99, 23},   // PIC_SET_FAN_SPEED_ICON_EN
    /* 112 */ {0x002854DC, 99, 23},   // PIC_ACTUAL_FAN_SPEED_ICON_CN
    /* 113 */ {0x002866A6, 99, 23},   // PIC_ACTUAL_FAN_SPEED_ICON_EN
    /* 114 */ {0x00287870, 34, 22},   // PIC_BOOST_MODE_ICON
    /* 115 */ {0x00287E48, 28, 22},   // PIC_SETTING_PAGE_CELSIUS_ICON
    /* 116 */ {0x00288318, 28, 22},   // PIC_SETTING_PAGE_FAHRENHEIT_ICON
    /* 117 */ {0x002887E8, 20, 16},   // PIC_MAIN_INTERFACE_CELSIUS_ICON
    /* 118 */ {0x00288A68, 20, 16},   // PIC_MAIN_INTERFACE_FAHRENHEIT_ICON
    /* 119 */ {0x00288CE8, 20, 20},   // PIC_CURVE_SCREEN_CELSIUS_ICON
    /* 120 */ {0x00289008, 20, 20},   // PIC_CURVE_SCREEN_FAHRENHEIT_ICON
    /* 121 */ {0x00289328, 21, 21},   // PIC_SOUND_ICON
    /* 122 */ {0x0028969A, 21, 21},   // PIC_MUTE_ICON
    /* 123 */ {0x00289A0C, 17, 17},   // PIC_SELECT_SETTING_ICON
    /* 124 */ {0x00289C4E, 17, 17},   // PIC_DESELECT_SETTING_ICON
    /* 125 */ {0x00289E90, 11, 13},   // PIC_TEMP_SLIDER_ICON
    /* 126 */ {0x00289FAE, 11, 13},   // PIC_FAN_SLIDER_ICON
    /* 127 */ {0x0028A0CC, 6, 6},   // PIC_POWER_OUTPUT_SLIDER_ICON
    /* 128 */ {0x0028A114, 97, 9},   // PIC_HA01_MODEL_ICON
    /* 129 */ {0x0028A7E6, 97, 9},   // PIC_HA02_MODEL_ICON
    /* 130 */ {0x0028AEB8, 97, 9},   // PIC_HA03_MODEL_ICON
    /* 131 */ {0x0028B58A, 73, 26},   // PIC_POWER_OUTPUT_ICON_CN
    /* 132 */ {0x0028C45E, 73, 26},   // PIC_POWER_OUTPUT_ICON_EN
    /* 133 */ {0x0028D332, 320, 33},    // PIC_BOTTOM_INDICATOR_BAR
	/* 134 */ {0x002925B2, 37, 19},    //  
	/* 135 */ {0x00292B30, 14, 14},    //  


};












































































































































_Static_assert(sizeof(g_picInfo) / sizeof(g_picInfo[0]) == PIC_NUM, "g_picInfo size mismatch");

/**
 * @brief Display image from external SPI flash (optimized with DMA bulk transfer)
 * @param x1  Start X coordinate
 * @param y1  Start Y coordinate
 * @param picId  Picture ID (index into g_picInfo array)
 */
void LCD_ShowPictureFromFlash(uint16_t x1, uint16_t y1, PicID picId)
{
    if (picId >= PIC_NUM)
    {
        return;
    }

    uint32_t addr = g_picInfo[picId].addr;
    uint16_t width = g_picInfo[picId].width;
    uint16_t height = g_picInfo[picId].height;
    uint32_t total_bytes = (uint32_t)width * height * 2;   // RGB565, 2 bytes per pixel
    // Set LCD window
    LCD_Address_Set(x1, y1, x1 + width - 1, y1 + height - 1);
    // Keep CS low during entire transfer to avoid glitches
    LCD_CS_Clr();
    // Static buffer to avoid stack overflow (allocated in .bss section)
    static uint8_t buf[BLOCK_SIZE];

    while (total_bytes)
    {
        uint32_t chunk = (total_bytes > BLOCK_SIZE) ? BLOCK_SIZE : total_bytes;
        spiflash_read(buf, addr, chunk);          // Read from flash
        spi_dma_write_bytes(buf, chunk);          // DMA send over SPI (8-bit mode)
        addr += chunk;
        total_bytes -= chunk;
    }

    // Release CS
    LCD_CS_Set();
}


 

/**
 * @brief 绘制进度条（支持横向/竖向，带滑块图片）
 * @param x1, y1      进度条左上角坐标
 * @param x2, y2      进度条右下角坐标（x2 > x1, y2 > y1）
 * @param pic_width   图片宽度（横向模式必需）
 * @param pic_height  图片高度（竖向模式必需）
 * @param scale       进度值 0~100，内部限制防止图片越界
 * @param bar_color   进度区域颜色（横向：左侧/竖向：底部）
 * @param bc_color    背景区域颜色（横向：右侧/竖向：顶部）
 * @param pic         图片资源ID（存储在Flash中）
 * @param direction   0=竖向（上下滑动），1=横向（左右滑动）
 */
void DrawProgressBar(u16 x1, u16 y1, u16 x2, u16 y2,
                     u16 pic_width, u16 pic_height,
                     u8 scale, u16 bar_color, u16 bc_color,
                     u8 pic, u8 direction)
{
    if (x1 >= x2 || y1 >= y2) return;
    if (direction == 0 && pic_height == 0) return;
    if (direction == 1 && pic_width == 0) return;

    

    if (direction == 0)  // 竖向模式（保持原有逻辑）
    {
		// 限制 scale 范围（避免图片超出边界）
    if (scale > 98) scale = 98;
    if (scale < 5)  scale = 5;
		
        u16 bar_length = y2 - y1;
        u32 temp = (u32)bar_length * (100 - scale);
        u16 len = (u16)(temp / 100);
        s16 half_h = (s16)(pic_height / 2);
        s16 top_y    = (s16)y1 - half_h;
        s16 pic_y    = (s16)y1 + (s16)len - half_h;
        s16 bottom_y = (s16)y1 + (s16)len + half_h;

        s16 y_min = 0;
        s16 y_max = (s16)LCD_H - 1;

        // 顶部背景
        s16 fill1_top = (top_y    < y_min) ? y_min : top_y;
        s16 fill1_bot = (pic_y    > y_max) ? y_max : pic_y;
        if (fill1_top < fill1_bot)
            LCD_Fill(x1, (u16)fill1_top, x2 + 1, (u16)fill1_bot, bc_color);

        // 图片
        if (pic_y >= y_min && pic_y + (s16)pic_height <= y_max)
            LCD_ShowPictureFromFlash(x1, (u16)pic_y, pic);

        // 底部进度
        s16 fill2_top = (bottom_y < y_min) ? y_min : bottom_y;
        s16 fill2_bot = ((s16)y2   > y_max) ? y_max : (s16)y2;
        if (fill2_top < fill2_bot) {
            LCD_Fill(x1, (u16)fill2_top, x1+1, (u16)fill2_bot, bc_color);
            LCD_Fill(x2, (u16)fill2_top, x2 + 1, (u16)fill2_bot, bc_color);
            LCD_Fill(x1 + 1, (u16)fill2_top, x2, (u16)fill2_bot, bar_color);
        }
    }
    else  // 横向模式（使用您提供的简洁填充逻辑 + 图片）
    {
		// 限制 scale 范围（避免图片超出边界）
    if (scale > 98) scale = 98;
    if (scale <= 1)  scale = 1;
		
        u16 bar_width = x2 - x1;
        // 计算背景区域宽度（右侧未走完部分）
        u16 len = (u16)((float)bar_width * (100 - scale) / 100);
        u16 split_x = x2 - len;   // 分界点：左侧为进度色，右侧为背景色

        // 图片水平居中于分界点
        s16 half_w = (s16)(pic_width / 2);
        s16 pic_x = (s16)split_x - half_w;
        // 图片垂直居中于进度条
        s16 pic_y = (s16)y1 + ((s16)(y2 - y1) - (s16)pic_height) / 2;

        // 屏幕边界
        s16 x_min = 0, x_max = (s16)LCD_W - 1;
        s16 y_min = 0, y_max = (s16)LCD_H - 1;

        // 调整图片Y坐标
        if (pic_y < y_min) pic_y = y_min;
        if (pic_y + (s16)pic_height > y_max) pic_y = y_max - (s16)pic_height;

        // 左侧进度区域 [x1, split_x]
        if (x1 < split_x && split_x > x_min && x1 < x_max) {
            LCD_Fill(x1, y1, split_x, y2, bar_color);
        }

        // 右侧背景区域 [split_x, x2]
        if (split_x < x2 && split_x < x_max && x2 > x_min) {
            LCD_Fill(split_x, y1, x2, y2, bc_color);
        }

        // 图片显示（确保完全在屏幕内）
        if (pic_x >= x_min && pic_x + (s16)pic_width <= x_max &&
            pic_y >= y_min && pic_y + (s16)pic_height <= y_max) {
            //LCD_ShowPictureFromFlash((u16)pic_x, (u16)pic_y, pic);
        }
    }
}






void LCD_Show_Grid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t unit)
{
    uint8_t i, j;
    uint16_t x_line_space = 0;
    uint16_t y_line_space = 0;
    x_line_space = (x2 - x1) / 5;
    y_line_space = (y2 - y1) / 5;

    for (i = 0; i < 5; i++)
    {
        if (i == 0)
        {
             LCD_DrawRectangle(x1, y1, x2, y2, color);
        }
        else
        {
            LCD_DrawLine(x1, y1 + i * y_line_space, x2, y1 + i * y_line_space, color);
            LCD_DrawLine(x1+1 + i * x_line_space, y1, x1 + i * x_line_space, y2, color);
        }
    }

 
	
 
    LCD_ShowString(x1 + 2, y1 + 20, (uint8_t *)"500(932)", 0xfe50, 0x20e3, 12, 0);
    LCD_ShowString(x1 + 2, y1 + 53, (uint8_t *)"400(752)", 0xfe50, 0x20e3, 12, 0);
    LCD_ShowString(x1 + 2, y1 + 86, (uint8_t *)"300(572)", 0xfe50, 0x20e3, 12, 0);
    LCD_ShowString(x1 + 2, y1 + 119, (uint8_t *)"200(392)", 0xfe50, 0x20e3, 12, 0);
    LCD_ShowString(x1 + 2, y1 + 152, (uint8_t *)"100(212)", 0xfe50, 0x20e3, 12, 0);
    
	
	if(unit == 0)
	{
	    LCD_ShowString(x2 - 18,y1 + 20, (uint8_t *)"120", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 18 + 6,y1 + 53, (uint8_t *)"90", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 18 +6,y1 + 86,  (uint8_t *)"60", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 12,y1 + 119, (uint8_t *)"30", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 6, y1 + 152,(uint8_t *)"0", 0x6dfd, 0x20e3, 12, 0);
	}
	else
	{
	LCD_ShowString(x2 - 18,y1 + 20, (uint8_t *)"100", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 18+ 6,y1 + 53, (uint8_t *)"75", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 18+ 6,y1 + 86,  (uint8_t *)"50", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 12,y1 + 119, (uint8_t *)"25", 0x6dfd, 0x20e3, 12, 0);
    LCD_ShowString(x2 - 6, y1 + 152,(uint8_t *)"0", 0x6dfd, 0x20e3, 12, 0);
	}
	
    
}

// 通用电压到Y坐标映射函数
int voltage_to_y(float voltage, float v_min, float v_max, int y_top, int y_bottom)
{
    // 计算显示区域高度
    int display_height = y_bottom - y_top;

    // 确保电压在范围内
    if (voltage < v_min)
    {
        voltage = v_min;
    }

    if (voltage > v_max)
    {
        voltage = v_max;
    }

    // 计算电压在范围内的比例位置
    float position = (voltage - v_min) / (v_max - v_min);
    // 反转比例（因为屏幕Y轴向下增加）
    position = 1.0 - position;
    // 计算Y坐标（四舍五入）
    int y = y_top + (int)(position * display_height + 0.5);
    return y;
}

void LCD_Show_Curve(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                    float curve_1, float curve_2,
					uint16_t max_curve_1,uint16_t min_curve_1,
					uint16_t max_curve_2,uint16_t min_curve_2,
					uint8_t *curve_1_arr, uint8_t *curve_2_arr)
{
    int i;
    volatile static uint16_t input_data_1 = 0;
    volatile static uint16_t input_data_2 = 0;

    /* get s1 temp data*/
    if (curve_1 >= max_curve_1)
    {
        curve_1 = max_curve_1;
    }

    if (curve_1 <= min_curve_1)
    {
        curve_1 = min_curve_1;
    }

    /* get s1 power data*/
    if (curve_2 >= max_curve_2)
    {
        curve_2 = max_curve_2;
    }

    if (curve_2 <= min_curve_2)
    {
        curve_2 = 0;
    }

    input_data_1 = voltage_to_y(curve_1, 100, 600, y1, y2);
	
	
	
	
    input_data_2 = voltage_to_y(curve_2, 0, max_curve_2, y1+34, y2);
	
	
	if(input_data_1<=y2)
		input_data_1 --;
	if(input_data_2<=y2)
		input_data_2 --;
	

    for (i = 0; i < (x2 - x1 - 1); i++)
    {
        LCD_DrawLine(x1 + i, curve_1_arr[i], x1 + i + 2, curve_1_arr[i + 1], 0x20e4);
        LCD_DrawLine(x1 + i, curve_2_arr[i], x1 + i + 2, curve_2_arr[i + 1], 0x20e4);

        if (i < (x2 - x1 - 2))
        {
            LCD_DrawLine(x1 + i, curve_1_arr[i + 1], x1 + i + 2, curve_1_arr[i + 2], 0xfeb3);
            LCD_DrawLine(x1 + i, curve_2_arr[i + 1], x1 + i + 2, curve_2_arr[i + 2], 0x7e1c);
        }

        curve_1_arr[i] = curve_1_arr[i + 1];
        curve_2_arr[i] = curve_2_arr[i + 1];
    }

    curve_1_arr[(x2 - x1 - 1)] = input_data_1;
    curve_2_arr[(x2 - x1 - 1)] = input_data_2;
}


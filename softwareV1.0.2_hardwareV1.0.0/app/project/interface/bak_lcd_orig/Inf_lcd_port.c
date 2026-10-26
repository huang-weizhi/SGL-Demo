#include "Inf_lcd_port.h"

/*******************************************************************************
 * LCD hardware abstraction layer implementation
 ******************************************************************************/

/* Chip select control: 0 = asserted (low), 1 = released (high) */
void lcd_port_cs(uint8_t select)
{
    if (select)
    {
        gpio_bits_set(LCD_CS_GPIO_PORT, LCD_CS_PIN);    /* release chip select */
    }
    else
    {
        gpio_bits_reset(LCD_CS_GPIO_PORT, LCD_CS_PIN);  /* assert chip select */
    }
}

/* Data/command line control: 0 = command, 1 = data */
void lcd_port_dc(uint8_t is_data)
{
    if (is_data)
    {
        gpio_bits_set(LCD_DC_GPIO_PORT, LCD_DC_PIN);
    }
    else
    {
        gpio_bits_reset(LCD_DC_GPIO_PORT, LCD_DC_PIN);
    }
}

/* Hardware reset line control: 0 = asserted, 1 = released */
void lcd_port_rst(uint8_t level)
{
    if (level)
    {
        gpio_bits_set(LCD_RST_GPIO_PORT, LCD_RST_PIN);
    }
    else
    {
        gpio_bits_reset(LCD_RST_GPIO_PORT, LCD_RST_PIN);
    }
}

/* Backlight control: 0 = off, 1 = on */
void lcd_port_backlight(uint8_t on)
{
    if (on)
    {
        gpio_bits_set(LCD_BL_GPIO_PORT, LCD_BL_PIN);
    }
    else
    {
        gpio_bits_reset(LCD_BL_GPIO_PORT, LCD_BL_PIN);
    }
}

/* SPI2 blocking single byte transfer, does not touch CS/DC */
static uint8_t lcd_port_xfer_byte(uint8_t dat)
{
    while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(LCD_SPI, dat);

    while (spi_i2s_flag_get(LCD_SPI, SPI_I2S_BF_FLAG) != RESET);
    return (uint8_t)spi_i2s_data_receive(LCD_SPI);
}

/* SPI2 blocking single byte transfer: DC state must be set by the caller.
 * CS is framed per byte (assert -> send -> release), matching the reference
 * driver, so that commands and data always reach the LCD controller. */
void lcd_port_send_byte(uint8_t dat)
{
    lcd_port_cs(0);
    lcd_port_xfer_byte(dat);
    lcd_port_cs(1);
}

/*******************************************************************************
 * DMA bulk transfer
 *  - len is limited by the DMA counter width (AT32F415: 16-bit, max 65535)
 *  - the caller is responsible for splitting larger blocks
 ******************************************************************************/
void lcd_port_send_bulk(const uint8_t *buf, uint32_t len)
{
    dma_init_type dma_init_struct;

    if (len == 0)
    {
        return;
    }

    dma_default_para_init(&dma_init_struct);

    /* memory -> peripheral, byte width, memory address increments */
    dma_init_struct.peripheral_base_addr = (uint32_t)&LCD_SPI->dt;
    dma_init_struct.memory_base_addr   = (uint32_t)buf;
    dma_init_struct.direction          = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.buffer_size        = (uint16_t)len;
    dma_init_struct.memory_inc_enable  = TRUE;
    dma_init_struct.memory_data_width  = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.priority           = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable   = FALSE;
    dma_init(LCD_DMA_TX_CHANNEL, &dma_init_struct);

    spi_i2s_dma_transmitter_enable(LCD_SPI, TRUE);
    dma_channel_enable(LCD_DMA_TX_CHANNEL, TRUE);

    /* wait until the whole DMA transfer finishes, then disable channel and SPI DMA request */
    while (dma_flag_get(LCD_DMA_TX_FDT_FLAG) == RESET);
    dma_flag_clear(LCD_DMA_TX_FDT_FLAG);
    dma_channel_enable(LCD_DMA_TX_CHANNEL, FALSE);
    spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
}
#include "Inf_lcd_port.h"
#include <stdbool.h>

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
    lcd_port_send_bulk_start(buf, len);
    lcd_port_send_bulk_wait();
}

/*******************************************************************************
 * DMA bulk transfer, split into start/wait for double-buffered widgets:
 *  - lcd_port_send_bulk_start() configures and kicks the DMA, then returns
 *    immediately; the caller can prepare the next block meanwhile
 *  - lcd_port_send_bulk_wait() blocks until the transfer finished and cleans
 *    up; it is safe to call it without a pending transfer (no-op)
 *  - CS is controlled by the caller (kept asserted around the whole block)
 ******************************************************************************/
static bool lcd_bulk_running = false;
/* RX DMA remains active after start() returns. Its destination must not
 * reside on the stack, which subsequent calls may reuse before wait(). */
static volatile uint8_t lcd_bulk_rx_dummy;

void lcd_port_send_bulk_start(const uint8_t *buf, uint32_t len)
{
    dma_init_type dma_init_struct;

    if (len == 0)
    {
        return;
    }

    lcd_port_send_bulk_wait();  /* guard against a still-running transfer */

    /* mirror the reference driver's spi_dma_write_bytes exactly:
     * reset both channels, consume MISO with a dummy RX channel and wait for
     * the RX complete flag so every byte has fully left the shift register */
    dma_flexible_config(DMA1, FLEX_CHANNEL4, DMA_FLEXIBLE_SPI2_TX);
    dma_flexible_config(DMA1, FLEX_CHANNEL7, DMA_FLEXIBLE_SPI2_RX);

    dma_reset(LCD_DMA_RX_CHANNEL);
    dma_reset(LCD_DMA_TX_CHANNEL);

    /* RX channel: discard whatever MISO returns (prevents overrun) */
    dma_default_para_init(&dma_init_struct);
    dma_init_struct.direction             = DMA_DIR_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_data_width     = DMA_MEMORY_DATA_WIDTH_BYTE;
    dma_init_struct.memory_inc_enable     = FALSE;
    dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
    dma_init_struct.peripheral_inc_enable = FALSE;
    dma_init_struct.priority              = DMA_PRIORITY_HIGH;
    dma_init_struct.loop_mode_enable      = FALSE;
    dma_init_struct.buffer_size           = (uint16_t)len;
    dma_init_struct.peripheral_base_addr  = (uint32_t)&LCD_SPI->dt;
    dma_init_struct.memory_base_addr      = (uint32_t)&lcd_bulk_rx_dummy;
    dma_init(LCD_DMA_RX_CHANNEL, &dma_init_struct);

    /* TX channel: memory -> peripheral, byte width, memory address increments */
    dma_init_struct.direction             = DMA_DIR_MEMORY_TO_PERIPHERAL;
    dma_init_struct.memory_inc_enable     = TRUE;
    dma_init_struct.memory_base_addr      = (uint32_t)buf;
    dma_init(LCD_DMA_TX_CHANNEL, &dma_init_struct);

    /* enable SPI DMA requests, then start both channels */
    spi_i2s_dma_transmitter_enable(LCD_SPI, TRUE);
    spi_i2s_dma_receiver_enable(LCD_SPI, TRUE);
    dma_channel_enable(LCD_DMA_RX_CHANNEL, TRUE);
    dma_channel_enable(LCD_DMA_TX_CHANNEL, TRUE);

    lcd_bulk_running = true;
}

void lcd_port_send_bulk_wait(void)
{
    if (!lcd_bulk_running)
    {
        return;
    }

    /* wait for RX complete: implies every byte has been shifted out */
    while (dma_flag_get(LCD_DMA_RX_FDT_FLAG) == RESET);
    dma_flag_clear(LCD_DMA_RX_FDT_FLAG);

    /* stop channels and SPI DMA requests */
    dma_channel_enable(LCD_DMA_RX_CHANNEL, FALSE);
    dma_channel_enable(LCD_DMA_TX_CHANNEL, FALSE);
    spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
    spi_i2s_dma_receiver_enable(LCD_SPI, FALSE);

    lcd_bulk_running = false;
}

/*******************************************************************************
 * 16-bit (half-word) DMA pixel fill, mirrors the reference driver
 *  - switch SPI to 16-bit frame, use half-word DMA, do NOT increment the
 *    memory pointer so the same color value is repeated for every pixel
 *  - MISO data is consumed on an idle channel (CH7) to avoid RX overrun
 *  - SPI frame is restored to 8-bit when done
 *  - CS is controlled by the caller (kept asserted around the whole fill)
 ******************************************************************************/
void lcd_port_send_halfwords(const uint16_t *buf, uint32_t count)
{
    volatile uint16_t dummy_data;
    dma_init_type dma_init_struct;

    if (count == 0)
    {
        return;
    }

    /* make sure the flexible request mapping is set for both SPI2 DMA channels */
    dma_flexible_config(DMA1, FLEX_CHANNEL4, DMA_FLEXIBLE_SPI2_TX);
    dma_flexible_config(DMA1, FLEX_CHANNEL7, DMA_FLEXIBLE_SPI2_RX);

    /* split into chunks because the DMA counter is limited to 65535 */
    while (count)
    {
        uint16_t chunk = (count > 65534) ? 65534 : (uint16_t)count;

        spi_frame_bit_num_set(LCD_SPI, SPI_FRAME_16BIT);

        /* RX channel: discard whatever MISO returns */
        dma_reset(LCD_DMA_RX_CHANNEL);
        dma_default_para_init(&dma_init_struct);
        dma_init_struct.direction            = DMA_DIR_PERIPHERAL_TO_MEMORY;
        dma_init_struct.memory_data_width    = DMA_MEMORY_DATA_WIDTH_HALFWORD;
        dma_init_struct.memory_inc_enable    = FALSE;
        dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
        dma_init_struct.peripheral_inc_enable = FALSE;
        dma_init_struct.priority             = DMA_PRIORITY_HIGH;
        dma_init_struct.loop_mode_enable     = FALSE;
        dma_init_struct.buffer_size          = chunk;
        dma_init_struct.peripheral_base_addr = (uint32_t)&LCD_SPI->dt;
        dma_init_struct.memory_base_addr     = (uint32_t)&dummy_data;
        dma_init(LCD_DMA_RX_CHANNEL, &dma_init_struct);

        /* TX channel: repeat the same color value (memory pointer not incremented) */
        dma_reset(LCD_DMA_TX_CHANNEL);
        dma_default_para_init(&dma_init_struct);
        dma_init_struct.direction            = DMA_DIR_MEMORY_TO_PERIPHERAL;
        dma_init_struct.memory_data_width    = DMA_MEMORY_DATA_WIDTH_HALFWORD;
        dma_init_struct.memory_inc_enable    = FALSE;
        dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
        dma_init_struct.peripheral_inc_enable = FALSE;
        dma_init_struct.priority             = DMA_PRIORITY_HIGH;
        dma_init_struct.loop_mode_enable     = FALSE;
        dma_init_struct.buffer_size          = chunk;
        dma_init_struct.peripheral_base_addr = (uint32_t)&LCD_SPI->dt;
        dma_init_struct.memory_base_addr     = (uint32_t)buf;
        dma_init(LCD_DMA_TX_CHANNEL, &dma_init_struct);

        spi_i2s_dma_transmitter_enable(LCD_SPI, TRUE);
        spi_i2s_dma_receiver_enable(LCD_SPI, TRUE);
        dma_channel_enable(LCD_DMA_RX_CHANNEL, TRUE);
        dma_channel_enable(LCD_DMA_TX_CHANNEL, TRUE);

        while (dma_flag_get(LCD_DMA_RX_FDT_FLAG) == RESET);
        dma_flag_clear(LCD_DMA_RX_FDT_FLAG);
        dma_channel_enable(LCD_DMA_RX_CHANNEL, FALSE);
        dma_channel_enable(LCD_DMA_TX_CHANNEL, FALSE);
        spi_i2s_dma_transmitter_enable(LCD_SPI, FALSE);
        spi_i2s_dma_receiver_enable(LCD_SPI, FALSE);

        spi_frame_bit_num_set(LCD_SPI, SPI_FRAME_8BIT);

        count -= chunk;
    }
}
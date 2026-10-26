/*=============================================================================
 * Inf_w25qxxx.c - W25Q SPI NOR flash driver (SPI3, DMA)
 *
 * Standard command set (write-enable/page program/sector erase/read status/JEDEC ID)
 * plus an asynchronous read path used by the image loader:
 *   spiflash_read_async_start() : CS low, send 0x03+addr, arm RX DMA, return
 *   spiflash_read_async_done()  : wait DMA finish, CS high (polling)
 * spiflash_read() is the blocking wrapper. Clock divider is set in
 * wk_spi3_init(); keep it at DIV_4 (<=50MHz MCU SPI limit).
 *===========================================================================*//**
  **************************************************************************
  * @file     spi_flash.c
  * @brief    spi_flash source code
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

#include "Inf_w25qxxx.h"

/** @addtogroup AT32F415_periph_examples
  * @{
  */

/** @addtogroup 415_SPI_w25q_flash
  * @{
  */

uint8_t spiflash_sector_buf[SPIF_SECTOR_SIZE];

/* Dummy byte sent while the RX DMA clocked data out of the flash.
   It must have static storage duration so that it stays valid for the
   whole async DMA transfer (a local variable would be destroyed when
   spiflash_read_async_start() returns). */
uint8_t spiflash_dma_dummy = FLASH_SPI_DUMMY_BYTE;

/**
  * @brief  spi configuration.
  * @param  none
  * @retval none
  */
void spiflash_init(void)
{
  gpio_init_type gpio_initstructure;
  spi_init_type spi_init_struct;

  
  crm_periph_clock_enable(SPI_FLASH_CS_GPIO_CLK, TRUE);
  crm_periph_clock_enable(SPI_FLASH_SCK_GPIO_CLK, TRUE);
  crm_periph_clock_enable(SPI_FLASH_MISO_GPIO_CLK, TRUE);
  crm_periph_clock_enable(SPI_FLASH_MOSI_GPIO_CLK, TRUE);
  crm_periph_clock_enable(SPI_FLASH_DMA_CLK, TRUE);
	

  
  gpio_initstructure.gpio_out_type       = GPIO_OUTPUT_PUSH_PULL;
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_OUTPUT;
  gpio_initstructure.gpio_drive_strength = GPIO_DRIVE_STRENGTH_STRONGER;
  gpio_initstructure.gpio_pins           = SPI_FLASH_CS_PIN;
  gpio_init(SPI_FLASH_CS_GPIO_PORT, &gpio_initstructure);

  
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = SPI_FLASH_SCK_PIN;
  gpio_init(SPI_FLASH_SCK_GPIO_PORT, &gpio_initstructure);

  
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_INPUT;
  gpio_initstructure.gpio_pins           = SPI_FLASH_MISO_PIN;
  gpio_init(SPI_FLASH_MISO_GPIO_PORT, &gpio_initstructure);

  
  gpio_initstructure.gpio_pull           = GPIO_PULL_UP;
  gpio_initstructure.gpio_mode           = GPIO_MODE_MUX;
  gpio_initstructure.gpio_pins           = SPI_FLASH_MOSI_PIN;
  gpio_init(SPI_FLASH_MOSI_GPIO_PORT, &gpio_initstructure);

  FLASH_CS_HIGH();

  
  crm_periph_clock_enable(SPI_FLASH_SPI_CLK, TRUE);
  spi_default_para_init(&spi_init_struct);
  spi_init_struct.transmission_mode      = SPI_TRANSMIT_FULL_DUPLEX;
  spi_init_struct.master_slave_mode      = SPI_MODE_MASTER;
  spi_init_struct.mclk_freq_division     = SPI_FLASH_CLOCK_DIV;
  spi_init_struct.first_bit_transmission = SPI_FIRST_BIT_MSB;
  spi_init_struct.frame_bit_num          = SPI_FRAME_8BIT;
  spi_init_struct.clock_polarity         = SPI_CLOCK_POLARITY_HIGH;
  spi_init_struct.clock_phase            = SPI_CLOCK_PHASE_2EDGE;
  spi_init_struct.cs_mode_selection      = SPI_CS_SOFTWARE_MODE;
  spi_init(SPI_FLASH_SPI, &spi_init_struct);
  spi_enable(SPI_FLASH_SPI, TRUE);
}

/**
  * @brief  write data to flash
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void spiflash_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  uint32_t sector_pos;
  uint16_t sector_offset;
  uint16_t sector_remain;
  uint16_t index;
  uint8_t *spiflash_buf;
  spiflash_buf = spiflash_sector_buf;

  /* sector address */
  sector_pos = write_addr / SPIF_SECTOR_SIZE;

  /* address offset in a sector */
  sector_offset = write_addr % SPIF_SECTOR_SIZE;

  /* the remain in a sector */
  sector_remain = SPIF_SECTOR_SIZE - sector_offset;
  if(length <= sector_remain)
  {
    /* smaller than a sector size */
    sector_remain = length;
  }
  while(1)
  {
    /* read a sector */
    spiflash_read(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE);

    /* validate the read erea */
    for(index = 0; index < sector_remain; index++)
    {
      if(spiflash_buf[sector_offset + index] != 0xFF)
      {
        /* there are some data not equal 0xff, so this secotr needs erased */
        break;
      }
    }
    if(index < sector_remain)
    {
      /* erase the sector */
      spiflash_sector_erase(sector_pos);

      /* copy the write data */
      for(index = 0; index < sector_remain; index++)
      {
        spiflash_buf[index + sector_offset] = pbuffer[index];
      }
      spiflash_write_nocheck(spiflash_buf, sector_pos * SPIF_SECTOR_SIZE, SPIF_SECTOR_SIZE); /* program the sector */
    }
    else
    {
      /* write directly in the erased area */
      spiflash_write_nocheck(pbuffer, write_addr, sector_remain);
    }
    if(length == sector_remain)
    {
      /* write end */
      break;
    }
    else
    {
      /* go on writing */
      sector_pos++;
      sector_offset = 0;

      pbuffer += sector_remain;
      write_addr += sector_remain;
      length -= sector_remain;
      if(length > SPIF_SECTOR_SIZE)
      {
        /* could not write the remain data in the next sector */
        sector_remain = SPIF_SECTOR_SIZE;
      }
      else
      {
        /* could write the remain data in the next sector */
        sector_remain = length;
      }
    }
  }
}

/**
  * @brief  read data from flash (blocking wrapper around the async API)
  * @param  pbuffer: the pointer for data buffer
  * @param  read_addr: the address where the data is read
  * @param  length: buffer length
  * @retval none
  */
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length)
{
  spiflash_read_async_start(pbuffer, read_addr, length);
  spiflash_read_async_done();
}

/**
  * @brief  start an async (non-blocking) read from flash
  * @param  pbuffer: the pointer for data buffer (must stay valid until done)
  * @param  read_addr: the address where the data is read
  * @param  length: buffer length
  * @retval none
  * @note   Pulls CS low, sends the read command + 24-bit address over CPU,
  *         then starts the TX/RX DMA pair and returns immediately.
  *         spiflash_read_async_done() must be called to wait for completion
  *         and to release CS. This lets a caller overlap the SPI DMA read
  *         with other work (e.g. an LCD DMA transfer) using double buffers.
  */
void spiflash_read_async_start(uint8_t *pbuffer, uint32_t read_addr, uint32_t length)
{
  dma_init_type dma_init_struct;

  FLASH_CS_LOW();
  spi_byte_write(SPIF_READDATA); /* send instruction (standard Read 0x03) */
  spi_byte_write((uint8_t)((read_addr) >> 16)); /* send 24-bit address */
  spi_byte_write((uint8_t)((read_addr) >> 8));
  spi_byte_write((uint8_t)read_addr);

  /* configure TX DMA: repeatedly clock out the dummy byte (memory fixed) */
  dma_reset(SPI_FLASH_DMA_RX_CHANNEL);
  dma_reset(SPI_FLASH_DMA_TX_CHANNEL);
  /* spiflash waits for the RX DMA by polling the FDT flag, it never uses the
     DMA interrupt. Force the channels' full-transfer interrupt OFF so that
     DMA1_Channel1/2_IRQHandler cannot clear the FDT flags and race with the
     polling in spiflash_read_async_done() (esp. if WorkBench re-generated
     wk_dma.c with dma_interrupt_enable(...DMA_FDT_INT, TRUE)). */
  dma_interrupt_enable(SPI_FLASH_DMA_RX_CHANNEL, DMA_FDT_INT, FALSE);
  dma_interrupt_enable(SPI_FLASH_DMA_TX_CHANNEL, DMA_FDT_INT, FALSE);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t)&spiflash_dma_dummy;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = FALSE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_TX_CHANNEL, &dma_init_struct);

  /* configure RX DMA: receive the data into pbuffer */
  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)pbuffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_RX_CHANNEL, &dma_init_struct);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, TRUE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, TRUE);
  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, TRUE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, TRUE);
}

/**
  * @brief  wait for an async read started by spiflash_read_async_start()
  * @param  none
  * @retval none
  * @note   Waits for the RX DMA full-transfer flag, waits for the SPI bus
  *         to go idle, disables the DMA channels and releases CS.
  */
void spiflash_read_async_done(void)
{
  while(dma_flag_get(SPI_FLASH_DMA_RX_FDT_FLAG) == RESET);
  dma_flag_clear(SPI_FLASH_DMA_RX_FDT_FLAG);

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);

  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, FALSE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, FALSE);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, FALSE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, FALSE);
  FLASH_CS_HIGH();
}

/**
  * @brief  erase a sector data
  * @param  erase_addr: sector address to erase
  * @retval none
  */
void spiflash_sector_erase(uint32_t erase_addr)
{
  erase_addr *= SPIF_SECTOR_SIZE; /* translate sector address to byte address */
  spiflash_write_enable();
  spiflash_wait_busy();
  FLASH_CS_LOW();
  spi_byte_write(SPIF_SECTORERASE);
  spi_byte_write((uint8_t)((erase_addr) >> 16));
  spi_byte_write((uint8_t)((erase_addr) >> 8));
  spi_byte_write((uint8_t)erase_addr);
  FLASH_CS_HIGH();
  spiflash_wait_busy();
}

/**
  * @brief  write data without check
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void spiflash_write_nocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  uint16_t page_remain;

  /* remain bytes in a page */
  page_remain = SPIF_PAGE_SIZE - write_addr % SPIF_PAGE_SIZE;
  if(length <= page_remain)
  {
    /* smaller than a page size */
    page_remain = length;
  }
  while(1)
  {
    spiflash_page_write(pbuffer, write_addr, page_remain);
    if(length == page_remain)
    {
      /* all data are programmed */
      break;
    }
    else
    {
      /* length > page_remain */
      pbuffer += page_remain;
      write_addr += page_remain;

      /* the remain bytes to be prorammed */
      length -= page_remain;
      if(length > SPIF_PAGE_SIZE)
      {
        /* can be progrmmed a page at a time */
        page_remain = SPIF_PAGE_SIZE;
      }
      else
      {
        /* smaller than a page size */
        page_remain = length;
      }
    }
  }
}

/**
  * @brief  write a page data
  * @param  pbuffer: the pointer for data buffer
  * @param  write_addr: the address where the data is written
  * @param  length: buffer length
  * @retval none
  */
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length)
{
  if((0 < length) && (length <= SPIF_PAGE_SIZE))
  {
    /* set write enable */
    spiflash_write_enable();

    FLASH_CS_LOW();

    /* send instruction */
    spi_byte_write(SPIF_PAGEPROGRAM);

    /* send 24-bit address */
    spi_byte_write((uint8_t)((write_addr) >> 16));
    spi_byte_write((uint8_t)((write_addr) >> 8));
    spi_byte_write((uint8_t)write_addr);
    spi_bytes_write(pbuffer,length);

    FLASH_CS_HIGH();

    /* wait for program end */
    spiflash_wait_busy();
  }
}

/**
  * @brief  write data continuously
  * @param  pbuffer: the pointer for data buffer
  * @param  length: buffer length
  * @retval none
  */
void spi_bytes_write(uint8_t *pbuffer, uint32_t length)
{
  volatile uint8_t dummy_data;

#if defined(SPI_TRANS_DMA)
  dma_init_type dma_init_struct;
  dma_reset(SPI_FLASH_DMA_RX_CHANNEL);
  dma_reset(SPI_FLASH_DMA_TX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)&dummy_data;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = FALSE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_RX_CHANNEL, &dma_init_struct);

  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t)pbuffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_TX_CHANNEL, &dma_init_struct);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, TRUE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, TRUE);

  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, TRUE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, TRUE);

  while(dma_flag_get(SPI_FLASH_DMA_RX_FDT_FLAG) == RESET);
  dma_flag_clear(SPI_FLASH_DMA_RX_FDT_FLAG);

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);

  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, FALSE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, FALSE);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, FALSE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, FALSE);
#else
  while(length--)
  {
    while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI_FLASH_SPI, *pbuffer);
    while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_RDBF_FLAG) == RESET);
    dummy_data = spi_i2s_data_receive(SPI_FLASH_SPI);
    pbuffer++;
  }

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);
#endif
}

/**
  * @brief  read data continuously
  * @param  pbuffer: buffer to save data
  * @param  length: buffer length
  * @retval none
  */
void spi_bytes_read(uint8_t *pbuffer, uint32_t length)
{
  uint8_t write_value = FLASH_SPI_DUMMY_BYTE;

#if defined(SPI_TRANS_DMA)
  dma_init_type dma_init_struct;
  dma_reset(SPI_FLASH_DMA_RX_CHANNEL);
  dma_reset(SPI_FLASH_DMA_TX_CHANNEL);
  dma_default_para_init(&dma_init_struct);
  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_MEMORY_TO_PERIPHERAL;
  dma_init_struct.memory_base_addr = (uint32_t)&write_value;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = FALSE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_TX_CHANNEL, &dma_init_struct);

  dma_init_struct.buffer_size = length;
  dma_init_struct.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
  dma_init_struct.memory_base_addr = (uint32_t)pbuffer;
  dma_init_struct.memory_data_width = DMA_MEMORY_DATA_WIDTH_BYTE;
  dma_init_struct.memory_inc_enable = TRUE;
  dma_init_struct.peripheral_base_addr = (uint32_t)(&SPI_FLASH_SPI->dt);
  dma_init_struct.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_BYTE;
  dma_init_struct.peripheral_inc_enable = FALSE;
  dma_init_struct.priority = SPI_FLASH_DMA_PRIORITY;
  dma_init_struct.loop_mode_enable = FALSE;
  dma_init(SPI_FLASH_DMA_RX_CHANNEL, &dma_init_struct);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, TRUE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, TRUE);
  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, TRUE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, TRUE);

  while(dma_flag_get(SPI_FLASH_DMA_RX_FDT_FLAG) == RESET);
  dma_flag_clear(SPI_FLASH_DMA_RX_FDT_FLAG);

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);

  dma_channel_enable(SPI_FLASH_DMA_RX_CHANNEL, FALSE);
  dma_channel_enable(SPI_FLASH_DMA_TX_CHANNEL, FALSE);

  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, FALSE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, FALSE);
#else
  while(length--)
  {
    while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_TDBE_FLAG) == RESET);
    spi_i2s_data_transmit(SPI_FLASH_SPI, write_value);
    while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_RDBF_FLAG) == RESET);
    *pbuffer = spi_i2s_data_receive(SPI_FLASH_SPI);
    pbuffer++;
  }

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);
#endif
}

/**
  * @brief  wait program done
  * @param  none
  * @retval none
  */
void spiflash_wait_busy(void)
{
  while((spiflash_read_sr1() & 0x01) == 0x01);
}

/**
  * @brief  read sr1 register
  * @param  none
  * @retval none
  */
uint8_t spiflash_read_sr1(void)
{
  uint8_t breadbyte = 0;
  FLASH_CS_LOW();
  spi_byte_write(SPIF_READSTATUSREG1);
  breadbyte = (uint8_t)spi_byte_read();
  FLASH_CS_HIGH();
  return (breadbyte);
}

/**
  * @brief  enable write operation
  * @param  none
  * @retval none
  */
void spiflash_write_enable(void)
{
  FLASH_CS_LOW();
  spi_byte_write(SPIF_WRITEENABLE);
  FLASH_CS_HIGH();
}

/**
  * @brief  read device id
  * @param  none
  * @retval device id
  */
uint16_t spiflash_read_id(void)
{
  uint16_t wreceivedata = 0;
  FLASH_CS_LOW();
  spi_byte_write(SPIF_MANUFACTDEVICEID);
  spi_byte_write(0x00);
  spi_byte_write(0x00);
  spi_byte_write(0x00);
  wreceivedata |= spi_byte_read() << 8;
  wreceivedata |= spi_byte_read();
  FLASH_CS_HIGH();
  return wreceivedata;
}

/**
  * @brief  write a byte to flash
  * @param  data: data to write
  * @retval flash return data
  */
uint8_t spi_byte_write(uint8_t data)
{
  uint8_t brxbuff;
  spi_i2s_dma_transmitter_enable(SPI_FLASH_SPI, FALSE);
  spi_i2s_dma_receiver_enable(SPI_FLASH_SPI, FALSE);
  spi_i2s_data_transmit(SPI_FLASH_SPI, data);
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_RDBF_FLAG) == RESET);
  brxbuff = spi_i2s_data_receive(SPI_FLASH_SPI);

  /* wait spi idle when communication end */
  while(spi_i2s_flag_get(SPI_FLASH_SPI, SPI_I2S_BF_FLAG) != RESET);

  return brxbuff;
}

/**
  * @brief  read a byte to flash
  * @param  none
  * @retval flash return data
  */
uint8_t spi_byte_read(void)
{
  return (spi_byte_write(FLASH_SPI_DUMMY_BYTE));
}

/**
  * @brief  flash read/write self test
  * @param  none
  * @retval 1 = write/read round-trip OK, 0 = mismatch
  */
uint8_t spiflash_self_test(void)
{
  static uint8_t wr_buf[512], rd_buf[512];
  const uint32_t test_addr = 0x00010000;   /* dedicated test area, off the image */
  uint32_t i;

  for (i = 0; i < 512; i++)
  {
    wr_buf[i] = (uint8_t)i;                /* 0..255,0..255 */
  }

  spiflash_write(wr_buf, test_addr, 512);
  spiflash_read(rd_buf, test_addr, 512);

  for (i = 0; i < 512; i++)
  {
    if (wr_buf[i] != rd_buf[i])
    {
      return 0;
    }
  }
  return 1;
}

/**
  * @}
  */

/**
  * @}
  */
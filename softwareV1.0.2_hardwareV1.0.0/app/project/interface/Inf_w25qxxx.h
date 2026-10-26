/*=============================================================================
 * Inf_w25qxxx.h - SPI flash driver interface
 *   Commands/status defines for W25Q family
 *   spiflash_init/read/write/erase : blocking API
 *   spiflash_read_async_start/done : non-blocking pipelined read
 *===========================================================================*//**
 * @file    Inf_w25qxxx.h
 * @brief   Header file for W25Q128 / W25Q256 SPI Flash driver
 */

#ifndef __INF_W25QXXX_H
#define __INF_W25QXXX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f415.h"

/** @addtogroup AT32F415_periph_examples
  * @{
  */

/** @addtogroup 415_SPI_w25q_flash
  * @{
  */

/* use dma transfer spi data */
#define SPI_TRANS_DMA

/*============================================================================*/

/*============================================================================*/


#define SPI_FLASH_SPI                   SPI1
#define SPI_FLASH_SPI_CLK               CRM_SPI1_PERIPH_CLOCK


 
#define SPI_FLASH_CS_GPIO_PORT          GPIOA
#define SPI_FLASH_CS_GPIO_CLK           CRM_GPIOA_PERIPH_CLOCK
#define SPI_FLASH_CS_PIN                GPIO_PINS_4

#define SPI_FLASH_SCK_GPIO_PORT         GPIOA
#define SPI_FLASH_SCK_GPIO_CLK          CRM_GPIOA_PERIPH_CLOCK 
#define SPI_FLASH_SCK_PIN               GPIO_PINS_5

#define SPI_FLASH_MISO_GPIO_PORT        GPIOA
#define SPI_FLASH_MISO_GPIO_CLK         CRM_GPIOA_PERIPH_CLOCK
#define SPI_FLASH_MISO_PIN              GPIO_PINS_6

#define SPI_FLASH_MOSI_GPIO_PORT        GPIOA
#define SPI_FLASH_MOSI_GPIO_CLK         CRM_GPIOA_PERIPH_CLOCK
#define SPI_FLASH_MOSI_PIN              GPIO_PINS_7


#define FLASH_CS_HIGH()    gpio_bits_set(SPI_FLASH_CS_GPIO_PORT, SPI_FLASH_CS_PIN)
#define FLASH_CS_LOW()     gpio_bits_reset(SPI_FLASH_CS_GPIO_PORT, SPI_FLASH_CS_PIN)


#define SPI_FLASH_DMA_CLK               CRM_DMA1_PERIPH_CLOCK
#define SPI_FLASH_DMA_TX_CHANNEL        DMA1_CHANNEL3    
#define SPI_FLASH_DMA_RX_CHANNEL        DMA1_CHANNEL2    
#define SPI_FLASH_DMA_PRIORITY          DMA_PRIORITY_LOW


#define SPI_FLASH_DMA_RX_FDT_FLAG       DMA1_FDT2_FLAG   



#define SPI_FLASH_CLOCK_DIV             SPI_MCLK_DIV_2

/*============================================================================*/

/*============================================================================*/

/** @defgroup SPI_flash_id_definition
  * @{
  */

/*
 * flash define
 */
#define W25Q80                           0xEF13
#define W25Q16                           0xEF14
#define W25Q32                           0xEF15
#define W25Q64                           0xEF16
/* 16mb, the range of address:0~0xFFFFFF */
#define W25Q128                          0xEF17

/**
  * @}
  */

/** @defgroup SPI_flash_operation_definition
  * @{
  */

#define SPIF_CHIP_SIZE                   0x1000000
#define SPIF_SECTOR_SIZE                 4096
#define SPIF_PAGE_SIZE                   256

#define SPIF_WRITEENABLE                 0x06
#define SPIF_WRITEDISABLE                0x04
/* s7-s0 */
#define SPIF_READSTATUSREG1              0x05
#define SPIF_WRITESTATUSREG1             0x01
/* s15-s8 */
#define SPIF_READSTATUSREG2              0x35
#define SPIF_WRITESTATUSREG2             0x31
/* s23-s16 */
#define SPIF_READSTATUSREG3              0x15
#define SPIF_WRITESTATUSREG3             0x11
#define SPIF_READDATA                    0x03
#define SPIF_FASTREADDATA                0x0B
#define SPIF_FASTREADDUAL                0x3B
#define SPIF_PAGEPROGRAM                 0x02
/* block size:64kb */
#define SPIF_BLOCKERASE                  0xD8
#define SPIF_SECTORERASE                 0x20
#define SPIF_CHIPERASE                   0xC7
#define SPIF_POWERDOWN                   0xB9
#define SPIF_RELEASEPOWERDOWN            0xAB
#define SPIF_DEVICEID                    0xAB
#define SPIF_MANUFACTDEVICEID            0x90
#define SPIF_JEDECDEVICEID               0x9F
#define FLASH_SPI_DUMMY_BYTE             0xA5

/**
  * @}
  */

/** @defgroup SPI_flash_exported_functions
  * @{
  */

void spiflash_init(void);
void spiflash_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spiflash_read(uint8_t *pbuffer, uint32_t read_addr, uint32_t length);
/* Non-blocking (async DMA) read: start the read, then poll/completion later.
   spiflash_read_async_start() issues the command + starts DMA and returns
   immediately; spiflash_read_async_done() waits for the DMA to finish and
   releases the chip-select line. spiflash_read() is a blocking wrapper that
   combines both. */
void spiflash_read_async_start(uint8_t *pbuffer, uint32_t read_addr, uint32_t length);
void spiflash_read_async_done(void);
void spiflash_sector_erase(uint32_t erase_addr);
void spiflash_write_nocheck(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spiflash_page_write(uint8_t *pbuffer, uint32_t write_addr, uint32_t length);
void spi_bytes_write(uint8_t *pbuffer, uint32_t length);
void spi_bytes_read(uint8_t *pbuffer, uint32_t length);
void spiflash_wait_busy(void);
uint8_t spiflash_read_sr1(void);
void spiflash_write_enable(void);
uint16_t spiflash_read_id(void);
uint8_t spi_byte_write(uint8_t data);
uint8_t spi_byte_read(void);

/**
  * @brief  flash read/write self test
  * @param  none
  * @retval 1 = write/read round-trip OK, 0 = mismatch
  *  note: uses static buffers; writes to a dedicated test area 0x00010000
  *        (erases one sector there), away from the image stored at address 0.
  */
uint8_t spiflash_self_test(void);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif
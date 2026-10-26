/**
  **************************************************************************
  * @file     flash.h
  * @brief    flash header file
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

#ifndef __FLASH_H__
#define __FLASH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "at32f415.h"

/** @addtogroup UTILITIES_examples
  * @{
  */

/** @addtogroup USART_iap_bootloader
  * @{
  */

/** @defgroup bootloader_definition
  * @{
  */

#define FLASH_SIZE    (*((uint32_t*)0x1FFFF7E0))  /* read from at32 flash capacity register(unit:kbyte) */
#define SRAM_SIZE     32                         /* sram size, unit:kbyte */

#define APP_START_ADDR          0x08002000
/* the previous sector of app starting address is iap upgrade flag */
#define IAP_UPGRADE_FLAG_ADDR    (APP_START_ADDR - 0x800)

/* when app received cmd 0x5aa5 from pc-tool, will set up the flag,
indicates that an app upgrade will follow, see iap application note for more details */
#define IAP_UPGRADE_FLAG         0x41544B38
/**
  * @}
  */

/** @defgroup bootloader_exported_functions
  * @{
  */

void flash_2kb_write(uint32_t write_addr, uint8_t *pbuffer);
void flash_1kb_write(uint32_t write_addr, uint8_t *pbuffer); 
flag_status flash_upgrade_flag_read(void);

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

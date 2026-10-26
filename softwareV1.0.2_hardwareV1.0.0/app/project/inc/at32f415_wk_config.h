/* add user code begin Header */
/**
 **************************************************************************
 * @file     at32f415_wk_config.h
 * @brief    header file of work bench config
 **************************************************************************
 * Copyright (c) 2025, Artery Technology, All rights reserved.
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
/* add user code end Header */

/* define to prevent recursive inclusion -----------------------------------*/
#ifndef __AT32F415_WK_CONFIG_H
#define __AT32F415_WK_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes -----------------------------------------------------------------------*/
#include "stdio.h"
#include "at32f415.h"

/* private includes -------------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* exported types -------------------------------------------------------------*/
/* add user code begin exported types */

/* add user code end exported types */

/* exported constants --------------------------------------------------------*/
/* add user code begin exported constants */

/* add user code end exported constants */

/* exported macro ------------------------------------------------------------*/
/* add user code begin exported macro */

/* add user code end exported macro */

/* add user code begin dma define */
/* user can only modify the dma define value */
#define DMA1_CHANNEL1_BUFFER_SIZE   20      /* 20 NTC sequences = 20 ms window */
#define DMA1_CHANNEL1_MEMORY_BASE_ADDR (uint32_t)scz01.ad_value_buff
  // #define DMA1_CHANNEL1_PERIPHERAL_BASE_ADDR  0

#define DMA1_CHANNEL2_BUFFER_SIZE 0
#define DMA1_CHANNEL2_MEMORY_BASE_ADDR 0
  // #define DMA1_CHANNEL2_PERIPHERAL_BASE_ADDR   0

#define DMA1_CHANNEL3_BUFFER_SIZE 0
#define DMA1_CHANNEL3_MEMORY_BASE_ADDR 0
  // #define DMA1_CHANNEL3_PERIPHERAL_BASE_ADDR   0

#define DMA1_CHANNEL4_BUFFER_SIZE 0
#define DMA1_CHANNEL4_MEMORY_BASE_ADDR 0
  // #define DMA1_CHANNEL4_PERIPHERAL_BASE_ADDR   0

#define DMA1_CHANNEL5_BUFFER_SIZE 0
#define DMA1_CHANNEL5_MEMORY_BASE_ADDR 0
  // #define DMA1_CHANNEL5_PERIPHERAL_BASE_ADDR   0

#define DMA1_CHANNEL6_BUFFER_SIZE 0
#define DMA1_CHANNEL6_MEMORY_BASE_ADDR 0
// #define DMA1_CHANNEL6_PERIPHERAL_BASE_ADDR   0

// #define DMA1_CHANNEL7_BUFFER_SIZE   0
// #define DMA1_CHANNEL7_MEMORY_BASE_ADDR   0
// #define DMA1_CHANNEL7_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL1_BUFFER_SIZE   0
// #define DMA2_CHANNEL1_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL1_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL2_BUFFER_SIZE   0
// #define DMA2_CHANNEL2_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL2_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL3_BUFFER_SIZE   0
// #define DMA2_CHANNEL3_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL3_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL4_BUFFER_SIZE   0
// #define DMA2_CHANNEL4_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL4_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL5_BUFFER_SIZE   0
// #define DMA2_CHANNEL5_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL5_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL6_BUFFER_SIZE   0
// #define DMA2_CHANNEL6_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL6_PERIPHERAL_BASE_ADDR   0

// #define DMA2_CHANNEL7_BUFFER_SIZE   0
// #define DMA2_CHANNEL7_MEMORY_BASE_ADDR   0
// #define DMA2_CHANNEL7_PERIPHERAL_BASE_ADDR   0
/* add user code end dma define */

/* Private defines -------------------------------------------------------------*/
#define IN_KEY_SET_L_PIN    GPIO_PINS_0
#define IN_KEY_SET_L_GPIO_PORT    GPIOA
#define IN_TEMP_ANALOG_PIN    GPIO_PINS_1
#define IN_TEMP_ANALOG_GPIO_PORT    GPIOA
#define IN_KEY_ADD_L_PIN    GPIO_PINS_2
#define IN_KEY_ADD_L_GPIO_PORT    GPIOA
#define OUT_HEATING_PWM_PIN    GPIO_PINS_3
#define OUT_HEATING_PWM_GPIO_PORT    GPIOA
#define OUT_W25Q64_CS_L_PIN    GPIO_PINS_4
#define OUT_W25Q64_CS_L_GPIO_PORT    GPIOA
#define W25Q64_CLK_PIN    GPIO_PINS_5
#define W25Q64_CLK_GPIO_PORT    GPIOA
#define W25Q64_MISO_PIN    GPIO_PINS_6
#define W25Q64_MISO_GPIO_PORT    GPIOA
#define W25Q64_MOSI_PIN    GPIO_PINS_7
#define W25Q64_MOSI_GPIO_PORT    GPIOA
#define OUT_BEEP_H_PIN    GPIO_PINS_10
#define OUT_BEEP_H_GPIO_PORT    GPIOB
#define OUT_LCD_BL_H_PIN    GPIO_PINS_11
#define OUT_LCD_BL_H_GPIO_PORT    GPIOB
#define OUT_LCD_CS_L_PIN    GPIO_PINS_12
#define OUT_LCD_CS_L_GPIO_PORT    GPIOB
#define LCD_CLK_PIN    GPIO_PINS_13
#define LCD_CLK_GPIO_PORT    GPIOB
#define LCD_MISO_PIN    GPIO_PINS_14
#define LCD_MISO_GPIO_PORT    GPIOB
#define LCD_MOSI_PIN    GPIO_PINS_15
#define LCD_MOSI_GPIO_PORT    GPIOB
#define OUT_LCD_DC_HL_PIN    GPIO_PINS_8
#define OUT_LCD_DC_HL_GPIO_PORT    GPIOA
#define OUT_LCD_RST_H_PIN    GPIO_PINS_11
#define OUT_LCD_RST_H_GPIO_PORT    GPIOA
#define IN_KEY_CH2_L_PIN    GPIO_PINS_7
#define IN_KEY_CH2_L_GPIO_PORT    GPIOB
#define IN_KEY_CH1_L_PIN    GPIO_PINS_8
#define IN_KEY_CH1_L_GPIO_PORT    GPIOB
#define IN_KEY_DOWN_L_PIN    GPIO_PINS_9
#define IN_KEY_DOWN_L_GPIO_PORT    GPIOB

/* exported functions ------------------------------------------------------- */
  /* system clock config. */
  void wk_system_clock_config(void);

  /* config periph clock. */
  void wk_periph_clock_config(void);

  /* nvic config. */
  void wk_nvic_config(void);

/* add user code begin exported functions */

  /* add user code end exported functions */

#ifdef __cplusplus
}
#endif

#endif

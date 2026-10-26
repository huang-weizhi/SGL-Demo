/**
  **************************************************************************
  * @file     iap.c
  * @brief    iap program
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

#include "iap.h"
#include "flash.h"


/** @addtogroup UTILITIES_examples
  * @{
  */

/** @addtogroup USART_iap_bootloader
  * @{
  */

iapfun jump_to_app;

/* app_load don't optimize */
#if defined (__ARMCC_VERSION)
 #if (__ARMCC_VERSION >= 6010050)
  __attribute__((optnone))
 #else
  #pragma O0
 #endif
#elif defined (__ICCARM__)
  #pragma optimize=s none
#elif defined (__GNUC__)
  __attribute__((optimize("O0")))
#endif

/**
  * @brief  app load.
  * @param  app_addr
  *         app code starting address
  * @retval none
  */
void app_load(uint32_t app_addr)
{
  /* check the address of stack */
  if(((*(uint32_t*)app_addr) - 0x20000000) <= (SRAM_SIZE * 1024))
  {
  /* disable periph clock */
  /* disable crc periph clock */
  crm_periph_clock_enable(CRM_CRC_PERIPH_CLOCK, FALSE);

  /* disable iomux periph clock */
  crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, FALSE);

  /* disable gpioa periph clock */
  crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, FALSE);

  /* disable gpiob periph clock */
  crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, FALSE);

  /* disable gpioc periph clock */
  crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, FALSE);

  /* disable gpiod periph clock */
  crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, FALSE);

  /* disable gpiof periph clock */
  crm_periph_clock_enable(CRM_GPIOF_PERIPH_CLOCK, FALSE);

  /* disable usart1 periph clock */
  crm_periph_clock_enable(CRM_USART1_PERIPH_CLOCK, FALSE);

	  

    /* disable nvic irq and clear pending */
    nvic_irq_disable(USART1_IRQn);
    __NVIC_ClearPendingIRQ(USART1_IRQn);

    jump_to_app = (iapfun)*(uint32_t*)(app_addr + 4);        /* code second word is reset address */
    __set_MSP(*(uint32_t*)app_addr);                        /* init app stack pointer(code first word is stack address) */
	jump_to_app();                                          /* jump to user app */
  }
}








/**
  * @}
  */

/**
  * @}
  */

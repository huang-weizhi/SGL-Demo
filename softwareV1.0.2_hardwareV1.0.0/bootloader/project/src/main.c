/* add user code begin Header */
/**
  **************************************************************************
  * @file     main.c
  * @brief    main program
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
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f415_wk_config.h"
#include "wk_crc.h"
#include "wk_debug.h"
#include "wk_usart.h"
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <stdbool.h>
#include "at32f415_int.h"
#include "iap.h"
#include "flash.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */
#define HA01 0
/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */
bool iap_done = false;

uint8_t LOCAL_DEVICE_ID_1 = 0;
uint8_t LOCAL_DEVICE_ID_2 = 0;
/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  /* add user code begin 1 */

  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* init debug function. */
  wk_debug_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config for
     void wk_delay_ms(uint32_t delay); */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init usart1 function. */
  wk_usart1_init();

  /* init crc function. */
  wk_crc_init();
 
  /* add user code begin 2 */

    if (flash_upgrade_flag_read() == RESET)
    {
        /* check app starting address whether 0x08xxxxxx */
        if (((*(uint32_t *)(APP_START_ADDR + 4)) & 0xFF000000) == 0x08000000)
        {
            app_load(APP_START_ADDR);    
        }
    }
	    /* check whether need to upgrade, if yes, response ok to pc-tool */
    if (flash_upgrade_flag_read() != RESET)
        /* send ok mes */
    {
        usart1_send_ok(); 
    }
  /* add user code end 2 */

  while(1)
  {
    /* add user code begin 3 */
        /* upgrade handle */
        if (iap_done)
        {
            iap_done = FALSE;
            /* check app starting address whether 0x08xxxxxx */
            if (((*(uint32_t *)(APP_START_ADDR + 4)) & 0xFF000000) == 0x08000000)
            { 
                crm_reset();
                /* jump and run in app */
                app_load(APP_START_ADDR);
            }
        }
    /* add user code end 3 */
  }
}

  /* add user code begin 4 */

  /* add user code end 4 */

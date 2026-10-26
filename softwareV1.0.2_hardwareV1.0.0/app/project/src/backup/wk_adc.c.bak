/* add user code begin Header */
/**
  **************************************************************************
  * @file     wk_adc.c
  * @brief    work bench config program
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

/* Includes ------------------------------------------------------------------*/
#include "wk_adc.h"

/* add user code begin 0 */

/* add user code end 0 */

/**
  * @brief  init adc1 function.
  * @param  none
  * @retval none
  */
void wk_adc1_init(void)
{
  /* add user code begin adc1_init 0 */

  /* add user code end adc1_init 0 */

  gpio_init_type gpio_init_struct;
  adc_base_config_type adc_base_struct;

  gpio_default_para_init(&gpio_init_struct);

  /* add user code begin adc1_init 1 */

  /* add user code end adc1_init 1 */

  /*gpio--------------------------------------------------------------------*/ 
  /* configure the IN1 pin */
  gpio_init_struct.gpio_mode = GPIO_MODE_ANALOG;
  gpio_init_struct.gpio_pins = IN_TEMP_ANALOG_PIN;
  gpio_init(IN_TEMP_ANALOG_GPIO_PORT, &gpio_init_struct);

  adc_reset(ADC1);
  crm_adc_clock_div_set(CRM_ADC_DIV_4);

  adc_tempersensor_vintrv_enable(TRUE);

  /*adc_settings--------------------------------------------------------------------*/ 
  adc_base_default_para_init(&adc_base_struct);
  adc_base_struct.sequence_mode = FALSE;
  adc_base_struct.repeat_mode = FALSE;
  adc_base_struct.data_align = ADC_RIGHT_ALIGNMENT;
  adc_base_struct.ordinary_channel_length = 1;
  adc_base_config(ADC1, &adc_base_struct);

  /* adc_ordinary_conversionmode-------------------------------------------- */
  adc_ordinary_channel_set(ADC1, ADC_CHANNEL_1, 1, ADC_SAMPLETIME_239_5);

  /* When "ADCx_ORDINARY_TRIG_SOFTWARE" is selected, user can only use software trigger. \
  The software trigger function is adc_ordinary_software_trigger_enable(ADCx, TRUE); */
  adc_ordinary_conversion_trigger_set(ADC1, ADC12_ORDINARY_TRIG_TMR3TRGOUT, TRUE);

  adc_ordinary_part_mode_enable(ADC1, FALSE);

  adc_dma_mode_enable(ADC1, TRUE);
  /* adc_preempt_conversionmode--------------------------------------------- */
  adc_preempt_channel_length_set(ADC1, 1);

  adc_preempt_channel_set(ADC1, ADC_CHANNEL_16, 1, ADC_SAMPLETIME_239_5);
  adc_preempt_offset_value_set(ADC1, ADC_PREEMPT_CHANNEL_1, 0x0);

  /* When "ADCx_PREEMPT_TRIG_SOFTWARE" is selected, user can only use software trigger. \
  The software trigger function is adc_preempt_software_trigger_enable(ADCx, TRUE); */
  adc_preempt_conversion_trigger_set(ADC1, ADC12_PREEMPT_TRIG_SOFTWARE, TRUE);

  /* add user code begin adc1_init 2 */

  /* add user code end adc1_init 2 */
  
  adc_enable(ADC1, TRUE);
  
  /* adc calibration-------------------------------------------------------- */
  adc_calibration_init(ADC1);
  while(adc_calibration_init_status_get(ADC1));
  adc_calibration_start(ADC1);
  while(adc_calibration_status_get(ADC1));

  /* add user code begin adc1_init 3 */
//  adc_ordinary_software_trigger_enable(ADC1, TRUE);
  /* add user code end adc1_init 3 */
}

/* add user code begin 1 */
/*--------------------------- MCU temperature ------------------------------*/
/* MCU internal temperature sensor (ADC1_IN16) conversion:
 *     T(degC) = 25 + (V25 - VTS) / Slope
 *     VTS = raw * VREF / 4096
 * Constants are the field-verified calibration of the previous project
 * (V25 = 1.26 V, slope = -4.23 mV/degC, VREF = 3.29 V), NOT the datasheet
 * typical values: the datasheet V25 spread is +-200 mV (~+-47 degC), so
 * the measured calibration is what makes the reading sensible.
 * ADCCLK = APB2CLK / 4 = 18 MHz, one conversion = 239.5 + 12.5 = 252 ADCCLK
 * (~14 us), which is the longest sampling window this part offers. */
#define MCU_TS_V25_MV 1260         /* VTS at 25 degC, measured */
#define MCU_TS_SLOPE_MV_X1000 4230 /* 4.23 mV/degC, times 1000 */
#define MCU_TS_VREF_MV 3290        /* VREF+ rail in mV, measured */
#define MCU_TS_ADC_FULL_SCALE 4096 /* 12-bit ADC full scale */
/* Plausible window for the power-up snapshot. Outside it the reading is
 * rejected (mcu_temp_c stays 0, which disables the display lower limit). */
#define MCU_TS_VALID_MIN_X10 0     /* accept 0 < T <= 50 degC */
#define MCU_TS_VALID_MAX_X10 500

#define MCU_TS_SAMPLE_NUM 9U       /* first one is discarded (sample path warm-up) */
#define MCU_TS_GUARD_LOOPS 2000U   /* bounded wait, one conversion is ~14 us */

/**
  * @brief  read the MCU internal temperature sensor once (on demand).
  * @param  temp_c: destination for the temperature in degC
  * @retval 1 = *temp_c updated, 0 = the ADC did not answer or the reading is
  *             outside the plausible window (caller keeps the previous value)
  */
uint8_t wk_adc_mcu_temp_get(int16_t *temp_c)
{
  uint32_t sum = 0U;
  int32_t vts_mv;
  int32_t temp_x10;
  uint16_t guard;
  uint8_t i;

  if (temp_c == NULL)
  {
    return 0U;
  }

  for (i = 0U; i < MCU_TS_SAMPLE_NUM; i++)
  {
    adc_flag_clear(ADC1, ADC_PCCE_FLAG);
    adc_preempt_software_trigger_enable(ADC1, TRUE);

    guard = MCU_TS_GUARD_LOOPS;
    while (adc_flag_get(ADC1, ADC_PCCE_FLAG) == RESET && guard > 0U)
    {
      guard--;
    }
    if (guard == 0U)
    {
      return 0U; /* ADC not answering: keep the previous value */
    }

    if (i > 0U)
    {
      sum += (uint32_t)adc_preempt_conversion_data_get(ADC1, ADC_PREEMPT_CHANNEL_1);
    }
  }
  adc_flag_clear(ADC1, ADC_PCCE_FLAG);

  /* raw -> mV -> degC, integer math, kept in 0.1 degC */
  vts_mv = (int32_t)((sum / (MCU_TS_SAMPLE_NUM - 1U)) * MCU_TS_VREF_MV) /
           MCU_TS_ADC_FULL_SCALE;
  temp_x10 = 250 + ((MCU_TS_V25_MV - vts_mv) * 1000) / MCU_TS_SLOPE_MV_X1000;

  if (temp_x10 <= MCU_TS_VALID_MIN_X10 || temp_x10 > MCU_TS_VALID_MAX_X10)
  {
    return 0U;
  }

  *temp_c = (int16_t)((temp_x10 + 5) / 10);
  return 1U;
}

/* add user code end 1 */

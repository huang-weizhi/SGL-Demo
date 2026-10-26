/* add user code begin Header */
/**
 ******************************************************************************
 * File Name          : freertos_app.c
 * Description        : Code for freertos applications
 */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "freertos_app.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "wk_adc.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#define TASK_INPUT_IDX 1
#define TASK_OUTPUT_IDX 2
#define TASK_COMMON_IDX 3
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */
volatile uint32_t task_life_signs[4] = {0, 0, 0, 0};
/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/* task handler */
TaskHandle_t input_task_handle;
TaskHandle_t output_task_handle;
TaskHandle_t common_task_handle;
TaskHandle_t wdt_task_handle;

/* Idle task control block and stack */
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t idle_task_tcb;
static StaticTask_t timer_task_tcb;

/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );
extern void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize );

/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &idle_task_tcb;
  *ppxIdleTaskStackBuffer = &idle_task_stack[0];
  *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}
/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &timer_task_tcb;
  *ppxTimerTaskStackBuffer = &timer_task_stack[0];
  *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* add user code begin 1 */

/* add user code end 1 */

/**
  * @brief  initializes all task.
  * @param  none
  * @retval none
  */
void freertos_task_create(void)
{
  /* create input_task task */
  xTaskCreate(input_func,
              "input_task",
              128,
              NULL,
              0,
              &input_task_handle);

  /* create output_task task */
  xTaskCreate(output_func,
              "output_task",
              128,
              NULL,
              0,
              &output_task_handle);

  /* create common_task task */
  xTaskCreate(common_func,
              "common_task",
              128,
              NULL,
              0,
              &common_task_handle);

  /* create wdt_task task */
  xTaskCreate(wdt_function,
              "wdt_task",
              128,
              NULL,
              1,
              &wdt_task_handle);
}

/**
  * @brief  freertos init and begin run.
  * @param  none
  * @retval none
  */
void wk_freertos_init(void)
{
  /* add user code begin freertos_init 0 */

  /* add user code end freertos_init 0 */

  /* enter critical */
  taskENTER_CRITICAL();

  freertos_task_create();
	
  /* add user code begin freertos_init 1 */

  /* add user code end freertos_init 1 */

  /* exit critical */
  taskEXIT_CRITICAL();

  /* start scheduler */
  vTaskStartScheduler();
}

/**
  * @brief input_task function.
  * @param  none
  * @retval none
  */
void input_func(void *pvParameters)
{
  /* add user code begin input_func 0 */
  /* Stop the TMR3 trigger BEFORE (re)configuring the DMA channel: the ADC
   * has been converting since wk_tmr3_init() while the DMA channel was
   * still disabled, so a pending DMA request at channel-enable time would
   * be serviced into slot 0 and permanently shift the NTC/TS order in the
   * ring buffer by one sample. */
  tmr_counter_enable(TMR3, FALSE);
  vTaskDelay(2);                    /* let any in-flight conversion finish */
  adc_dma_mode_enable(ADC1, FALSE); /* clears a stale ADC->DMA request line */

  wk_dma_channel_config(DMA1_CHANNEL1,
                        (uint32_t)&ADC1->odt,
                        DMA1_CHANNEL1_MEMORY_BASE_ADDR,
                        DMA1_CHANNEL1_BUFFER_SIZE);
  dma_interrupt_enable(DMA1_CHANNEL1, DMA_FDT_INT, TRUE);
  adc_dma_mode_enable(ADC1, TRUE);
  dma_channel_enable(DMA1_CHANNEL1, TRUE);
  /* one-shot MCU temperature snapshot for the display lower limit. Taken
   * here, before the 1 kHz ordinary stream starts, through the ADC preempted
   * group: the value is captured once per power-up and never shares the DMA
   * ring with the NTC samples. A rejected reading (out of the plausible
   * window) leaves mcu_temp_c at 0, which disables the lower limit. */
  {
    int16_t mcu_temp;
    uint8_t attempt;

    for (attempt = 0U; attempt < 3U; attempt++)
    {
      if (wk_adc_mcu_temp_get(&mcu_temp) != 0U)
      {
        scz01.mcu_temp_c = mcu_temp;
        scz01.mcu_temp_f = 9 * mcu_temp / 5 + 32;
        break;
      }
    }
  }

  tmr_counter_enable(TMR3, TRUE); /* now restart the trigger, DMA ready */
  /* add user code end input_func 0 */

  /* add user code begin input_func 2 */

  /* add user code end input_func 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin input_func 1 */
	task_life_signs[TASK_INPUT_IDX]++;
    input_handle(&scz01);
    vTaskDelay(20);
    /* add user code end input_func 1 */
  }
}


/**
  * @brief output_task function.
  * @param  none
  * @retval none
  */
void output_func(void *pvParameters)
{
  /* add user code begin output_func 0 */
	
	
  lcd_init();
  PID_Init(&handle_pid,30,1,65,MAX_PWM_OUT);     /* pwm = 2hz ,3S output once no over temp but very slow */

  /* add user code end output_func 0 */

  /* add user code begin output_func 2 */
  /* show logo */
  lcd_show_picture(25, 144, 0, spiflash_read);
  /* open lcd bl */
  lcd_port_backlight(1);
  /* delay 1.5 S */
  vTaskDelay(1500);

  /* add user code end output_func 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin output_func 1 */
	task_life_signs[TASK_OUTPUT_IDX]++;
    output_handle(&scz01);
    ui_handle();
    vTaskDelay(20);
    /* add user code end output_func 1 */
  }
}


/**
  * @brief common_task function.
  * @param  none
  * @retval none
  */
void common_func(void *pvParameters)
{
  /* add user code begin common_func 0 */
  iap_init();
  /* init uart1 dma transport (non-blocking tx / lossless rx). */
  uart_init(uart1);
  flash_init();
  /* add user code end common_func 0 */

  /* add user code begin common_func 2 */

  /* add user code end common_func 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin common_func 1 */
	task_life_signs[TASK_COMMON_IDX]++;
    pc_handle();
    iap_command_handle();
    flash_save();
    vTaskDelay(5);

    /* add user code end common_func 1 */
  }
}


/**
  * @brief wdt_task function.
  * @param  none
  * @retval none
  */
void wdt_function(void *pvParameters)
{
  /* add user code begin wdt_function 0 */
  uint32_t last_signs[4] = {0};
  while (!scz01.init_flag)
  {
    vTaskDelay(2000);
  }
  /* add user code end wdt_function 0 */

  /* add user code begin wdt_function 2 */
   wdt_enable();
  /* add user code end wdt_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin wdt_function 1 */
    vTaskDelay(400);
    taskENTER_CRITICAL();
    int all_alive = 1;

    for (int i = 1; i <= 4; i++)
    {
      if (task_life_signs[i] == last_signs[i])
      {
        all_alive = 0;
        break;
      }
    }

    if (all_alive)
    {
      for (int i = 1; i <= 4; i++)
      {
        last_signs[i] = task_life_signs[i];
      }

      wdt_counter_reload();
    }

    taskEXIT_CRITICAL();
  /* add user code end wdt_function 1 */
  }
}


/* add user code begin 2 */

/* add user code end 2 */


#ifndef __INF_UART_H__
#define __INF_UART_H__

#include "stdint.h"

/*******************************************************************************
 * UART DMA transport layer - multi instance
 *
 * Each enabled instance owns:
 *   - one RX DMA channel in circular mode, drained by HDT/FDT/IDLE ISRs
 *   - one TX DMA channel in normal mode, chunked by the FDT ISR
 *   - an RX ring buffer (ISR produces, task consumes, SPSC)
 *   - a  TX ring buffer (task produces, DMA consumes, SPSC)
 *
 * - TX: non-blocking, data is copied into a ring buffer and pushed out by
 *   DMA in the background
 * - RX: no byte is lost as long as the application drains the ring buffer
 *   before it overflows
 * - no protocol here: the application builds/parses its own frames on top
 *   of uart_send() / uart_read()
 *
 * Everything tunable lives in the macro blocks below, so adding, removing
 * or resizing an instance never requires touching the driver body.
 *
 * Typical usage:
 *     uart_init(uart1);
 *     uart_send(uart1, buf, len);
 *     if (uart_rx_available(uart2)) { uart_read(uart2, buf, sizeof(buf)); }
 ******************************************************************************/

/* ============================================================================
 * Global feature switches
 * ========================================================================== */

/* 1 = wake the protocol task on new RX data via FreeRTOS task notification
 *     (event driven, register the task with uart_set_notify_task())
 * 0 = polling only with uart_rx_available()/uart_read()
 * The notification path is disabled by default, turn the macro on when needed */
#ifndef UART_USE_TASK_NOTIFY
#define UART_USE_TASK_NOTIFY   0
#endif

#if UART_USE_TASK_NOTIFY
#include "FreeRTOS.h"
#include "task.h"
#endif

/* ============================================================================
 * Per-instance enable switch
 *
 * Set to 1 only for the UARTs that are actually used. Each enabled instance
 * costs RAM (RX DMA + RX ring + TX ring + a small handle) and three vectors.
 * ========================================================================== */
#define UART1_ENABLE    1
#define UART2_ENABLE    0
#define UART3_ENABLE    0

/* ============================================================================
 * Instance 1 - USART1
 *
 * The DMA request mapping must follow the AT32F415 DMA request table.
 * The USART itself has to be initialised by WorkBench before uart_init().
 * ========================================================================== */
#if UART1_ENABLE
#define UART1_PERIPH             USART1
#define UART1_IRQN               USART1_IRQn

#define UART1_RX_DMA             DMA1_CHANNEL5
#define UART1_TX_DMA             DMA1_CHANNEL6
#define UART1_RX_DMA_IRQN        DMA1_Channel5_IRQn
#define UART1_TX_DMA_IRQN        DMA1_Channel6_IRQn

#define UART1_RX_DMA_HDT_FLAG    DMA1_HDT5_FLAG
#define UART1_RX_DMA_FDT_FLAG    DMA1_FDT5_FLAG
#define UART1_RX_DMA_DTERR_FLAG  DMA1_DTERR5_FLAG
#define UART1_TX_DMA_FDT_FLAG    DMA1_FDT6_FLAG

#define UART1_RX_DMA_SIZE        512U    /* DMA circular buffer, keep a power of 2 */
#define UART1_RX_RING_SIZE       4096U   /* RX ring, >= 2x the largest frame       */
#define UART1_TX_RING_SIZE       512U    /* TX queue, >= 2x the largest send chunk */
#endif

/* ============================================================================
 * Instance 2 - USART2
 * ========================================================================== */
#if UART2_ENABLE
#define UART2_PERIPH             USART2
#define UART2_IRQN               USART2_IRQn

#define UART2_RX_DMA             DMA1_CHANNEL7
#define UART2_TX_DMA             DMA1_CHANNEL1
#define UART2_RX_DMA_IRQN        DMA1_Channel7_IRQn
#define UART2_TX_DMA_IRQN        DMA1_Channel1_IRQn

#define UART2_RX_DMA_HDT_FLAG    DMA1_HDT7_FLAG
#define UART2_RX_DMA_FDT_FLAG    DMA1_FDT7_FLAG
#define UART2_RX_DMA_DTERR_FLAG  DMA1_DTERR7_FLAG
#define UART2_TX_DMA_FDT_FLAG    DMA1_FDT1_FLAG

#define UART2_RX_DMA_SIZE        256U
#define UART2_RX_RING_SIZE       2048U
#define UART2_TX_RING_SIZE       256U
#endif

/* ============================================================================
 * Instance 3 - USART3
 * ========================================================================== */
#if UART3_ENABLE
#define UART3_PERIPH             USART3
#define UART3_IRQN               USART3_IRQn

#define UART3_RX_DMA             DMA1_CHANNEL3
#define UART3_TX_DMA             DMA1_CHANNEL2
#define UART3_RX_DMA_IRQN        DMA1_Channel3_IRQn
#define UART3_TX_DMA_IRQN        DMA1_Channel2_IRQn

#define UART3_RX_DMA_HDT_FLAG    DMA1_HDT3_FLAG
#define UART3_RX_DMA_FDT_FLAG    DMA1_FDT3_FLAG
#define UART3_RX_DMA_DTERR_FLAG  DMA1_DTERR3_FLAG
#define UART3_TX_DMA_FDT_FLAG    DMA1_FDT2_FLAG

#define UART3_RX_DMA_SIZE        256U
#define UART3_RX_RING_SIZE       2048U
#define UART3_TX_RING_SIZE       256U
#endif

/* ============================================================================
 * Instance handle
 *
 * The handle is opaque, only its pointer is passed to the driver API.
 * ========================================================================== */
typedef struct uart_dev_s uart_dev_t;

#if UART1_ENABLE
extern uart_dev_t *const uart1;
#endif
#if UART2_ENABLE
extern uart_dev_t *const uart2;
#endif
#if UART3_ENABLE
extern uart_dev_t *const uart3;
#endif

/* ============================================================================
 * Public API
 * ========================================================================== */

/* Init the DMA transport of one instance. Call ONCE per instance after the
 * WorkBench peripheral (USART + RX/TX DMA channels) has been configured. */
void uart_init(uart_dev_t *dev);

/* Non-blocking send: copies as many bytes as fit into the TX ring buffer,
 * returns the number of bytes accepted (may be less than len when full) */
uint16_t uart_send(uart_dev_t *dev, const uint8_t *buf, uint16_t len);

/* Blocking send: loops until ALL bytes are queued into the TX ring buffer
 * (the DMA still drains the queue asynchronously) */
void uart_send_blocking(uart_dev_t *dev, const uint8_t *buf, uint16_t len);

/* 1 = DMA still draining the TX queue */
uint8_t uart_tx_busy(uart_dev_t *dev);

/* Number of bytes waiting in the RX ring buffer */
uint16_t uart_rx_available(uart_dev_t *dev);

/* Pop up to len bytes from the RX ring buffer, returns bytes actually read */
uint16_t uart_read(uart_dev_t *dev, uint8_t *buf, uint16_t len);

/* Pop one byte, valid only when uart_rx_available() > 0 */
uint8_t uart_read_byte(uart_dev_t *dev);

/* Total bytes dropped because the RX ring buffer was full */
uint32_t uart_rx_overflow_count(uart_dev_t *dev);

#if UART_USE_TASK_NOTIFY
/* Register the task to be woken on new RX data. Call once from that task.
 * With this set, uart_read() should be called right after a successful
 * ulTaskNotifyTake(). Leaving it unset keeps pure polling mode. */
void uart_set_notify_task(uart_dev_t *dev, TaskHandle_t task);
#endif

/* --- internal hooks, called from the at32f415_int.c user code sections --- */
void uart_rx_dma_isr(uart_dev_t *dev);     /* RX DMA half/full transfer: drain new bytes */
void uart_rx_dma_error(uart_dev_t *dev);   /* RX DMA error: restart the circular RX DMA  */
void uart_tx_dma_isr(uart_dev_t *dev);     /* TX DMA chunk finished: kick the next chunk */
void uart_idle_isr(uart_dev_t *dev);       /* USART idle line: collect the tail of a frame */

#endif /* __INF_UART_H__ */
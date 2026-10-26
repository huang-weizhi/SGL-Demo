#include "Inf_uart.h"
#include "wk_usart.h"
#include "wk_dma.h"

/*******************************************************************************
 * UART DMA transport layer - multi instance implementation
 *
 * Per-instance resources (buffers, states, handles) are generated from the
 * macro blocks declared in Inf_uart.h, so the code below stays fully generic.
 ******************************************************************************/

/* ============================================================================
 * Device object
 * ========================================================================== */

struct uart_dev_s
{
    /* ---- static resources, filled at compile time ---- */
    usart_type       *usart;
    dma_channel_type *rx_dma;
    dma_channel_type *tx_dma;

    uint8_t  *rx_dma_buf;
    uint16_t  rx_dma_size;

    uint8_t  *rx_ring;
    uint16_t  rx_ring_size;

    uint8_t  *tx_ring;
    uint16_t  tx_ring_size;

    IRQn_Type rx_dma_irqn;
    IRQn_Type tx_dma_irqn;
    IRQn_Type usart_irqn;

    uint32_t  rx_dma_hdt_flag;
    uint32_t  rx_dma_fdt_flag;
    uint32_t  rx_dma_dterr_flag;
    uint32_t  tx_dma_fdt_flag;

    /* ---- RX runtime state (ISR writes, uart_read() consumes, SPSC) ---- */
    volatile uint16_t rx_dma_pos;   /* next position to copy in rx_dma_buf */
    volatile uint16_t rx_write;     /* written by ISRs                    */
    volatile uint16_t rx_read;      /* written by uart_read()             */
    volatile uint32_t rx_overflow;  /* dropped byte counter               */

    /* ---- TX runtime state (uart_send() produces, DMA consumes) ---- */
    volatile uint16_t tx_write;
    volatile uint16_t tx_read;      /* advances only when a DMA chunk completes */
    volatile uint8_t  tx_busy;      /* a DMA chunk is in flight                */
    volatile uint16_t tx_busy_len;  /* bytes of the in-flight chunk            */

#if UART_USE_TASK_NOTIFY
    TaskHandle_t notify_task;
#endif
};

/* ============================================================================
 * Instance storage - one block per enabled UART
 * ========================================================================== */

#if UART1_ENABLE
static uint8_t uart1_rx_dma_buf[UART1_RX_DMA_SIZE];
static uint8_t uart1_rx_ring[UART1_RX_RING_SIZE];
static uint8_t uart1_tx_ring[UART1_TX_RING_SIZE];

static struct uart_dev_s uart1_dev =
{
    .usart             = UART1_PERIPH,
    .rx_dma            = UART1_RX_DMA,
    .tx_dma            = UART1_TX_DMA,

    .rx_dma_buf        = uart1_rx_dma_buf,
    .rx_dma_size       = UART1_RX_DMA_SIZE,

    .rx_ring           = uart1_rx_ring,
    .rx_ring_size      = UART1_RX_RING_SIZE,

    .tx_ring           = uart1_tx_ring,
    .tx_ring_size      = UART1_TX_RING_SIZE,

    .rx_dma_irqn       = UART1_RX_DMA_IRQN,
    .tx_dma_irqn       = UART1_TX_DMA_IRQN,
    .usart_irqn        = UART1_IRQN,

    .rx_dma_hdt_flag   = UART1_RX_DMA_HDT_FLAG,
    .rx_dma_fdt_flag   = UART1_RX_DMA_FDT_FLAG,
    .rx_dma_dterr_flag = UART1_RX_DMA_DTERR_FLAG,
    .tx_dma_fdt_flag   = UART1_TX_DMA_FDT_FLAG,
};

uart_dev_t *const uart1 = &uart1_dev;
#endif

#if UART2_ENABLE
static uint8_t uart2_rx_dma_buf[UART2_RX_DMA_SIZE];
static uint8_t uart2_rx_ring[UART2_RX_RING_SIZE];
static uint8_t uart2_tx_ring[UART2_TX_RING_SIZE];

static struct uart_dev_s uart2_dev =
{
    .usart             = UART2_PERIPH,
    .rx_dma            = UART2_RX_DMA,
    .tx_dma            = UART2_TX_DMA,

    .rx_dma_buf        = uart2_rx_dma_buf,
    .rx_dma_size       = UART2_RX_DMA_SIZE,

    .rx_ring           = uart2_rx_ring,
    .rx_ring_size      = UART2_RX_RING_SIZE,

    .tx_ring           = uart2_tx_ring,
    .tx_ring_size      = UART2_TX_RING_SIZE,

    .rx_dma_irqn       = UART2_RX_DMA_IRQN,
    .tx_dma_irqn       = UART2_TX_DMA_IRQN,
    .usart_irqn        = UART2_IRQN,

    .rx_dma_hdt_flag   = UART2_RX_DMA_HDT_FLAG,
    .rx_dma_fdt_flag   = UART2_RX_DMA_FDT_FLAG,
    .rx_dma_dterr_flag = UART2_RX_DMA_DTERR_FLAG,
    .tx_dma_fdt_flag   = UART2_TX_DMA_FDT_FLAG,
};

uart_dev_t *const uart2 = &uart2_dev;
#endif

#if UART3_ENABLE
static uint8_t uart3_rx_dma_buf[UART3_RX_DMA_SIZE];
static uint8_t uart3_rx_ring[UART3_RX_RING_SIZE];
static uint8_t uart3_tx_ring[UART3_TX_RING_SIZE];

static struct uart_dev_s uart3_dev =
{
    .usart             = UART3_PERIPH,
    .rx_dma            = UART3_RX_DMA,
    .tx_dma            = UART3_TX_DMA,

    .rx_dma_buf        = uart3_rx_dma_buf,
    .rx_dma_size       = UART3_RX_DMA_SIZE,

    .rx_ring           = uart3_rx_ring,
    .rx_ring_size      = UART3_RX_RING_SIZE,

    .tx_ring           = uart3_tx_ring,
    .tx_ring_size      = UART3_TX_RING_SIZE,

    .rx_dma_irqn       = UART3_RX_DMA_IRQN,
    .tx_dma_irqn       = UART3_TX_DMA_IRQN,
    .usart_irqn        = UART3_IRQN,

    .rx_dma_hdt_flag   = UART3_RX_DMA_HDT_FLAG,
    .rx_dma_fdt_flag   = UART3_RX_DMA_FDT_FLAG,
    .rx_dma_dterr_flag = UART3_RX_DMA_DTERR_FLAG,
    .tx_dma_fdt_flag   = UART3_TX_DMA_FDT_FLAG,
};

uart_dev_t *const uart3 = &uart3_dev;
#endif

/* ============================================================================
 * Task notification helper
 * ========================================================================== */

#if UART_USE_TASK_NOTIFY
/* Wake the protocol task registered on this instance (ISR context) */
static void uart_notify_task_from_isr(uart_dev_t *dev)
{
    BaseType_t woken = pdFALSE;

    if (dev->notify_task != 0)
    {
        vTaskNotifyGiveFromISR(dev->notify_task, &woken);
        portYIELD_FROM_ISR(woken);
    }
}
#else
/* notification path disabled: compile to nothing */
static void uart_notify_task_from_isr(uart_dev_t *dev)
{
    (void)dev;
}
#endif

/*******************************************************************************
 * RX engine
 ******************************************************************************/

/* Move all new bytes from the DMA circular buffer into the RX ring buffer.
 * Called from the USART IDLE ISR and the RX DMA ISR; both share the same
 * preemption priority so they never preempt each other. */
static void uart_rx_dma_to_ring(uart_dev_t *dev)
{
    uint16_t pos;
    uint16_t new_bytes;

    pos = (uint16_t)(dev->rx_dma_size - (uint16_t)dev->rx_dma->dtcnt);
    new_bytes = (uint16_t)(pos - dev->rx_dma_pos + dev->rx_dma_size) % dev->rx_dma_size;

    while (new_bytes--)
    {
        uint16_t next = (uint16_t)(dev->rx_write + 1) % dev->rx_ring_size;

        if (next != dev->rx_read)
        {
            dev->rx_ring[dev->rx_write] = dev->rx_dma_buf[dev->rx_dma_pos];
            dev->rx_write = next;
        }
        else
        {
            /* ring full: drop the newest byte and count it */
            dev->rx_overflow++;
        }

        dev->rx_dma_pos = (uint16_t)(dev->rx_dma_pos + 1) % dev->rx_dma_size;
    }
}

/* HDT / FDT: drain whatever arrived so far, then wake the protocol task */
void uart_rx_dma_isr(uart_dev_t *dev)
{
    uart_rx_dma_to_ring(dev);
    uart_notify_task_from_isr(dev);
}

/* DTERR: restart the circular RX transfer with our buffer */
void uart_rx_dma_error(uart_dev_t *dev)
{
    dma_channel_enable(dev->rx_dma, FALSE);
    dev->rx_dma->dtcnt = dev->rx_dma_size;
    dev->rx_dma->maddr = (uint32_t)dev->rx_dma_buf;
    dma_channel_enable(dev->rx_dma, TRUE);

    uart_rx_dma_to_ring(dev);
}

/* IDLE line: a frame gap was detected, collect the tail bytes of the frame */
void uart_idle_isr(uart_dev_t *dev)
{
    uart_rx_dma_to_ring(dev);
    uart_notify_task_from_isr(dev);
}

/*******************************************************************************
 * TX engine
 ******************************************************************************/

/* Start a new DMA chunk from the ring buffer if idle, called with IRQs off */
static void uart_tx_kick(uart_dev_t *dev)
{
    uint16_t unread;

    if (dev->tx_busy)
    {
        return;
    }

    unread = (uint16_t)(dev->tx_write - dev->tx_read + dev->tx_ring_size) % dev->tx_ring_size;

    if (unread == 0)
    {
        return;
    }

    /* chunk = contiguous run up to the end of the ring buffer */
    dev->tx_busy_len = (uint16_t)(dev->tx_ring_size - dev->tx_read);

    if (dev->tx_busy_len > unread)
    {
        dev->tx_busy_len = unread;
    }

    dev->tx_busy = 1;

    dma_channel_enable(dev->tx_dma, FALSE);
    dev->tx_dma->dtcnt = dev->tx_busy_len;
    dev->tx_dma->maddr = (uint32_t)&dev->tx_ring[dev->tx_read];
    dma_flag_clear(dev->tx_dma_fdt_flag);
    dma_channel_enable(dev->tx_dma, TRUE);
}

/* FDT: a chunk left the ring, release it and start the next one */
void uart_tx_dma_isr(uart_dev_t *dev)
{
    dev->tx_read = (uint16_t)(dev->tx_read + dev->tx_busy_len) % dev->tx_ring_size;
    dev->tx_busy = 0;

    uart_tx_kick(dev);
}

/*******************************************************************************
 * Public API
 ******************************************************************************/

void uart_init(uart_dev_t *dev)
{
    /* start the RX circular transfer with our buffer */
    dma_channel_enable(dev->rx_dma, FALSE);
    dev->rx_dma->dtcnt = dev->rx_dma_size;
    dev->rx_dma->maddr = (uint32_t)dev->rx_dma_buf;
    dev->rx_dma->paddr = (uint32_t)&dev->usart->dt;
    dma_flag_clear(dev->rx_dma_fdt_flag |
                   dev->rx_dma_hdt_flag |
                   dev->rx_dma_dterr_flag);
    dma_channel_enable(dev->rx_dma, TRUE);

    /* the TX channel stays disabled until the first uart_send() kicks it */

    /* RX ISRs (RX DMA + USART IDLE) must not preempt each other:
     * same preemption priority, only the sub priority differs */
    nvic_irq_enable(dev->rx_dma_irqn, 1, 1);
    nvic_irq_enable(dev->tx_dma_irqn, 1, 2);
    nvic_irq_enable(dev->usart_irqn,  1, 0);
}

uint16_t uart_send(uart_dev_t *dev, const uint8_t *buf, uint16_t len)
{
    uint16_t sent = 0;
    uint32_t primask;

    while (sent < len)
    {
        uint16_t free_space;
        uint16_t n;
        uint16_t to_end;
        uint16_t i;

        free_space = (uint16_t)(dev->tx_ring_size - 1) -
                     ((uint16_t)(dev->tx_write - dev->tx_read + dev->tx_ring_size) % dev->tx_ring_size);

        if (free_space == 0)
        {
            break;
        }

        n = (uint16_t)(len - sent);

        if (n > free_space)
        {
            n = free_space;
        }

        /* do not wrap inside one copy loop */
        to_end = (uint16_t)(dev->tx_ring_size - dev->tx_write);

        if (n > to_end)
        {
            n = to_end;
        }

        for (i = 0; i < n; i++)
        {
            dev->tx_ring[(dev->tx_write + i) % dev->tx_ring_size] = buf[sent + i];
        }

        dev->tx_write = (uint16_t)(dev->tx_write + n) % dev->tx_ring_size;
        sent += n;

        /* start the DMA if idle (critical section against the FDT ISR) */
        primask = __get_PRIMASK();
        __disable_irq();
        uart_tx_kick(dev);

        if (!primask)
        {
            __enable_irq();
        }
    }

    return sent;
}

void uart_send_blocking(uart_dev_t *dev, const uint8_t *buf, uint16_t len)
{
    uint16_t sent = 0;

    while (sent < len)
    {
        sent += uart_send(dev, buf + sent, (uint16_t)(len - sent));
    }
}

uint8_t uart_tx_busy(uart_dev_t *dev)
{
    return dev->tx_busy;
}

uint16_t uart_rx_available(uart_dev_t *dev)
{
    return (uint16_t)(dev->rx_write - dev->rx_read + dev->rx_ring_size) % dev->rx_ring_size;
}

uint16_t uart_read(uart_dev_t *dev, uint8_t *buf, uint16_t len)
{
    uint16_t n = 0;

    while (n < len && dev->rx_read != dev->rx_write)
    {
        buf[n++] = dev->rx_ring[dev->rx_read];
        dev->rx_read = (uint16_t)(dev->rx_read + 1) % dev->rx_ring_size;
    }

    return n;
}

uint8_t uart_read_byte(uart_dev_t *dev)
{
    uint8_t dat = 0;

    if (dev->rx_read != dev->rx_write)
    {
        dat = dev->rx_ring[dev->rx_read];
        dev->rx_read = (uint16_t)(dev->rx_read + 1) % dev->rx_ring_size;
    }

    return dat;
}

uint32_t uart_rx_overflow_count(uart_dev_t *dev)
{
    return dev->rx_overflow;
}

#if UART_USE_TASK_NOTIFY
void uart_set_notify_task(uart_dev_t *dev, TaskHandle_t task)
{
    dev->notify_task = task;
}
#endif
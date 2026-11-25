#include "inf_sgl_port.h"
#include <stddef.h>
#include <stdbool.h>

/* Two partial framebuffers: 2 * 240 * 4 pixels * 2 B = 7680 B.
 * The flush does not wait for the DMA: it returns as soon as the transfer is
 * kicked off, so SGL renders the next slice into the other buffer while the
 * previous one is still shifting out over SPI. That overlap is what makes
 * double buffering faster than a single synchronous buffer on this panel
 * (measured 23 FPS vs 15 FPS). */
static sgl_color_t pfb[2][240 * 4];

static bool prev_in_flight = false;

static void sgl_port_flush(sgl_area_t *area, sgl_color_t *src)
{
    uint16_t w = area->x2 - area->x1 + 1;
    uint16_t h = area->y2 - area->y1 + 1;

    if (prev_in_flight) {
        lcd_blit_wait();             /* wait for the previous DMA, release CS */
        sgl_fbdev_flush_ready();     /* return the buffer freed by that DMA */
    }

    lcd_blit_start(area->x1, area->y1, w, h, (const uint8_t *)src);
    prev_in_flight = true;
}

/* The last slice of a frame kicks its DMA and returns, so nobody ever waits
 * for that transfer. Call this once after sgl_task_handler() returns: it
 * releases CS, gives the last buffer back to SGL and clears the flag, so the
 * bus and fb_ready converge instead of staying dirty while the UI is idle.
 * The DMA has already finished by then, so this adds no measurable frame
 * time. On an idle frame (no dirty area, no flush) it is a no-op. */
void sgl_port_flush_finish(void)
{
    if (!prev_in_flight) {
        return;
    }

    lcd_blit_wait();
    sgl_fbdev_flush_ready();
    prev_in_flight = false;
}

void sgl_port_init(void)
{
    sgl_fbdev_register_dev(pfb[0], pfb[1], 240 * 4, 240, 320, sgl_port_flush);
}

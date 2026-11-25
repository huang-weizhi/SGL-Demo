#ifndef __INF_SGL_PORT_H
#define __INF_SGL_PORT_H

#include "at32f415.h"

#include "sgl.h"
//#include "sgl_bench.h"
#include "ui.h"
#include "Inf_lcd.h"
#include "Inf_lcd_dev.h"
#include "Inf_lcd_port.h"

void sgl_port_init(void);
void sgl_port_flush_finish(void);
static void sgl_port_flush(sgl_area_t *area, sgl_color_t *src);
#endif


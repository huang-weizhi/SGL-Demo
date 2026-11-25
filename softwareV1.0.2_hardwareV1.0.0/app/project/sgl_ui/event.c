/* SGL Auto Generated Events */
#include "ui.h"

// button0 控件的 Released 事件处理
sgl_weak_fn void button0_released(sgl_event_t *e)
{

   SGL_LOG_INFO("button0 released");
    // Action 1: ChangeScreen
    ui_screen_load(UI_SCREEN1);
}

// button0 的事件分发路由函数
void button0_event_handler(sgl_event_t *event) {
    switch(event->type) {
        case SGL_EVENT_RELEASED:
            button0_released(event);
            break;
        default: break;
    }
}

// button3 控件的 Released 事件处理
sgl_weak_fn void button3_released(sgl_event_t *e)
{

   SGL_LOG_INFO("button3 released");
    // Action 1: ChangeScreen
    ui_screen_load(UI_SCREEN0);
}

// button3 的事件分发路由函数
void button3_event_handler(sgl_event_t *event) {
    switch(event->type) {
        case SGL_EVENT_RELEASED:
            button3_released(event);
            break;
        default: break;
    }
}


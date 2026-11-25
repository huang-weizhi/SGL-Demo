#include "ui.h"


sgl_obj_t* ui_screen0_init(void) {
    // --- screen0 ---
    screen0 = sgl_rect_create(NULL);
    sgl_obj_set_size(screen0, 240, 320);
    sgl_obj_set_pos(screen0, 0, 0);
    sgl_rect_set_color(screen0, sgl_rgb(0x7F, 0xFF, 0xD4));
    sgl_rect_set_alpha(screen0, 255);
    sgl_rect_set_radius(screen0, 0);
    sgl_rect_set_border_width(screen0, 0);
    sgl_rect_set_border_color(screen0, sgl_rgb(0x00, 0x00, 0x00));
    // Children of screen0
    // --- button0 ---
    button0 = sgl_button_create(screen0);
    sgl_obj_set_size(button0, 50, 24);
    sgl_obj_set_pos(button0, 95, 196);
    sgl_button_set_color(button0, sgl_rgb(0x02, 0x88, 0xD1));
    sgl_button_set_text(button0, "Button");
    sgl_button_set_font(button0, &consolas14);
    sgl_button_set_text_color(button0, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_button_set_alpha(button0, 255);
    sgl_button_set_radius(button0, 4);
    sgl_button_set_border_width(button0, 1);
    sgl_button_set_border_color(button0, sgl_rgb(0x02, 0x88, 0xD1));
    sgl_obj_set_clickable(button0);
    sgl_obj_set_event_cb(button0, button0_event_handler,NULL);
    // --- label0 ---
    label0 = sgl_label_create(screen0);
    sgl_obj_set_size(label0, 130, 20);
    sgl_obj_set_pos(label0, 55, 112);
    sgl_label_set_text(label0, "This Is Page 1");
    sgl_label_set_font(label0, &consolas14);
    sgl_label_set_text_color(label0, sgl_rgb(0xDC, 0x14, 0x3C));
    sgl_label_set_text_align(label0, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label0, 255);
    sgl_label_set_radius(label0, 4);
    // --- label2 ---
    label2 = sgl_label_create(screen0);
    sgl_obj_set_size(label2, 40, 20);
    sgl_obj_set_pos(label2, 100, 56);
    sgl_label_set_text(label2, "WoW");
    sgl_label_set_font(label2, &consolas14);
    sgl_label_set_text_color(label2, sgl_rgb(0xBF, 0x5E, 0x6C));
    sgl_label_set_text_align(label2, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label2, 255);
    sgl_label_set_radius(label2, 4);

  return screen0;
}

#include "ui.h"


sgl_obj_t* ui_screen1_init(void) {
    // --- screen1 ---
    screen1 = sgl_rect_create(NULL);
    sgl_obj_set_size(screen1, 240, 320);
    sgl_obj_set_pos(screen1, 0, 0);
    sgl_rect_set_color(screen1, sgl_rgb(0x41, 0x69, 0xE1));
    sgl_rect_set_alpha(screen1, 255);
    sgl_rect_set_radius(screen1, 0);
    sgl_rect_set_border_width(screen1, 0);
    sgl_rect_set_border_color(screen1, sgl_rgb(0x00, 0x00, 0x00));
    // Children of screen1
    // --- label1 ---
    label1 = sgl_label_create(screen1);
    sgl_obj_set_size(label1, 130, 20);
    sgl_obj_set_pos(label1, 55, 112);
    sgl_label_set_text(label1, "This Is Page 2");
    sgl_label_set_font(label1, &consolas14);
    sgl_label_set_text_color(label1, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label1, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label1, 255);
    sgl_label_set_radius(label1, 4);
    // --- button3 ---
    button3 = sgl_button_create(screen1);
    sgl_obj_set_size(button3, 50, 24);
    sgl_obj_set_pos(button3, 95, 196);
    sgl_button_set_color(button3, sgl_rgb(0x02, 0x88, 0xD1));
    sgl_button_set_text(button3, "Button");
    sgl_button_set_font(button3, &consolas14);
    sgl_button_set_text_color(button3, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_button_set_alpha(button3, 255);
    sgl_button_set_radius(button3, 4);
    sgl_button_set_border_width(button3, 1);
    sgl_button_set_border_color(button3, sgl_rgb(0x02, 0x88, 0xD1));
    sgl_obj_set_clickable(button3);
    sgl_obj_set_event_cb(button3, button3_event_handler,NULL);

  return screen1;
}

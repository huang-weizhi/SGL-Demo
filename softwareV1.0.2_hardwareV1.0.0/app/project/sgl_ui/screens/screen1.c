#include "ui.h"


sgl_obj_t* ui_screen1_init(void) {
    // --- screen1 ---
    screen1 = sgl_rect_create(NULL);
    sgl_obj_set_size(screen1, 240, 320);
    sgl_obj_set_pos(screen1, 0, 0);
    sgl_rect_set_color(screen1, sgl_rgb(0x01, 0x01, 0x01));
    sgl_rect_set_alpha(screen1, 255);
    sgl_rect_set_radius(screen1, 0);
    sgl_rect_set_border_width(screen1, 0);
    sgl_rect_set_border_color(screen1, sgl_rgb(0x00, 0x00, 0x00));
    // Children of screen1
    // --- label1 ---
    label1 = sgl_label_create(screen1);
    sgl_obj_set_size(label1, 240, 20);
    sgl_obj_set_pos(label1, 0, 0);
    sgl_label_set_text(label1, " Model:FW-CZ01");
    sgl_label_set_font(label1, &consolas14);
    sgl_label_set_text_color(label1, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_bg_color(label1, sgl_rgb(0x34, 0x34, 0x34));
    sgl_label_set_text_align(label1, SGL_ALIGN_LEFT_MID);
    sgl_label_set_alpha(label1, 255);
    // --- img_ext0 ---
    img_ext0 = sgl_img_ext_create(screen1);
    sgl_obj_set_size(img_ext0, 240, 29);
    sgl_obj_set_pos(img_ext0, 0, 291);
    // --- rectangle4 ---
    rectangle4 = sgl_rect_create(screen1);
    sgl_obj_set_size(rectangle4, 193, 32);
    sgl_obj_set_pos(rectangle4, 24, 30);
    sgl_rect_set_color(rectangle4, sgl_rgb(0x2F, 0x2F, 0x2F));
    sgl_rect_set_alpha(rectangle4, 255);
    sgl_rect_set_radius(rectangle4, 8);
    sgl_rect_set_border_width(rectangle4, 2);
    sgl_rect_set_border_color(rectangle4, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle4, 255);
    // Children of rectangle4
    // --- label11 ---
    label11 = sgl_label_create(rectangle4);
    sgl_obj_set_size(label11, 100, 20);
    sgl_obj_set_pos(label11, 47, 8);
    sgl_label_set_text(label11, "page select");
    sgl_label_set_font(label11, &consolas14);
    sgl_label_set_text_color(label11, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label11, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label11, 255);
    sgl_label_set_radius(label11, 4);
    // --- rectangle5 ---
    rectangle5 = sgl_rect_create(screen1);
    sgl_obj_set_size(rectangle5, 193, 32);
    sgl_obj_set_pos(rectangle5, 24, 84);
    sgl_rect_set_color(rectangle5, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle5, 255);
    sgl_rect_set_radius(rectangle5, 8);
    sgl_rect_set_border_width(rectangle5, 2);
    sgl_rect_set_border_color(rectangle5, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle5, 255);
    // Children of rectangle5
    // --- label12 ---
    label12 = sgl_label_create(rectangle5);
    sgl_obj_set_size(label12, 100, 20);
    sgl_obj_set_pos(label12, 47, 8);
    sgl_label_set_text(label12, "temp unit");
    sgl_label_set_font(label12, &consolas14);
    sgl_label_set_text_color(label12, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label12, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label12, 255);
    sgl_label_set_radius(label12, 4);
    // --- rectangle6 ---
    rectangle6 = sgl_rect_create(screen1);
    sgl_obj_set_size(rectangle6, 193, 32);
    sgl_obj_set_pos(rectangle6, 23, 139);
    sgl_rect_set_color(rectangle6, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle6, 255);
    sgl_rect_set_radius(rectangle6, 8);
    sgl_rect_set_border_width(rectangle6, 2);
    sgl_rect_set_border_color(rectangle6, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle6, 255);
    // Children of rectangle6
    // --- label13 ---
    label13 = sgl_label_create(rectangle6);
    sgl_obj_set_size(label13, 100, 20);
    sgl_obj_set_pos(label13, 47, 8);
    sgl_label_set_text(label13, "temp cal");
    sgl_label_set_font(label13, &consolas14);
    sgl_label_set_text_color(label13, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label13, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label13, 255);
    sgl_label_set_radius(label13, 4);
    // --- rectangle7 ---
    rectangle7 = sgl_rect_create(screen1);
    sgl_obj_set_size(rectangle7, 193, 32);
    sgl_obj_set_pos(rectangle7, 24, 194);
    sgl_rect_set_color(rectangle7, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle7, 255);
    sgl_rect_set_radius(rectangle7, 8);
    sgl_rect_set_border_width(rectangle7, 2);
    sgl_rect_set_border_color(rectangle7, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle7, 255);
    // Children of rectangle7
    // --- label14 ---
    label14 = sgl_label_create(rectangle7);
    sgl_obj_set_size(label14, 100, 20);
    sgl_obj_set_pos(label14, 47, 8);
    sgl_label_set_text(label14, "language");
    sgl_label_set_font(label14, &consolas14);
    sgl_label_set_text_color(label14, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label14, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label14, 255);
    sgl_label_set_radius(label14, 4);
    // --- rectangle8 ---
    rectangle8 = sgl_rect_create(screen1);
    sgl_obj_set_size(rectangle8, 193, 32);
    sgl_obj_set_pos(rectangle8, 24, 248);
    sgl_rect_set_color(rectangle8, sgl_rgb(0x31, 0x31, 0x31));
    sgl_rect_set_alpha(rectangle8, 255);
    sgl_rect_set_radius(rectangle8, 8);
    sgl_rect_set_border_width(rectangle8, 2);
    sgl_rect_set_border_color(rectangle8, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle8, 255);
    // Children of rectangle8
    // --- label15 ---
    label15 = sgl_label_create(rectangle8);
    sgl_obj_set_size(label15, 100, 20);
    sgl_obj_set_pos(label15, 47, 8);
    sgl_label_set_text(label15, "speaker");
    sgl_label_set_font(label15, &consolas14);
    sgl_label_set_text_color(label15, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label15, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label15, 255);
    sgl_label_set_radius(label15, 4);

  return screen1;
}

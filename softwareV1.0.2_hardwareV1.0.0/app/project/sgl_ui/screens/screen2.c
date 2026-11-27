#include "ui.h"


sgl_obj_t* ui_screen2_init(void) {
    // --- screen2 ---
    screen2 = sgl_rect_create(NULL);
    sgl_obj_set_size(screen2, 240, 320);
    sgl_obj_set_pos(screen2, 0, 0);
    sgl_rect_set_color(screen2, sgl_rgb(0x01, 0x01, 0x01));
    sgl_rect_set_alpha(screen2, 255);
    sgl_rect_set_radius(screen2, 0);
    sgl_rect_set_border_width(screen2, 0);
    sgl_rect_set_border_color(screen2, sgl_rgb(0x00, 0x00, 0x00));
    // Children of screen2
    // --- label2 ---
    label2 = sgl_label_create(screen2);
    sgl_obj_set_size(label2, 240, 20);
    sgl_obj_set_pos(label2, 0, 0);
    sgl_label_set_text(label2, " Model:FW-CZ01");
    sgl_label_set_font(label2, &consolas14);
    sgl_label_set_text_color(label2, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_bg_color(label2, sgl_rgb(0x35, 0x35, 0x35));
    sgl_label_set_text_align(label2, SGL_ALIGN_LEFT_MID);
    sgl_label_set_alpha(label2, 255);
    // --- rectangle9 ---
    rectangle9 = sgl_rect_create(screen2);
    sgl_obj_set_size(rectangle9, 193, 32);
    sgl_obj_set_pos(rectangle9, 24, 30);
    sgl_rect_set_color(rectangle9, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle9, 255);
    sgl_rect_set_radius(rectangle9, 8);
    sgl_rect_set_border_width(rectangle9, 2);
    sgl_rect_set_border_color(rectangle9, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle9, 255);
    // Children of rectangle9
    // --- label16 ---
    label16 = sgl_label_create(rectangle9);
    sgl_obj_set_size(label16, 100, 20);
    sgl_obj_set_pos(label16, 47, 8);
    sgl_label_set_text(label16, "version");
    sgl_label_set_font(label16, &consolas14);
    sgl_label_set_text_color(label16, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label16, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label16, 255);
    sgl_label_set_radius(label16, 4);
    // --- rectangle10 ---
    rectangle10 = sgl_rect_create(screen2);
    sgl_obj_set_size(rectangle10, 193, 32);
    sgl_obj_set_pos(rectangle10, 24, 84);
    sgl_rect_set_color(rectangle10, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle10, 255);
    sgl_rect_set_radius(rectangle10, 8);
    sgl_rect_set_border_width(rectangle10, 2);
    sgl_rect_set_border_color(rectangle10, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle10, 255);
    // Children of rectangle10
    // --- label17 ---
    label17 = sgl_label_create(rectangle10);
    sgl_obj_set_size(label17, 100, 20);
    sgl_obj_set_pos(label17, 47, 8);
    sgl_label_set_text(label17, "reset");
    sgl_label_set_font(label17, &consolas14);
    sgl_label_set_text_color(label17, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label17, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label17, 255);
    sgl_label_set_radius(label17, 4);
    // --- rectangle11 ---
    rectangle11 = sgl_rect_create(screen2);
    sgl_obj_set_size(rectangle11, 193, 32);
    sgl_obj_set_pos(rectangle11, 23, 139);
    sgl_rect_set_color(rectangle11, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle11, 255);
    sgl_rect_set_radius(rectangle11, 8);
    sgl_rect_set_border_width(rectangle11, 2);
    sgl_rect_set_border_color(rectangle11, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle11, 255);
    // Children of rectangle11
    // --- label18 ---
    label18 = sgl_label_create(rectangle11);
    sgl_obj_set_size(label18, 100, 20);
    sgl_obj_set_pos(label18, 47, 8);
    sgl_label_set_text(label18, "support");
    sgl_label_set_font(label18, &consolas14);
    sgl_label_set_text_color(label18, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label18, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label18, 255);
    sgl_label_set_radius(label18, 4);
    // --- img_ext1 ---
    img_ext1 = sgl_img_ext_create(screen2);
    sgl_obj_set_size(img_ext1, 240, 29);
    sgl_obj_set_pos(img_ext1, 0, 291);
    // --- rectangle12 ---
    rectangle12 = sgl_rect_create(screen2);
    sgl_obj_set_size(rectangle12, 60, 30);
    sgl_obj_set_pos(rectangle12, 95, 239);
    sgl_rect_set_color(rectangle12, sgl_rgb(0x31, 0x31, 0x31));
    sgl_rect_set_alpha(rectangle12, 255);
    sgl_rect_set_radius(rectangle12, 8);
    sgl_rect_set_border_width(rectangle12, 2);
    sgl_rect_set_border_color(rectangle12, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle12, 255);
    // Children of rectangle12
    // --- label19 ---
    label19 = sgl_label_create(rectangle12);
    sgl_obj_set_size(label19, 51, 20);
    sgl_obj_set_pos(label19, 3, 7);
    sgl_label_set_text(label19, "return");
    sgl_label_set_font(label19, &consolas14);
    sgl_label_set_text_color(label19, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label19, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label19, 255);
    sgl_label_set_radius(label19, 4);
    // --- rectangle13 ---
    rectangle13 = sgl_rect_create(screen2);
    sgl_obj_set_size(rectangle13, 193, 32);
    sgl_obj_set_pos(rectangle13, 23, 192);
    sgl_rect_set_color(rectangle13, sgl_rgb(0x30, 0x30, 0x30));
    sgl_rect_set_alpha(rectangle13, 255);
    sgl_rect_set_radius(rectangle13, 8);
    sgl_rect_set_border_width(rectangle13, 2);
    sgl_rect_set_border_color(rectangle13, sgl_rgb(0xF0, 0xF8, 0xFF));
    sgl_rect_set_border_alpha(rectangle13, 255);
    // Children of rectangle13
    // --- label20 ---
    label20 = sgl_label_create(rectangle13);
    sgl_obj_set_size(label20, 118, 20);
    sgl_obj_set_pos(label20, 34, 8);
    sgl_label_set_text(label20, "countdown mode");
    sgl_label_set_font(label20, &consolas14);
    sgl_label_set_text_color(label20, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label20, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label20, 255);
    sgl_label_set_radius(label20, 4);

  return screen2;
}

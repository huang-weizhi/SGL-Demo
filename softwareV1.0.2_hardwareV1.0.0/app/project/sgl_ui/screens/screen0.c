#include "ui.h"


sgl_obj_t* ui_screen0_init(void) {
    // --- screen0 ---
    screen0 = sgl_rect_create(NULL);
    sgl_obj_set_size(screen0, 240, 320);
    sgl_obj_set_pos(screen0, 0, 0);
    sgl_rect_set_color(screen0, sgl_rgb(0x01, 0x01, 0x01));
    sgl_rect_set_alpha(screen0, 255);
    sgl_rect_set_radius(screen0, 0);
    sgl_rect_set_border_width(screen0, 0);
    sgl_rect_set_border_color(screen0, sgl_rgb(0x00, 0x00, 0x00));
    // Children of screen0
    // --- label4 ---
    label4 = sgl_label_create(screen0);
    sgl_obj_set_size(label4, 240, 20);
    sgl_obj_set_pos(label4, 0, 0);
    sgl_label_set_text(label4, " Model:FW-CZ01");
    sgl_label_set_font(label4, &consolas14);
    sgl_label_set_text_color(label4, sgl_rgb(0xEB, 0xEB, 0xEB));
    sgl_label_set_bg_color(label4, sgl_rgb(0x33, 0x33, 0x33));
    sgl_label_set_text_align(label4, SGL_ALIGN_LEFT_MID);
    sgl_label_set_alpha(label4, 255);
    // --- label5 ---
    label5 = sgl_label_create(screen0);
    sgl_obj_set_size(label5, 150, 80);
    sgl_obj_set_pos(label5, 42, 120);
    sgl_label_set_text(label5, "320");
    sgl_label_set_font(label5, &consolas32);
    sgl_label_set_text_color(label5, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label5, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label5, 255);
    sgl_label_set_radius(label5, 4);
    // --- rectangle0 ---
    rectangle0 = sgl_rect_create(screen0);
    sgl_obj_set_size(rectangle0, 58, 32);
    sgl_obj_set_pos(rectangle0, 13, 276);
    sgl_rect_set_color(rectangle0, sgl_rgb(0x34, 0x34, 0x34));
    sgl_rect_set_alpha(rectangle0, 255);
    sgl_rect_set_radius(rectangle0, 6);
    sgl_rect_set_border_width(rectangle0, 2);
    sgl_rect_set_border_color(rectangle0, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_rect_set_border_alpha(rectangle0, 255);
    // Children of rectangle0
    // --- line0 ---
    line0 = sgl_line_create(rectangle0);
    sgl_obj_set_size(line0, 55, 10);
    sgl_obj_set_pos(line0, -1, 9);
    // --- 绝对坐标线条: line0 ---
    sgl_line_set_pos(line0, -1, 14, 54, 14);
    sgl_line_set_color(line0, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_line_set_alpha(line0, 255);
    sgl_line_set_width(line0, 2);
    // --- label3 ---
    label3 = sgl_label_create(rectangle0);
    sgl_obj_set_size(label3, 40, 20);
    sgl_obj_set_pos(label3, 8, -2);
    sgl_label_set_text(label3, "CH1");
    sgl_label_set_font(label3, &consolas14);
    sgl_label_set_text_color(label3, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label3, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label3, 255);
    sgl_label_set_radius(label3, 4);
    // --- label6 ---
    label6 = sgl_label_create(rectangle0);
    sgl_obj_set_size(label6, 40, 20);
    sgl_obj_set_pos(label6, 8, 14);
    sgl_label_set_text(label6, "320");
    sgl_label_set_font(label6, &consolas14);
    sgl_label_set_text_color(label6, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label6, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label6, 255);
    sgl_label_set_radius(label6, 4);
    // --- rectangle1 ---
    rectangle1 = sgl_rect_create(screen0);
    sgl_obj_set_size(rectangle1, 58, 32);
    sgl_obj_set_pos(rectangle1, 92, 276);
    sgl_rect_set_color(rectangle1, sgl_rgb(0x34, 0x34, 0x34));
    sgl_rect_set_alpha(rectangle1, 255);
    sgl_rect_set_radius(rectangle1, 6);
    sgl_rect_set_border_width(rectangle1, 2);
    sgl_rect_set_border_color(rectangle1, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_rect_set_border_alpha(rectangle1, 255);
    // Children of rectangle1
    // --- line1 ---
    line1 = sgl_line_create(rectangle1);
    sgl_obj_set_size(line1, 55, 10);
    sgl_obj_set_pos(line1, -1, 9);
    // --- 绝对坐标线条: line1 ---
    sgl_line_set_pos(line1, -1, 14, 54, 14);
    sgl_line_set_color(line1, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_line_set_alpha(line1, 255);
    sgl_line_set_width(line1, 2);
    // --- label7 ---
    label7 = sgl_label_create(rectangle1);
    sgl_obj_set_size(label7, 40, 20);
    sgl_obj_set_pos(label7, 8, -2);
    sgl_label_set_text(label7, "CH2");
    sgl_label_set_font(label7, &consolas14);
    sgl_label_set_text_color(label7, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label7, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label7, 255);
    sgl_label_set_radius(label7, 4);
    // --- label8 ---
    label8 = sgl_label_create(rectangle1);
    sgl_obj_set_size(label8, 40, 20);
    sgl_obj_set_pos(label8, 8, 14);
    sgl_label_set_text(label8, "180");
    sgl_label_set_font(label8, &consolas14);
    sgl_label_set_text_color(label8, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label8, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label8, 255);
    sgl_label_set_radius(label8, 4);
    // --- rectangle2 ---
    rectangle2 = sgl_rect_create(screen0);
    sgl_obj_set_size(rectangle2, 58, 32);
    sgl_obj_set_pos(rectangle2, 171, 276);
    sgl_rect_set_color(rectangle2, sgl_rgb(0x34, 0x34, 0x34));
    sgl_rect_set_alpha(rectangle2, 255);
    sgl_rect_set_radius(rectangle2, 6);
    sgl_rect_set_border_width(rectangle2, 2);
    sgl_rect_set_border_color(rectangle2, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_rect_set_border_alpha(rectangle2, 255);
    // Children of rectangle2
    // --- line2 ---
    line2 = sgl_line_create(rectangle2);
    sgl_obj_set_size(line2, 55, 10);
    sgl_obj_set_pos(line2, -1, 9);
    // --- 绝对坐标线条: line2 ---
    sgl_line_set_pos(line2, -1, 14, 54, 14);
    sgl_line_set_color(line2, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_line_set_alpha(line2, 255);
    sgl_line_set_width(line2, 2);
    // --- label9 ---
    label9 = sgl_label_create(rectangle2);
    sgl_obj_set_size(label9, 40, 20);
    sgl_obj_set_pos(label9, 8, -2);
    sgl_label_set_text(label9, "CH3");
    sgl_label_set_font(label9, &consolas14);
    sgl_label_set_text_color(label9, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label9, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label9, 255);
    sgl_label_set_radius(label9, 4);
    // --- label10 ---
    label10 = sgl_label_create(rectangle2);
    sgl_obj_set_size(label10, 40, 20);
    sgl_obj_set_pos(label10, 8, 14);
    sgl_label_set_text(label10, "260");
    sgl_label_set_font(label10, &consolas14);
    sgl_label_set_text_color(label10, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label10, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label10, 255);
    sgl_label_set_radius(label10, 4);
    // --- rectangle3 ---
    rectangle3 = sgl_rect_create(screen0);
    sgl_obj_set_size(rectangle3, 210, 55);
    sgl_obj_set_pos(rectangle3, 15, 23);
    sgl_rect_set_color(rectangle3, sgl_rgb(0x33, 0x33, 0x33));
    sgl_rect_set_alpha(rectangle3, 255);
    sgl_rect_set_radius(rectangle3, 8);
    sgl_rect_set_border_width(rectangle3, 2);
    sgl_rect_set_border_color(rectangle3, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_rect_set_border_alpha(rectangle3, 255);
    // Children of rectangle3
    // --- circle0 ---
    circle0 = sgl_circle_create(rectangle3);
    sgl_obj_set_size(circle0, 24, 24);
    sgl_obj_set_pos(circle0, 153, 14);
    sgl_circle_set_radius(circle0, 12);
    sgl_circle_set_color(circle0, sgl_rgb(0x43, 0xE7, 0x82));
    sgl_circle_set_alpha(circle0, 255);
    sgl_circle_set_border_width(circle0, 1);
    sgl_circle_set_border_color(circle0, sgl_rgb(0x34, 0x34, 0x34));
    // --- polygon0 ---
    polygon0 = sgl_polygon_create(rectangle3);
    sgl_obj_set_size(polygon0, 10, 10);
    sgl_obj_set_pos(polygon0, 161, 21);
    static sgl_pos_t polygon0_pts[] = {
        {0, 0},
        {10, 5},
        {0, 10},
    };
    sgl_polygon_set_vertices(polygon0, polygon0_pts, 3);
    sgl_polygon_set_fill_color(polygon0, sgl_rgb(0x33, 0x33, 0x33));
    sgl_polygon_set_border_color(polygon0, sgl_rgb(0xFF, 0x14, 0x93));
    sgl_polygon_set_border_width(polygon0, 0);
    sgl_polygon_set_alpha(polygon0, 255);
    // --- label0 ---
    label0 = sgl_label_create(rectangle3);
    sgl_obj_set_size(label0, 96, 35);
    sgl_obj_set_pos(label0, 58, 11);
    sgl_label_set_text(label0, "320");
    sgl_label_set_font(label0, &consolas32);
    sgl_label_set_text_color(label0, sgl_rgb(0xFF, 0xFF, 0xFF));
    sgl_label_set_text_align(label0, SGL_ALIGN_CENTER);
    sgl_label_set_alpha(label0, 255);
    sgl_label_set_radius(label0, 4);

  return screen0;
}

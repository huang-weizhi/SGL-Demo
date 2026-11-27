#ifndef __UI_FLASH_IMG_H__
#define __UI_FLASH_IMG_H__

#include "ui_handle.h"

/**
 * Flash picture table: converted from the legacy FW_CZ01
 * interface/Inf_lcd_picture.h lcd_picture_table[].
 * The 66 pictures live in external SPI flash (merged.bin @ address 0),
 * RGB565 big-endian, row-scanned, no header (width/height are here).
 */

#define FLASH_PIC_NUM   66  /* entries in flash_pictures[] below */

/* frequently used ids (index = legacy picture id) */
#define PIC_LOGO                0   /* 1-loge-190x33 */
#define PIC_TOP_BAR             56  /* 6.1 顶部状态栏 240x29 */
#define PIC_BOTTOM_BAR_CN       57  /* 6.2 底部装饰栏 240x29 */
#define PIC_MENU_FIRST          18  /* 4.1 菜单选中/未选中成对开始（18..35） */
#define PIC_EXIT_SEL_CN         36  /* 4.37 选中退出 84x30 */
#define PIC_EXIT_UNSEL_CN       37  /* 4.38 取消选中退出 84x30 */

extern const sgl_pixmap_t flash_pictures[];

/* bind a flash picture (by id) to an sgl_img_ext widget */
void ui_pic_bind(sgl_obj_t *img, uint16_t id);

#endif /* __UI_FLASH_IMG_H__ */

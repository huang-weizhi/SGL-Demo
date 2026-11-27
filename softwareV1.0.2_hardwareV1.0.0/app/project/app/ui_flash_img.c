/**
 * @file ui_flash_img.c
 *
 * SGL pixmap table for the 66 pictures stored in external SPI flash
 * (W25Qxxx, merged.bin @ address 0). Converted from the legacy FW_CZ01
 * interface/Inf_lcd_picture.h lcd_picture_table[].
 *
 * Legacy .ebm format: RGB565, BIG-endian, row-scanned, no header
 * (width/height live in this table). The read adapter below swaps
 * byte order because SGL pixels are little-endian in RAM.
 */
#include "ui_flash_img.h"
#include "Inf_w25qxxx.h"

/**
 * SGL read callback: void (size_t addr, uint8_t *out, uint32_t len).
 * spiflash_read signature is (buf, addr, len) -> argument order swapped,
 * plus big-endian -> little-endian byte swap for RGB565 pixels.
 */
static void flash_pic_read(size_t addr, uint8_t *out, uint32_t len)
{
    spiflash_read(out, (uint32_t)addr, len);
    for (uint32_t i = 0; i + 1 < len; i += 2) {
        uint8_t t = out[i];
        out[i] = out[i + 1];
        out[i + 1] = t;
    }
}

void ui_pic_bind(sgl_obj_t *img, uint16_t id)
{
    if (img == NULL || id >= FLASH_PIC_NUM) {
        return;
    }
    sgl_img_ext_set_pixmap(img, &flash_pictures[id]);
    sgl_img_ext_set_read_ops(img, flash_pic_read);
}

/* width, height, flash address, format - one entry per legacy picture id.
 * designated initializers required: width/height/format are bit-fields. */
const sgl_pixmap_t flash_pictures[] = {
    /*  0 */ { .width = 190, .height =  33, .bitmap = { .addr = 0x00000000 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 1-loge-190x33 */
    /*  1 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x000030FC }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.1-曲线-加热停止-选中设定时间 */
    /*  2 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x00005ABA }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.2-曲线-加热开始-选中设定时间 */
    /*  3 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x00008478 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.3-曲线-加热停止-取消选中设定时间 */
    /*  4 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x0000AE36 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.4-曲线-加热开始-取消选中设定时间 */
    /*  5 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x0000D7F4 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.5-曲线-选中设定温度 */
    /*  6 */ { .width = 137, .height =  39, .bitmap = { .addr = 0x000101B2 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.6-曲线-取消选中设定温度 */
    /*  7 */ { .width =  19, .height =  18, .bitmap = { .addr = 0x00012B70 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.7-曲线-摄氏度图标 */
    /*  8 */ { .width =  19, .height =  18, .bitmap = { .addr = 0x00012E1C }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.8-曲线-华氏度图标 */
    /*  9 */ { .width =  23, .height = 128, .bitmap = { .addr = 0x000130C8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 2.9-曲线-曲线刻度 */
    /* 10 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x000147C8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.1-数值-加热停止-选中设定时间 */
    /* 11 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x0001AC18 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.2-数值-加热开始-选中设定时间 */
    /* 12 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x00021068 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.3-数值-加热停止-取消选中设定时间 */
    /* 13 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x000274B8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.4-数值-加热开始-取消选中设定时间 */
    /* 14 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x0002D908 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.5-数值-选中设定温度 */
    /* 15 */ { .width = 214, .height =  60, .bitmap = { .addr = 0x00033D58 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.6-数值-取消选中设定温度 */
    /* 16 */ { .width =  25, .height =  24, .bitmap = { .addr = 0x0003A1A8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.7-数值-摄氏度图标 */
    /* 17 */ { .width =  25, .height =  24, .bitmap = { .addr = 0x0003A658 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 3.8-数值-华氏度图标 */
    /* 18 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0003AB08 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.1-菜单-选中界面选择 */
    /* 19 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0003DB48 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.2-菜单-取消选中界面选择 */
    /* 20 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00040B88 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.5-菜单-选中温度单位 */
    /* 21 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00043BC8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.6-菜单-取消选中温度单位 */
    /* 22 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00046C08 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.9-菜单-选中温度补偿 */
    /* 23 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00049C48 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.10-菜单-取消选中温度补偿 */
    /* 24 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0004CC88 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.13-菜单-选中语言设置 */
    /* 25 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0004FCC8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.14-菜单-取消选中语言设置 */
    /* 26 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00052D08 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.17-菜单-选中声音设置 */
    /* 27 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00055D48 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.18-菜单-取消选中声音设置 */
    /* 28 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00058D88 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.21-菜单-选中倒计时选项 */
    /* 29 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0005BDC8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.22-菜单-取消选中倒计时选项 */
    /* 30 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0005EE08 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.25-菜单-选中版本号 */
    /* 31 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00061E48 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.26-菜单-取消选中版本号 */
    /* 32 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00064E88 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.29-菜单-选中恢复出厂设置 */
    /* 33 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x00067EC8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.30-菜单-取消选中恢复出厂设置 */
    /* 34 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0006AF08 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.33-菜单-选中技术支持 */
    /* 35 */ { .width = 193, .height =  32, .bitmap = { .addr = 0x0006DF48 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.34-取消选中技术支持 */
    /* 36 */ { .width =  84, .height =  30, .bitmap = { .addr = 0x00070F88 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.37-菜单-选中退出 */
    /* 37 */ { .width =  84, .height =  30, .bitmap = { .addr = 0x00072338 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 4.38-菜单-取消选中退出 */
    /* 38 */ { .width = 193, .height = 131, .bitmap = { .addr = 0x000736E8 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.1-设置界面选择 */
    /* 39 */ { .width = 193, .height = 131, .bitmap = { .addr = 0x0007FC6E }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.3-设置温度单位 */
    /* 40 */ { .width = 193, .height = 131, .bitmap = { .addr = 0x0008C1F4 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.5-温度补偿界面 */
    /* 41 */ { .width = 204, .height =  91, .bitmap = { .addr = 0x0009877A }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.7-设置中文界面 */
    /* 42 */ { .width = 204, .height =  91, .bitmap = { .addr = 0x000A1882 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.8-设置英文界面 */
    /* 43 */ { .width = 193, .height = 131, .bitmap = { .addr = 0x000AA98A }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.9-设置声音界面 */
    /* 44 */ { .width = 193, .height = 146, .bitmap = { .addr = 0x000B6F10 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.11-倒计时选项界面 */
    /* 45 */ { .width = 193, .height = 131, .bitmap = { .addr = 0x000C4B34 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.13-版本号界面 */
    /* 46 */ { .width = 193, .height = 170, .bitmap = { .addr = 0x000D10BA }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.15-取消选中恢复出厂 */
    /* 47 */ { .width = 193, .height = 170, .bitmap = { .addr = 0x000E110E }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.17-选中恢复出厂 */
    /* 48 */ { .width = 193, .height = 170, .bitmap = { .addr = 0x000F1162 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.19-已恢复出厂设置 */
    /* 49 */ { .width = 211, .height = 204, .bitmap = { .addr = 0x001011B6 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.21-技术支持界面 */
    /* 50 */ { .width =  84, .height =  30, .bitmap = { .addr = 0x001161FE }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.23-选中恢复出厂返回按钮 */
    /* 51 */ { .width =  84, .height =  30, .bitmap = { .addr = 0x001175AE }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.24-取消选中恢复出厂返回按钮 */
    /* 52 */ { .width =  17, .height =  17, .bitmap = { .addr = 0x0011895E }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.27-选中选项图标 */
    /* 53 */ { .width =  17, .height =  17, .bitmap = { .addr = 0x00118BA0 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.28-取消选中选项图标 */
    /* 54 */ { .width =  19, .height =  18, .bitmap = { .addr = 0x00118DE2 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.29-温度补偿-摄氏度图标 */
    /* 55 */ { .width =  19, .height =  18, .bitmap = { .addr = 0x0011908E }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 5.30-温度补偿-华氏度图标 */
    /* 56 */ { .width = 240, .height =  29, .bitmap = { .addr = 0x0011933A }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.1-顶部状态栏 */
    /* 57 */ { .width = 240, .height =  29, .bitmap = { .addr = 0x0011C99A }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.2-底部装饰栏-中文 */
    /* 58 */ { .width =  13, .height =  11, .bitmap = { .addr = 0x0011FFFA }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.4-音量图标 */
    /* 59 */ { .width =  13, .height =  11, .bitmap = { .addr = 0x00120118 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.5-静量图标 */
    /* 60 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x00120236 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.6-选中通道1 */
    /* 61 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x001215B6 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.7-取消选中通道1 */
    /* 62 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x00122936 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.8-选中通道2 */
    /* 63 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x00123CB6 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.9-取消选中通道2 */
    /* 64 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x00125036 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.10-选中通道3 */
    /* 65 */ { .width =  64, .height =  39, .bitmap = { .addr = 0x001263B6 }, .format = SGL_PIXMAP_FMT_RGB565 }, /* 6.11-取消选中通道3 */
};

#ifndef __LCD_H__
#define __LCD_H__

#include "stdint.h"
#include "at32f415.h"
#include "at32_spiflash.h"
/*=============================================================================
 *                             Hardware Configuration
 *============================================================================*/

/* LCD screen size */
#define LCD_W               320
#define LCD_H               240

/* SPI peripheral selection */
#define LCD_SPI             SPI2

/* SPI DMA channel configuration (refer to MCU datasheet) */
#define SPI_DMA_TX_CHANNEL  DMA1_CHANNEL4   // MOSI transmit channel
#define SPI_DMA_RX_CHANNEL  DMA1_CHANNEL3   // MISO receive channel
#define SPI_DMA_TX_FLAG     DMA1_FDT4_FLAG  // Transmit complete flag
#define SPI_DMA_RX_FLAG     DMA1_FDT3_FLAG  // Receive complete flag

/* DMA priority */
#define LCD_SPI_DMA_PRIORITY    DMA_PRIORITY_VERY_HIGH

/* SPI communication timeout (milliseconds) */
#define SPI_TIMEOUT_MS      200

/* External flash read data */
#define BLOCK_SIZE          2048
// 图片信息结构体
typedef struct {
    uint32_t addr;   // 图片在外部Flash中的起始地址
    uint16_t width;  // 图片宽度（像素）
    uint16_t height; // 图片高度（像素）
} PicInfo;

typedef enum {
    PIC_LOGO,                               // 1-logo.jpg
	
    PIC_MAIN_INTERFACE,                     // 2-主界面.jpg
	
    PIC_CURVE_SCREEN,                       // 3-曲线界面.jpg
	
    PIC_MENU_SELECTED_WORK_MODE_CN,         // 4-菜单-选中工作模式-中文.jpg
    PIC_MENU_UNSELECTED_WORK_MODE_CN,       // 5-菜单-取消选中工作模式-中文.jpg
    PIC_MENU_SELECTED_WORK_MODE_EN,         // 6-菜单-选中工作模式-英文.jpg
    PIC_MENU_UNSELECTED_WORK_MODE_EN,       // 7-菜单-取消选中工作模式-英文.jpg
	
    PIC_MENU_SELECTED_TEMP_UNIT_CN,         // 8-菜单-选中温度单位-中文.jpg
    PIC_MENU_UNSELECTED_TEMP_UNIT_CN,       // 9-菜单-取消选中温度单位-中文.jpg
    PIC_MENU_SELECTED_TEMP_UNIT_EN,         // 10-菜单-选中温度单位-英.jpg
    PIC_MENU_UNSELECTED_TEMP_UNIT_EN,       // 11-菜单-取消选中温度单位-英文.jpg
	
    PIC_MENU_SELECTED_SOUND_SETTING_CN,     // 12-菜单-选中声音设置-中文.jpg
    PIC_MENU_UNSELECTED_SOUND_SETTING_CN,   // 13-菜单-取消选中声音设置-中文.jpg
    PIC_MENU_SELECTED_SOUND_SETTING_EN,     // 14-菜单-选中声音设置-英文.jpg
    PIC_MENU_UNSELECTED_SOUND_SETTING_EN,   // 15-菜单-取消选中声音设置-英文.jpg

    PIC_MENU_SELECTED_LANGUAGE_SETTING_CN,  // 16-菜单-选中语言设置-中文.jpg
    PIC_MENU_UNSELECTED_LANGUAGE_SETTING_CN,// 17-菜单-取消选中语言设置-中文.jpg
    PIC_MENU_SELECTED_LANGUAGE_SETTING_EN,  // 18-菜单-选中语言设置-英文.jpg
    PIC_MENU_UNSELECTED_LANGUAGE_SETTING_EN,// 19-菜单-取消选中语言设置-英文.jpg
	
    PIC_MENU_SELECTED_TEMP_CALIB_CN,        // 20-菜单-选中温度校准-中文.jpg
    PIC_MENU_UNSELECTED_TEMP_CALIB_CN,      // 21-菜单-取消选中温度校准-中文.jpg
    PIC_MENU_SELECTED_TEMP_CALIB_EN,        // 22-菜单-选中温度校准-英文.jpg
    PIC_MENU_UNSELECTED_TEMP_CALIB_EN,      // 23-菜单-取消选中温度校准-英文.jpg

    PIC_MENU_SELECTED_SLEEP_SETTING_CN,     // 24-菜单-选中休眠设置-中文.jpg
    PIC_MENU_UNSELECTED_SLEEP_SETTING_CN,   // 25-菜单-取消选中休眠设置-中文.jpg
    PIC_MENU_SELECTED_SLEEP_SETTING_EN,     // 26-菜单-选中休眠设置-英文.jpg
    PIC_MENU_UNSELECTED_SLEEP_SETTING_EN,   // 27-菜单-取消选中休眠设置-英文.jpg
	
    PIC_MENU_SELECTED_TEMP_DISPLAY_CN,    // 28-菜单-选中温度显示-中文.jpg
    PIC_MENU_UNSELECTED_TEMP_DISPLAY_CN,      // 29-菜单-取消选中温度显示-中文.jpg
    PIC_MENU_SELECTED_TEMP_DISPLAY_EN,      // 30-菜单-选中温度显示-英文.jpg
    PIC_MENU_UNSELECTED_TEMP_DISPLAY_EN,    // 31-菜单-取消选中温度显示-英文.jpg
	
    PIC_MENU_SELECTED_VERSION_CN,           // 32-菜单-选中版本号-中文.jpg
    PIC_MENU_UNSELECTED_VERSION_CN,         // 33-菜单-取消选中版本号-中文.jpg
    PIC_MENU_SELECTED_VERSION_EN,           // 34-菜单-选中版本号-英文.jpg
    PIC_MENU_UNSELECTED_VERSION_EN,         // 35-菜单-取消选中版本号-英文.jpg
	
    PIC_MENU_SELECTED_FACTORY_RESET_CN,     // 36-菜单-选中恢复出厂设置-中文.jpg
    PIC_MENU_UNSELECTED_FACTORY_RESET_CN,   // 37-菜单-取消选中恢复出厂设置-中文.jpg
    PIC_MENU_SELECTED_FACTORY_RESET_EN,     // 38-菜单-选中恢复出厂设置-英文.jpg
    PIC_MENU_UNSELECTED_FACTORY_RESET_EN,   // 39-菜单-取消选中恢复出厂设置-英文.jpg
	
    PIC_MENU_SELECTED_TECH_SUPPORT_CN,      // 40-菜单-选中技术支持-中文.jpg
    PIC_MENU_UNSELECTED_TECH_SUPPORT_CN,    // 41-菜单-取消选中技术支持-中文.jpg
    PIC_MENU_SELECTED_TECH_SUPPORT_EN,      // 42-菜单-选中技术支持-英文.jpg
    PIC_MENU_UNSELECTED_TECH_SUPPORT_EN,    // 43-菜单-取消选中技术支持-英文.jpg
	
    PIC_SETTING_WORK_MODE_CN,               // 44-设置-工作模式-中文.jpg
    PIC_SETTING_WORK_MODE_EN,               // 45-设置-工作模式-英文.jpg
	
    PIC_SETTING_TEMP_UNIT_CN,               // 46-设置-温度单位-中文.jpg
    PIC_SETTING_TEMP_UNIT_EN,               // 47-设置-温度单位-英文.jpg
	
    PIC_SETTING_SOUND_SETTING_CN,           // 48-设置-声音设置-中文.jpg
    PIC_SETTING_SOUND_SETTING_EN,           // 49-设置-声音设置-英文.jpg

    PIC_SETTING_CN,                         // 50-设置-设置中文.jpg
    PIC_SETTING_EN,                         // 51-设置-设置英文.jpg
	
    PIC_SETTING_TEMP_CALIB_CN,              // 52-设置-温度校准-中文.jpg
    PIC_SETTING_TEMP_CALIB_EN,              // 53-设置-温度校准-英文.jpg

    PIC_SETTING_SLEEP_MODE_CN,              // 54-设置-休眠模式-中文.jpg
    PIC_SETTING_SLEEP_MODE_EN,              // 55-设置-休眠模式-英文.jpg
	
    PIC_SETTING_TEMP_DISPLAY_CN,            // 56-设置-温度显示-中文.jpg
    PIC_SETTING_TEMP_DISPLAY_EN,            // 57-设置-温度显示-英文.jpg
	
    PIC_SETTING_VERSION_CN,                 // 58-设置-版本号-中文.jpg
    PIC_SETTING_VERSION_EN,                 // 59-设置-版本号-英文.jpg
	
    PIC_SETTING_UNSELECT_FACTORY_RESET_CN,  // 60-设置-取消选中恢复出厂设置-中文.jpg
    PIC_SETTING_UNSELECT_FACTORY_RESET_EN,  // 61-设置-取消选中恢复出厂设置-英文.jpg
    PIC_SETTING_SELECT_FACTORY_RESET_CN,    // 62-设置-选中恢复出厂设置-中文.jpg
    PIC_SETTING_SELECT_FACTORY_RESET_EN,    // 63-设置-选中恢复出厂设置-英文.jpg
    PIC_SETTING_FACTORY_RESET_DONE_CN,      // 64-设置-恢复出厂设置完成-中文.jpg
    PIC_SETTING_FACTORY_RESET_DONE_EN,      // 65-设置-恢复出厂设置完成-英文.jpg
	
    PIC_SETTING_TECH_HA01_SUPPORT_CN,       // 66-设置-BF03技术支持-中文.jpg
    PIC_SETTING_TECH_HA01_SUPPORT_EN,       // 67-设置-BF03技术支持-英文.jpg
    PIC_SETTING_TECH_HA02_SUPPORT_CN,       // 68-设置-BF03技术支持-中文.jpg
    PIC_SETTING_TECH_HA02_SUPPORT_EN,       // 69-设置-BF03技术支持-英文.jpg
    PIC_SETTING_TECH_HA03_SUPPORT_CN,       // 70-设置-BF03技术支持-中文.jpg
    PIC_SETTING_TECH_HA03_SUPPORT_EN,       // 71-设置-BF03技术支持-英文.jpg
	
    PIC_SELECT_RETURN_ICON_CN,              // 72-选中返回按键图标-中文.jpg
    PIC_SELECT_RETURN_ICON_EN,              // 73-选中返回按键图标-英文.jpg
    PIC_UNSELECT_RETURN_ICON_CN,            // 74-取消选中返回按键图标-中文.jpg
    PIC_UNSELECT_RETURN_ICON_EN,            // 75-取消选中返回按键图标-英文.jpg
	
    PIC_SELECT_EXIT_ICON_CN,                // 76-选中退出按键图标-中文.jpg
    PIC_SELECT_EXIT_ICON_EN,                // 77-选中退出按键图标-英文.jpg  
    PIC_UNSELECT_EXIT_ICON_CN,              // 78-取消选中退出按键图标-中文.jpg
    PIC_UNSELECT_EXIT_ICON_EN,              // 79-取消选中退出按键图标-英文.jpg
	
    PIC_HA01_NAVIGATION_BAR_ICON,           // 80-HA01导航栏图标.jpg
    PIC_HA02_NAVIGATION_BAR_ICON,           // 81-HA02导航栏图标.jpg
    PIC_HA03_NAVIGATION_BAR_ICON,           // 82-HA03导航栏图标.jpg
	
    PIC_STANDBY_ICON_CN,                    // 83-待机中图标-中文.jpg
    PIC_STANDBY_ICON_EN,                    // 84-待机中图标-英文.jpg
    PIC_CANCEL_STANDBY_ICON,                // 85-取消待机图标.jpg
	
    PIC_LOW_TEMP_ALARM_CN,                  // 86-低温报警图标-中文.jpg
    PIC_LOW_TEMP_ALARM_EN,                  // 87-低温报警图标-英文.jpg
	
    PIC_HIGH_TEMP_ALARM_CN,                 // 88-高温报警图标-中文.jpg
    PIC_HIGH_TEMP_ALARM_EN,                 // 89-高温报警图标-英文.jpg
	
    PIC_FAN_ABNORMAL_CN,                    // 90-风扇异常图标-中文.jpg
    PIC_FAN_ABNORMAL_EN,                    // 91-风扇异常图标-英文.jpg
	

    PIC_SELECT_CHANNEL1_ICON,               // 95-选中通道1图标.jpg
    PIC_SELECT_CHANNEL2_ICON,               // 96-选中通道2图标.jpg
    PIC_SELECT_CHANNEL3_ICON,               // 97-选中通道3图标.jpg
	
    PIC_UNSELECT_CHANNEL1_ICON,             // 92-取消选中通道1图标.jpg
    PIC_UNSELECT_CHANNEL2_ICON,             // 93-取消选中通道2图标.jpg
    PIC_UNSELECT_CHANNEL3_ICON,             // 94-取消选中通道3图标.jpg
	
    PIC_FAN1_ICON,                          // 98-风扇1图标.jpg
    PIC_FAN2_ICON,                          // 99-风扇2图标.jpg
    PIC_FAN3_ICON,                          // 100-风扇3图标.jpg
    PIC_FAN4_ICON,                          // 101-风扇4图标.jpg
	
    PIC_NORMAL_MODE_ICON,                   // 102-正常模式图标.jpg
    PIC_COOL_MODE_ICON_CN,                  // 103-冷风模式图标-中文.jpg
    PIC_COOL_MODE_ICON_EN,                  // 104-冷风模式图标-英文.jpg
    PIC_QUICK_RELEASE_MODE_CN,              // 105-快拆模式-中文.jpg
    PIC_QUICK_RELEASE_MODE_EN,              // 106-快拆模式-英文.jpg
	
    PIC_SET_TEMP_ICON_CN,                   // 107-设定温度图标-中文.jpg
    PIC_SET_TEMP_ICON_EN,                   // 108-设定温度图标-英文.jpg
    PIC_ACTUAL_TEMP_ICON_CN,                // 109-实际温度图标-中文.jpg
    PIC_ACTUAL_TEMP_ICON_EN,                // 110-实际温度图标-英文.jpg
	
    PIC_SET_FAN_SPEED_ICON_CN,              // 111-设定风量图标-中文.jpg
    PIC_SET_FAN_SPEED_ICON_EN,              // 112-设定风量图标-英文.jpg
    PIC_ACTUAL_FAN_SPEED_ICON_CN,           // 113-实际风量图标-中文.jpg
    PIC_ACTUAL_FAN_SPEED_ICON_EN,           // 114-实际风量图标-英文.jpg
	
    PIC_BOOST_MODE_ICON,                    // 115-加强模式图标.jpg
	
    PIC_SETTING_PAGE_CELSIUS_ICON,          // 116-设置页面摄氏度图标.jpg
    PIC_SETTING_PAGE_FAHRENHEIT_ICON,       // 117-设置页面华氏度图标.jpg
	
    PIC_MAIN_INTERFACE_CELSIUS_ICON,        // 118-主界面摄氏度图标.jpg
    PIC_MAIN_INTERFACE_FAHRENHEIT_ICON,     // 119-主界面华氏度图标.jpg
	
    PIC_CURVE_SCREEN_CELSIUS_ICON,          // 120-曲线界面摄氏度图标.jpg
    PIC_CURVE_SCREEN_FAHRENHEIT_ICON,       // 121-曲线界面华氏度图标.jpg
	
    PIC_SOUND_ICON,                         // 122-声音图标.jpg
    PIC_MUTE_ICON,                          // 123-静音图标.jpg
	
    PIC_SELECT_SETTING_ICON,                // 124-选择设置图标.jpg
    PIC_DESELECT_SETTING_ICON,              // 125-不选择设置图标.jpg
	
    PIC_TEMP_SLIDER_ICON,                   // 126-温度滑块图标.jpg
    PIC_FAN_SLIDER_ICON,                    // 127-风量滑块图标.jpg
	
    PIC_POWER_OUTPUT_SLIDER_ICON,           // 128-功率输出滑块图标.jpg
	
    PIC_HA01_MODEL_ICON,                    // 129-HA01型号图标.jpg
    PIC_HA02_MODEL_ICON,                    // 130-HA02型号图标.jpg
    PIC_HA03_MODEL_ICON,                    // 131-HA03型号图标.jpg
	
    PIC_POWER_OUTPUT_ICON_CN,               // 132-功率输出图标-中文.jpg
    PIC_POWER_OUTPUT_ICON_EN,               // 133-功率输出图标-英文.jpg
	
    PIC_BOTTOM_INDICATOR_BAR,               // 134-下方指示栏.jpg.
	PIC_CURVE_TEMP_ICON,                    // 135-曲线温度单位.jpg
	PIC_CURVE_WIND_ICON,                    // 136-曲线风量单位.jpg
 
    PIC_NUM                                 // 图片总数 = 136
} PicID;

/*=============================================================================
 *                              GPIO Control Macros
 *============================================================================*/
#define LCD_RES_Clr()       GPIOA->clr = 0x0800
#define LCD_RES_Set()       GPIOA->scr = 0x0800
#define LCD_DC_Clr()        GPIOA->clr = 0x0100
#define LCD_DC_Set()        GPIOA->scr = 0x0100
#define LCD_CS_Clr()        GPIOB->clr = 0x1000
#define LCD_CS_Set()        GPIOB->scr = 0x1000

/*=============================================================================
 *                         Screen Orientation Configuration
 *============================================================================*/
/* Macro to set orientation register */
#define LCD_SET_DIRECTION(dir_value)  do{ LCD_WR_REG(0x36); LCD_WR_DATA8(dir_value); }while(0)

/* Select orientation: 0/1 portrait, 2/3 landscape */
#define USE_HORIZONTAL      2

/* Orientation register values */
#define LCD_DIR_0           0x00   // Portrait mode 0
#define LCD_DIR_1           0xC0   // Landscape mode 1
#define LCD_DIR_2           0x70   // Landscape mode 2
#define LCD_DIR_3           0xA0   // Portrait mode 3

/* Auto-select direction value based on USE_HORIZONTAL */
#if (USE_HORIZONTAL == 0)
    #define LCD_CURRENT_DIR   LCD_DIR_0
#elif (USE_HORIZONTAL == 1)
    #define LCD_CURRENT_DIR   LCD_DIR_1
#elif (USE_HORIZONTAL == 2)
    #define LCD_CURRENT_DIR   LCD_DIR_2
#else
    #define LCD_CURRENT_DIR   LCD_DIR_3
#endif

/*=============================================================================
 *                                  Color Definitions
 *============================================================================*/
#define WHITE           0xFFFF
#define BLACK           0x0000
#define BLUE            0x001F
#define BRED            0xF81F
#define GRED            0xFFE0
#define GBLUE           0x07FF
#define RED             0xF800
#define MAGENTA         0xF81F
#define GREEN           0x07E0
#define CYAN            0x7FFF
#define YELLOW          0xFFE0
#define CRIMSON         0xD8A7
#define TROPICAL_ORANGE 0xFC06
#define Scarlet         0xF920
#define BROWN           0xBC40
#define BRRED           0xFC07
#define GRAY            0x8430
#define DARKBLUE        0x01CF
#define LIGHTBLUE       0x7D7C
#define GRAYBLUE        0x5458
#define LIGHTGREEN      0x841F
#define LGRAY           0xC618
#define LGRAYBLUE       0xA651
#define LBBLUE          0x2B12

/*=============================================================================
 *                         Low-Level Communication Functions
 *============================================================================*/
void LCD_Init(void);
void LCD_Write_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

/*=============================================================================
 *                            SPI DMA Transfer Functions
 *============================================================================*/
void spi_dma_write_bytes(uint8_t *pbuffer, uint32_t length);
void spi_dma_write_halfwords(uint16_t *pbuffer, uint32_t length);
uint8_t spi_write_read_byte(uint8_t data);

/*=============================================================================
 *                           High-Level Drawing Functions
 *============================================================================*/
void LCD_Clear(uint16_t color);
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color);
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color);

/* Chinese character display (various sizes) */
void LCD_ShowChinese(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void LCD_ShowChinese12x12(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void LCD_ShowChinese16x16(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void LCD_ShowChinese24x24(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void LCD_ShowChinese32x32(uint16_t x, uint16_t y, uint8_t *s, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);

/* ASCII character and string display */
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t num, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);
void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p, uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode);

/* Number display utilities */
uint32_t mypow(uint8_t m, uint8_t n);
void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len, uint16_t fc, uint16_t bc, uint8_t sizey);
 
/* Image display */
void LCD_ShowPicture(uint16_t x, uint16_t y, uint16_t length, uint16_t width, const uint8_t pic[]);
void LCD_ShowPictureFromFlash(uint16_t x1, uint16_t y1, PicID picId);

void DrawProgressBar(u16 x1, u16 y1, u16 x2, u16 y2,
                     u16 pic_width, u16 pic_height,
                     u8 scale, u16 bar_color, u16 bc_color,
                     u8 pic, u8 direction);

void LCD_Show_Grid(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, uint8_t unit);
void LCD_Show_Curve(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,
                    float curve_1, float curve_2,
					uint16_t max_curve_1,uint16_t min_curve_1,
					uint16_t max_curve_2,uint16_t min_curve_2,
					uint8_t *curve_1_arr, uint8_t *curve_2_arr);
#endif /* __LCD_H__ */
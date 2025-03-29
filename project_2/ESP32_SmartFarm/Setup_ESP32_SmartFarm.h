// TFT_eSPI_Setup.h
#define USER_SETUP_LOADED 1

// LCD 
#define ILI9341_DRIVER

// 
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4  // RSTGPIO4 
#define TFT_BL   17  // (GPIO0 -> GPIO17)

// 
#define TOUCH_CS 27
#define TOUCH_IRQ 32

// LCD 
#define LOAD_GLCD   // Font 1
#define LOAD_FONT2  // Font 2
#define LOAD_FONT4  // Font 4
#define LOAD_GFXFF  // FreeFonts

#define SMOOTH_FONT

// SPI uc8fcud30cuc218 uc124uc815
#define SPI_FREQUENCY  20000000  // 27MHzuc5d0uc11c 20MHzub85c uc8fcud30cuc218 ud1b5uc77c (LCD uae68uc9d0 ud604uc0c1 ubc29uc9c0)
#define SPI_TOUCH_FREQUENCY  2500000

// 
#define TFT_RGB_ORDER TFT_BGR  // : TFT_RGB TFT_BGR
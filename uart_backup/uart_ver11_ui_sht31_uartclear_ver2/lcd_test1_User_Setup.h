// ESP32 SmartFarm 프로젝트용 User_Setup.h 파일
// TFT_eSPI 라이브러리 설정

#define USER_SETUP_INFO "ESP32 SmartFarm Project"

// LCD 드라이버 선택
#define ILI9341_DRIVER

// TFT 디스플레이 핀 설정
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4
#define TFT_MOSI 23  // GPIO13에서 GPIO23으로 변경 (비행거리 센서 SHUT 핀과 충돌 회피)
#define TFT_SCLK 14
#define TFT_MISO 12
#define TFT_BL   5   // 백라이트 핀 (PWM 지원)

// 터치 스크린 핀 설정
#define TOUCH_CS   27
#define TOUCH_IRQ  32
#define TOUCH_DIN  25
#define TOUCH_DO   26
#define TOUCH_CLK  33
#define TOUCH_DRIVER XPT2046_DRIVER

// SPI 통신 설정
#define SPI_FREQUENCY        40000000  // 최대 80MHz, 안정적인 값으로 40MHz 설정
#define SPI_READ_FREQUENCY   20000000
#define SPI_TOUCH_FREQUENCY  2500000

// 폰트 설정
#define LOAD_GLCD   // 기본 폰트
#define LOAD_FONT2  // 중간 크기 폰트
#define LOAD_FONT4  // 큰 숫자 표시용
#define SMOOTH_FONT // 부드러운 폰트 활성화

// 색상 설정
#define TFT_BLACK       0x0000
#define TFT_NAVY        0x000F
#define TFT_DARKGREEN   0x03E0
#define TFT_DARKCYAN    0x03EF
#define TFT_MAROON      0x7800
#define TFT_PURPLE      0x780F
#define TFT_OLIVE       0x7BE0
#define TFT_LIGHTGREY   0xC618
#define TFT_DARKGREY    0x7BEF
#define TFT_BLUE        0x001F
#define TFT_GREEN       0x07E0
#define TFT_CYAN        0x07FF
#define TFT_RED         0xF800
#define TFT_MAGENTA     0xF81F
#define TFT_YELLOW      0xFFE0
#define TFT_WHITE       0xFFFF
#define TFT_ORANGE      0xFD20
#define TFT_GREENYELLOW 0xAFE5
#define TFT_PINK        0xF81F

// DMA 설정 (ESP32에서 DMA는 SPI 속도를 크게 향상)
#define DMA_CHANNEL 1 // 또는 2, DMA 채널 선택
#define ESP32_DMA
#define SUPPORT_TRANSACTIONS

# ESP32 LCD 테스트 프로젝트

## 프로젝트 개요
ESP32 SmartFarm 프로젝트를 위한 LCD 및 터치 기능 테스트 코드입니다.

## 개발 환경
- 보드: ESP32-WROOM-32
- LCD: ILI9341 드라이버
- 터치 컨트롤러: XPT2046 (테스트 중)

## 핀 설정
- TFT_MISO: GPIO12
- TFT_MOSI: GPIO13
- TFT_SCLK: GPIO14
- TFT_CS: GPIO15
- TFT_DC: GPIO2
- TFT_RST: GPIO4
- TFT_BL: GPIO17 (백라이트)
- TOUCH_CS: GPIO27
- TOUCH_IRQ: GPIO32 (터치 테스트 중)

## 개발 일지

### 2025-03-28 V1.0.0 - LCD 기본 기능 테스트 완료

#### 성공한 부분
- ILI9341 드라이버로 LCD 기본 기능 구현 완료
- 백라이트 핀 GPIO17로 설정 및 정상 작동
- 색상, 도형, 텍스트 표시 모두 정상 작동

#### 발생한 문제와 해결 방법
- User_Setup.h 파일에 폰트 설정(LOAD_GLCD, LOAD_FONT2) 추가로 텍스트 표시 문제 해결
- 터치 컨트롤러 설정 추가 시 LCD 표시 문제 발생
- SPI 버스 공유 시 LCD/터치 드라이버 간 간섭 가능성 확인
- 현재는 LCD 기능만 활성화하여 안정적 작동 상태 유지

### 2025-03-28 V1.1.0 - 터치스크린 보정 및 기능 구현

#### 성공한 부분
- XPT2046 터치 컨트롤러 드라이버 활성화 및 작동 확인
- 터치 좌표 보정(Calibration) 작업 완료
- 터치 기반 버튼 인터랙션 구현

#### 터치스크린 보정값
- X 보정 계수(xFactor): -100.00
- Y 보정 계수(yFactor): -20.00
- X 오프셋(xOffset): 1620
- Y 오프셋(yOffset): 620

#### 발생한 문제와 해결 방법
- 터치 좌표가 실제 터치 위치와 다른 문제 발생
- 5점 보정 방식으로 터치 좌표 변환 계수 계산
- 음수 계수값은 터치 좌표계와 화면 좌표계의 방향이 반대임을 의미
- 계산된 보정값을 적용하여 정확한 터치 위치 구현

#### 사용 방법
```cpp
// 보정된 터치 좌표 계산 방법
int16_t x = (touchX * TOUCH_X_FACTOR) + TOUCH_X_OFFSET;
int16_t y = (touchY * TOUCH_Y_FACTOR) + TOUCH_Y_OFFSET;
```

#### User_Setup.h 설정
```cpp
// LCD 드라이버 선택
#define ILI9341_DRIVER

// 핀 설정
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  4

// 백라이트 핀
#define TFT_BL   17

// 터치 스크린 핀 설정
#define TOUCH_CS 27
#define TOUCH_DRIVER XPT2046_DRIVER  
#define TOUCH_IRQ 32                

// SPI 설정
#define SPI_FREQUENCY  40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000

// 폰트 설정
#define LOAD_GLCD   // 기본 폰트
#define LOAD_FONT2  // 추가 폰트
```

## 테스트 파일 설명

### LCD_Basic_Test.ino
- 기본 LCD 기능 테스트 (색상, 도형, 텍스트)
- 시리얼 모니터를 통한 디버깅 메시지 출력

### LCD_Touch_Test.ino
- 터치 기능 테스트 
- 터치 좌표 감지 및 표시 기능 구현 시도

## 다음 단계 계획

1. SquareLine Studio로 생성한 UI 적용
2. 센서 데이터 시각화 화면 구현
3. 사용자 인터페이스 최적화
4. LCD 및 터치 기능과 메인 프로그램 통합

## 주의사항
- ESP32-WROOM-32의 메모리 제한을 고려하여 개발
- SPI 버스 공유 시 장치 간 간섭 가능성 주의
- 폰트 설정이 메모리 사용량에 영향을 미침
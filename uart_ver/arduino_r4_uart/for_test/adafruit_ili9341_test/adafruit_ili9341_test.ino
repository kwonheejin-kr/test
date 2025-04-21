/*
 * Adafruit ILI9341 Test for Arduino UNO R4 WiFi
 * 표준 Adafruit 라이브러리를 사용하여 LCD 문제 해결
 */

#include <SPI.h>
#include <Adafruit_GFX.h>    // Adafruit 그래픽 코어 라이브러리
#include <Adafruit_ILI9341.h> // ILI9341 LCD 라이브러리

// LCD 핀 정의
#define TFT_CS    10  // 칩 선택 핀
#define TFT_DC     9  // 데이터/명령 선택 핀
#define TFT_RST   A0  // 리셋 핀
#define TFT_BL    A1  // 백라이트 핀

// 색상 정의
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// Adafruit ILI9341 객체 생성
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  delay(1000);  // 안정화를 위한 지연
  Serial.println("\nAdafruit ILI9341 Test for Arduino UNO R4 WiFi");
  
  // 백라이트 설정
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);  // 백라이트 켜기
  Serial.println("Backlight turned ON");
  
  // SPI 저속 초기화
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  
  // LCD 초기화
  Serial.println("Initializing ILI9341 LCD...");
  tft.begin();  // 기본 초기화
  
  // LCD 정보 출력
  Serial.print("Display Driver ID: 0x");
  Serial.println(tft.readcommand8(ILI9341_RDMODE), HEX);
  Serial.print("Display Power Mode: 0x");
  Serial.println(tft.readcommand8(ILI9341_RDMADCTL), HEX);
  Serial.print("Display MADCTL Mode: 0x");
  Serial.println(tft.readcommand8(ILI9341_RDPIXFMT), HEX);
  Serial.print("Display Pixel Format: 0x");
  Serial.println(tft.readcommand8(ILI9341_RDIMGFMT), HEX);
  
  // 해상도 확인
  Serial.print("Display Width: ");
  Serial.println(tft.width());
  Serial.print("Display Height: ");
  Serial.println(tft.height());
  
  // LCD 테스트 시작
  Serial.println("Running LCD tests...");
  
  // LCD 기본 테스트 수행
  runBasicTests();
}

void loop() {
  // 3초마다 색상 변경
  static unsigned long lastTime = 0;
  static uint8_t colorIndex = 0;
  
  if (millis() - lastTime > 3000) {
    lastTime = millis();
    
    // 다양한 색상으로 화면 채우기
    switch (colorIndex % 7) {
      case 0:
        Serial.println("Drawing RED screen");
        tft.fillScreen(RED);
        break;
      case 1:
        Serial.println("Drawing GREEN screen");
        tft.fillScreen(GREEN);
        break;
      case 2:
        Serial.println("Drawing BLUE screen");
        tft.fillScreen(BLUE);
        break;
      case 3:
        Serial.println("Drawing CYAN screen");
        tft.fillScreen(CYAN);
        break;
      case 4:
        Serial.println("Drawing MAGENTA screen");
        tft.fillScreen(MAGENTA);
        break;
      case 5:
        Serial.println("Drawing YELLOW screen");
        tft.fillScreen(YELLOW);
        break;
      case 6:
        Serial.println("Drawing WHITE screen");
        tft.fillScreen(WHITE);
        break;
    }
    
    // 화면 중앙에 텍스트 추가
    centeredText();
    
    colorIndex++;
  }
}

void runBasicTests() {
  // 화면 초기화 및 스크린 테스트
  Serial.println("Clearing screen...");
  tft.fillScreen(BLACK);
  delay(500);
  
  // 모든 방향 테스트
  for (uint8_t rotation = 0; rotation < 4; rotation++) {
    tft.setRotation(rotation);
    tft.fillScreen(BLACK);
    tft.setCursor(10, 10);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Rotation ");
    tft.print(rotation);
    tft.println("");
    delay(1000);
  }
  
  // 최종 회전 설정
  tft.setRotation(1); // 가로 방향
  
  // 전체 화면 색상 테스트
  Serial.println("Color test...");
  tft.fillScreen(RED);
  delay(1000);
  tft.fillScreen(GREEN);
  delay(1000);
  tft.fillScreen(BLUE);
  delay(1000);
  
  // 체커보드 패턴 (단색보다 시각적으로 구분하기 쉬움)
  Serial.println("Checkerboard pattern test...");
  int blockSize = 20;
  for (int y = 0; y < tft.height(); y += blockSize) {
    for (int x = 0; x < tft.width(); x += blockSize) {
      if ((x/blockSize + y/blockSize) % 2) {
        tft.fillRect(x, y, blockSize, blockSize, RED);
      } else {
        tft.fillRect(x, y, blockSize, blockSize, BLUE);
      }
    }
  }
  
  delay(2000);
  
  // 그래픽 테스트
  tft.fillScreen(BLACK);
  
  // 텍스트 출력
  tft.setCursor(10, 10);
  tft.setTextColor(WHITE);
  tft.setTextSize(2);
  tft.println("Adafruit ILI9341");
  tft.setCursor(10, 40);
  tft.println("Display Test");
  
  // 선 그리기
  tft.drawLine(10, 70, tft.width()-10, 70, YELLOW);
  
  // 사각형 그리기
  tft.drawRect(10, 90, 100, 50, CYAN);
  tft.fillRect(150, 90, 100, 50, GREEN);
  
  // 원 그리기
  tft.drawCircle(50, 180, 30, MAGENTA);
  tft.fillCircle(160, 180, 30, RED);
  
  delay(3000);
}

void centeredText() {
  // 화면 중앙에 현재 테스트 정보 표시
  int16_t x1, y1;
  uint16_t w, h;
  
  tft.setTextSize(1);
  tft.setTextColor(BLACK);
  
  String testMsg = "Adafruit ILI9341 Test";
  tft.getTextBounds(testMsg, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(tft.width()/2 - w/2, tft.height()/2 - h/2);
  tft.println(testMsg);
  
  tft.setTextSize(1);
  testMsg = "Arduino UNO R4 WiFi";
  tft.getTextBounds(testMsg, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(tft.width()/2 - w/2, tft.height()/2 + 20);
  tft.println(testMsg);
}

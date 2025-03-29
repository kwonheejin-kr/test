#include <TFT_eSPI.h>
#include <SPI.h>

// TFT 객체 생성
TFT_eSPI tft = TFT_eSPI();

// 백라이트 핀 설정
#define TFT_BL 17

void setup() {
  // 시리얼 통신 초기화 (높은 속도로 설정)
  Serial.begin(115200);
  delay(1000); // 시리얼 포트 안정화 대기
  Serial.println("\n\n========== ESP32 LCD 테스트 시작 ==========");
  
  // 백라이트 핀 설정
  Serial.println("백라이트 핀(GPIO17) 초기화 중...");
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  Serial.println("백라이트 켜짐");
  
  // SPI 버스 상태 확인
  Serial.println("SPI 버스 확인 중...");
  Serial.print("MOSI: GPIO"); Serial.println(13);
  Serial.print("MISO: GPIO"); Serial.println(12);
  Serial.print("SCK: GPIO"); Serial.println(14);
  Serial.print("CS: GPIO"); Serial.println(15);
  Serial.print("DC: GPIO"); Serial.println(2);
  Serial.print("RST: GPIO"); Serial.println(4);
  
  // TFT 초기화
  Serial.println("TFT 드라이버 초기화 중...");
  tft.init(); // void 함수이므로 반환 값 체크하지 않음
  Serial.println("TFT 초기화 시도 완료");
  
  // TFT 정보 표시
  Serial.print("TFT 너비: "); Serial.println(tft.width());
  Serial.print("TFT 높이: "); Serial.println(tft.height());
  Serial.print("현재 회전 설정: "); Serial.println(tft.getRotation());
  
  // 화면 테스트 시작
  Serial.println("\n----- 화면 테스트 시작 -----");
  
  // 화면 채우기 테스트
  Serial.println("1. 화면 채우기 테스트");
  
  Serial.println("  검은색 화면 채우기");
  tft.fillScreen(TFT_BLACK);
  delay(500);
  
  Serial.println("  빨간색 화면 채우기");
  tft.fillScreen(TFT_RED);
  delay(500);
  
  Serial.println("  초록색 화면 채우기");
  tft.fillScreen(TFT_GREEN);
  delay(500);
  
  Serial.println("  파란색 화면 채우기");
  tft.fillScreen(TFT_BLUE);
  delay(500);
  
  Serial.println("  검은색 화면 채우기 (텍스트 배경)");
  tft.fillScreen(TFT_BLACK);
  delay(500);
  
  // 텍스트 테스트
  Serial.println("\n2. 텍스트 표시 테스트");
  
  // 폰트 정보 출력
  Serial.println("  폰트 설정 확인:");
  #ifdef LOAD_GLCD
    Serial.println("  - GLCD 폰트 로드됨 (기본 폰트)");
  #else
    Serial.println("  - GLCD 폰트 로드되지 않음 (기본 폰트 없음)");
  #endif
  
  #ifdef LOAD_FONT2
    Serial.println("  - Font2 로드됨");
  #else
    Serial.println("  - Font2 로드되지 않음");
  #endif
  
  Serial.println("  기본 폰트로 텍스트 표시 시도");
  tft.setTextFont(1);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  tft.setCursor(10, 10);
  tft.print("Hello World");
  Serial.println("  '텍스트 표시 완료' - 화면에 'Hello World'가 보이나요?");
  
  Serial.println("  텍스트 색상 변경 시도");
  tft.setTextColor(TFT_RED);
  tft.setCursor(10, 40);
  tft.print("Red Text");
  Serial.println("  빨간색 텍스트 표시 완료");
  
  // 도형 테스트
  Serial.println("\n3. 도형 테스트");
  
  Serial.println("  사각형 그리기");
  tft.drawRect(10, 80, 100, 50, TFT_YELLOW);
  Serial.println("  사각형 표시 완료");
  
  Serial.println("  원 그리기");
  tft.fillCircle(160, 100, 30, TFT_CYAN);
  Serial.println("  원 표시 완료");
  
  Serial.println("  선 그리기");
  tft.drawLine(10, 150, 230, 150, TFT_GREEN);
  Serial.println("  선 표시 완료");
  
  Serial.println("\n=== 테스트 완료 ===");
  Serial.println("화면에 어떤 것이 보이나요? (색상, 도형, 텍스트 등)");
}

void loop() {
  // 10초마다 상태 메시지 출력
  static unsigned long lastTime = 0;
  if (millis() - lastTime > 10000) {
    lastTime = millis();
    Serial.println("LCD 테스트 실행 중... 화면에 무엇이 보이나요?");
  }
}
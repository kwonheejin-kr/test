// 파일: sketch_mar28b.ino
// 다음과 같이 변경하세요.

#include <TFT_eSPI.h>
#include <SPI.h>
#include <EEPROM.h>  // 보정 데이터 저장용

TFT_eSPI tft = TFT_eSPI();
#define TFT_BL 17

// 터치 보정 관련 상수 및 변수
#define CAL_POINTS 5  // 중앙 + 4개 코너
#define EEPROM_CAL_ADDR 0
#define CAL_MARGIN 20  // 화면 가장자리에서의 여백

int calTouchX[CAL_POINTS]; // 보정시 터치 X 좌표
int calTouchY[CAL_POINTS]; // 보정시 터치 Y 좌표
int calScreenX[CAL_POINTS]; // 화면상 표시 X 좌표
int calScreenY[CAL_POINTS]; // 화면상 표시 Y 좌표
bool calDataExists = false; // 보정 데이터 존재 여부

// 애플리케이션 상태 관리
enum AppState {
  STATE_NORMAL,
  STATE_CALIBRATING,
  STATE_SETTINGS
};

AppState currentState = STATE_NORMAL;
int currentCalPoint = 0;

bool touchDebug = true; // 디버깅 정보 출력 여부

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n== 터치스크린 보정 테스트 ==");
  
  // EEPROM 초기화
  EEPROM.begin(512);
  
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  
  tft.init();
  tft.setRotation(1); // 다른 회전 값 시도 (0, 1, 2, 3)
  tft.fillScreen(TFT_BLACK);
  
  // 터치 초기화 - 중요! 이 줄이 없어서 터치가 작동하지 않았습니다
  uint16_t calData[5] = { 195, 3723, 268, 3618, 7 };
  tft.setTouch(calData);
  
  // 보정 데이터 로드 시도
  if (loadCalibration()) {
    Serial.println("보정 데이터 로드 성공");
    calDataExists = true;
    drawMainScreen();
  } else {
    Serial.println("보정 데이터 없음. 보정 모드 시작");
    startCalibration();
  }
}

void loop() {
  uint16_t touchX = 0, touchY = 0;
  
  // SPI 설정을 명시적으로 구성하여 터치 읽기
  bool touched = false;
  touched = tft.getTouch(&touchX, &touchY);
  
  // 터치 보정 관련 디버깅 정보 출력 - 터치 좌표 유효성 검사 추가
  if (touched && (touchX < 150 || touchY < 150 || touchX > 4000 || touchY > 4000)) {
    touched = false;
  }
  
  // 디버깅 정보 출력
  if (touchDebug) {
    static uint32_t debugTimer = 0;
    if (millis() - debugTimer > 500) { // 0.5초마다 출력
      Serial.print("터치 감지: "); 
      Serial.print(touched ? "O" : "X");
      if (touched) {
        Serial.print(" 좌표: (");
        Serial.print(touchX);
        Serial.print(",");
        Serial.print(touchY);
        Serial.println(")");
      } else {
        Serial.println();
      }
      debugTimer = millis();
    }
  }
  
  if (touched) {
    Serial.print("원본 터치: ("); Serial.print(touchX);
    Serial.print(","); Serial.print(touchY); Serial.println(")");
    
    if (currentState == STATE_CALIBRATING) {
      handleCalibrationTouch(touchX, touchY);
    } else if (currentState == STATE_SETTINGS) {
      handleSettingsTouch(touchX, touchY);
    } else {
      // 일반 상태에서는 보정된 좌표로 변환
      int16_t x, y;
      if (calDataExists) {
        x = calibrateX(touchX, touchY);
        y = calibrateY(touchX, touchY);
      } else {
        // 임시 방편 매핑 (보정 데이터가 없을 경우)
        x = map(touchX, 100, 300, 0, tft.width());
        y = map(touchY, 50, 200, 0, tft.height());
        
        // Y축 반전 (필요에 따라 주석 처리)
        y = tft.height() - y;
      }
      
      Serial.print("변환된 좌표: ("); Serial.print(x);
      Serial.print(","); Serial.print(y); Serial.println(")");
      
      // 화면 범위 내인지 확인
      if (x >= 0 && x < tft.width() && y >= 0 && y < tft.height()) {
        // 화면 하단 설정 버튼 영역 확인
        if (y > tft.height() - 40 && x > tft.width() - 60) {
          // 설정 버튼 클릭
          enterSettingsMode();
        } else {
          // 일반 터치 처리
          tft.fillCircle(x, y, 3, TFT_YELLOW);
        }
      }
    }
  }
  delay(10);
}

// 메인 화면 그리기
void drawMainScreen() {
  tft.fillScreen(TFT_BLACK);
  drawGrid();
  
  // 설정 버튼 추가
  tft.fillRect(tft.width() - 60, tft.height() - 40, 60, 40, TFT_DARKGREY);
  tft.setCursor(tft.width() - 50, tft.height() - 25);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println("설정");
}

// 격자 그리기
void drawGrid() {
  // 격자 그리기
  for (int x = 0; x < tft.width(); x += 40) {
    tft.drawFastVLine(x, 0, tft.height(), TFT_DARKGREY);
  }
  for (int y = 0; y < tft.height(); y += 40) {
    tft.drawFastHLine(0, y, tft.width(), TFT_DARKGREY);
  }
  
  // 보정 지점 표시
  if (calDataExists) {
    for (int i = 0; i < CAL_POINTS; i++) {
      tft.drawCircle(calScreenX[i], calScreenY[i], 3, TFT_CYAN);
    }
  }
}

// 설정 모드 진입
void enterSettingsMode() {
  currentState = STATE_SETTINGS;
  tft.fillScreen(TFT_NAVY);
  
  // 보정 버튼
  tft.fillRect(40, 60, 160, 40, TFT_DARKGREY);
  tft.setCursor(70, 75);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("화면 보정");
  
  // 돌아가기 버튼
  tft.fillRect(40, 120, 160, 40, TFT_DARKGREY);
  tft.setCursor(70, 135);
  tft.println("돌아가기");
}

// 설정 화면에서 터치 처리
void handleSettingsTouch(uint16_t touchX, uint16_t touchY) {
  int16_t x, y;
  if (calDataExists) {
    x = calibrateX(touchX, touchY);
    y = calibrateY(touchX, touchY);
  } else {
    x = map(touchX, 100, 300, 0, tft.width());
    y = map(touchY, 50, 200, 0, tft.height());
    y = tft.height() - y;
  }
  
  // 보정 버튼 영역
  if (x >= 40 && x <= 200 && y >= 60 && y <= 100) {
    startCalibration();
  }
  // 돌아가기 버튼 영역
  else if (x >= 40 && x <= 200 && y >= 120 && y <= 160) {
    currentState = STATE_NORMAL;
    drawMainScreen();
  }
}

// 보정 시작
void startCalibration() {
  currentState = STATE_CALIBRATING;
  currentCalPoint = 0;
  
  // 보정 포인트 위치 계산
  // 중앙
  calScreenX[0] = tft.width() / 2;
  calScreenY[0] = tft.height() / 2;
  
  // 좌상단
  calScreenX[1] = CAL_MARGIN;
  calScreenY[1] = CAL_MARGIN;
  
  // 우상단
  calScreenX[2] = tft.width() - CAL_MARGIN;
  calScreenY[2] = CAL_MARGIN;
  
  // 좌하단
  calScreenX[3] = CAL_MARGIN;
  calScreenY[3] = tft.height() - CAL_MARGIN;
  
  // 우하단
  calScreenX[4] = tft.width() - CAL_MARGIN;
  calScreenY[4] = tft.height() - CAL_MARGIN;
  
  drawCalibrationScreen();
}

// 보정 화면 표시
void drawCalibrationScreen() {
  tft.fillScreen(TFT_BLACK);
  
  // 안내 텍스트
  tft.setCursor(30, 30);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.println("화면 보정: 십자 표시를 터치하세요");
  tft.setCursor(tft.width()/2 - 40, tft.height()-20);
  tft.print("포인트 ");
  tft.print(currentCalPoint + 1);
  tft.print("/");
  tft.print(CAL_POINTS);
  
  // 보정 포인트 십자 표시
  int x = calScreenX[currentCalPoint];
  int y = calScreenY[currentCalPoint];
  
  tft.drawLine(x-10, y, x+10, y, TFT_RED);
  tft.drawLine(x, y-10, x, y+10, TFT_RED);
  tft.drawCircle(x, y, 5, TFT_RED);
}

// 보정 터치 처리
void handleCalibrationTouch(uint16_t touchX, uint16_t touchY) {
  // 터치 좌표 저장
  calTouchX[currentCalPoint] = touchX;
  calTouchY[currentCalPoint] = touchY;
  
  Serial.print("보정 포인트 ");
  Serial.print(currentCalPoint);
  Serial.print(": 화면(");
  Serial.print(calScreenX[currentCalPoint]);
  Serial.print(",");
  Serial.print(calScreenY[currentCalPoint]);
  Serial.print(") 터치(");
  Serial.print(touchX);
  Serial.print(",");
  Serial.print(touchY);
  Serial.println(")");
  
  // 다음 포인트로
  currentCalPoint++;
  
  // 모든 포인트 완료 확인
  if (currentCalPoint >= CAL_POINTS) {
    // 보정 완료
    calDataExists = true;
    saveCalibration();
    
    // 메인 화면으로 돌아가기
    currentState = STATE_NORMAL;
    drawMainScreen();
    
    Serial.println("보정 완료");
  } else {
    // 다음 포인트 표시
    drawCalibrationScreen();
  }
  
  // 터치 디바운싱
  delay(500);
}

// 보정 데이터 저장
bool saveCalibration() {
  // 매직 넘버 저장 (유효한 보정 데이터 확인용)
  EEPROM.writeUInt(EEPROM_CAL_ADDR, 0xCAFECAFE);
  
  int addr = EEPROM_CAL_ADDR + 4;
  for (int i = 0; i < CAL_POINTS; i++) {
    EEPROM.writeInt(addr, calTouchX[i]); addr += 4;
    EEPROM.writeInt(addr, calTouchY[i]); addr += 4;
    EEPROM.writeInt(addr, calScreenX[i]); addr += 4;
    EEPROM.writeInt(addr, calScreenY[i]); addr += 4;
  }
  
  return EEPROM.commit();
}

// 보정 데이터 로드
bool loadCalibration() {
  // 매직 넘버 확인
  uint32_t magic = EEPROM.readUInt(EEPROM_CAL_ADDR);
  if (magic != 0xCAFECAFE) {
    return false;
  }
  
  int addr = EEPROM_CAL_ADDR + 4;
  for (int i = 0; i < CAL_POINTS; i++) {
    calTouchX[i] = EEPROM.readInt(addr); addr += 4;
    calTouchY[i] = EEPROM.readInt(addr); addr += 4;
    calScreenX[i] = EEPROM.readInt(addr); addr += 4;
    calScreenY[i] = EEPROM.readInt(addr); addr += 4;
  }
  
  return true;
}

// 가장 가까운 4개의 보정 지점 찾기
void findClosestCalPoints(int touchX, int touchY, int closest[], int distances[]) {
  for (int i = 0; i < 4; i++) {
    closest[i] = -1;
    distances[i] = 99999;
  }
  
  for (int i = 0; i < CAL_POINTS; i++) {
    int dx = touchX - calTouchX[i];
    int dy = touchY - calTouchY[i];
    int distance = dx*dx + dy*dy;
    
    // 삽입 정렬로 상위 4개 유지
    for (int j = 0; j < 4; j++) {
      if (distance < distances[j]) {
        // 이동
        for (int k = 3; k > j; k--) {
          closest[k] = closest[k-1];
          distances[k] = distances[k-1];
        }
        closest[j] = i;
        distances[j] = distance;
        break;
      }
    }
  }
}

// X 좌표 보정 - 가장 가까운 4개 포인트 기반 가중 평균
int calibrateX(int touchX, int touchY) {
  int closest[4]; // 가장 가까운 4개 지점의 인덱스
  int distances[4]; // 거리
  
  findClosestCalPoints(touchX, touchY, closest, distances);
  
  float weightedX = 0;
  float totalWeight = 0;
  
  for (int i = 0; i < 4; i++) {
    if (closest[i] != -1) {
      // 거리의 역수를 가중치로 사용
      float weight = 1.0 / (distances[i] + 1.0); // 0으로 나누기 방지
      totalWeight += weight;
      weightedX += calScreenX[closest[i]] * weight;
    }
  }
  
  return (int)(weightedX / totalWeight);
}

// Y 좌표 보정 - 가장 가까운 4개 포인트 기반 가중 평균
int calibrateY(int touchX, int touchY) {
  int closest[4]; // 가장 가까운 4개 지점의 인덱스
  int distances[4]; // 거리
  
  findClosestCalPoints(touchX, touchY, closest, distances);
  
  float weightedY = 0;
  float totalWeight = 0;
  
  for (int i = 0; i < 4; i++) {
    if (closest[i] != -1) {
      // 거리의 역수를 가중치로 사용
      float weight = 1.0 / (distances[i] + 1.0); // 0으로 나누기 방지
      totalWeight += weight;
      weightedY += calScreenY[closest[i]] * weight;
    }
  }
  
  return (int)(weightedY / totalWeight);
}
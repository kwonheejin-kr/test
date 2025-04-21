#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

// PCM5102와 연결할 I2S 핀 정의
const uint8_t I2S_SCK = 5;       /* I2S 비트 클럭 (BCK) */
const uint8_t I2S_WS = 25;       /* I2S 워드 셀렉트/좌우 채널 (LRCLK) */
const uint8_t I2S_SDOUT = 26;    /* I2S 데이터 출력 (DIN) */

I2SClass i2s;
BluetoothA2DPSink a2dp_sink(i2s);

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Bluetooth Audio (PCM5102) System Started");
  
  // I2S 핀 설정
  i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
  
  // I2S 초기화
  if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("I2S 초기화 실패!");
    while (1); // 에러 시 정지
  }
  
  Serial.println("I2S 초기화 성공");
  
  // 블루투스 A2DP 시작
  a2dp_sink.start("ESP32_Audio");
  Serial.println("블루투스 시작. 'ESP32_Audio'에 연결하세요.");
}

void loop() {
  // 워치독 타이머 리셋을 위한 최소 딜레이
  delay(100);
}
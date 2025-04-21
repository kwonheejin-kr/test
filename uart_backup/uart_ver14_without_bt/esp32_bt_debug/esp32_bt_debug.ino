#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"

BluetoothA2DPSink a2dp_sink;

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32 Bluetooth Audio Sink 시작!");

  // I2S 핀 설정
  i2s_pin_config_t my_pin_config = {
    .bck_io_num = 26,       // BCLK
    .ws_io_num = 22,        // LRCK
    .data_out_num = 25,     // DATA
    .data_in_num = -1       // 사용 안 함
  };

  // I2S 핀 수동 설정 (구버전 방식)
  i2s_set_pin(I2S_NUM_0, &my_pin_config);

  // 블루투스 시작 (구버전 호환 방식)
  a2dp_sink.start("ESP32_BT_SPEAKER");
}

void loop() {
  // 아무것도 안 해도 블루투스 음악 수신 시 자동 재생
}

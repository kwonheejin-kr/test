#include "BluetoothA2DPSink.h"
#include "driver/i2s.h"

// I2S 핀 구성 (ESP32-CAM 최적화)
#define I2S_BCLK  33  // 비트 클록
#define I2S_LRCLK 19  // 좌/우 채널 클럭 
#define I2S_DOUT  18  // 데이터 출력

BluetoothA2DPSink a2dp_sink;
TaskHandle_t bluetoothTask_handle;

// 블루투스 태스크 (코어 0에서 실행)
void bluetoothTask(void *parameter) {
  while(1) {
    // 블루투스 스택에 시간 할당
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// I2S 핀 설정
void setupI2SPins() {
  // I2S 핀 구성 설정
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRCLK,
    .data_out_num = I2S_DOUT,
    .data_in_num = -1   // 입력 사용 안 함
  };
  
  i2s_set_pin(I2S_NUM_0, &pin_config);
  Serial.printf("I2S 핀 설정: BCLK=%d, LRCLK=%d, DOUT=%d\n", 
                I2S_BCLK, I2S_LRCLK, I2S_DOUT);
}

// 블루투스 오디오 초기화
void initializeBluetoothAudio() {
  // I2S 설정
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 44100,  // 표준 오디오 샘플레이트
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,    // DMA 버퍼 개수 증가
    .dma_buf_len = 64,     // 버퍼 크기 최적화
    .use_apll = true,      // 오디오 PLL 사용으로 품질 향상
    .tx_desc_auto_clear = true
  };
  
  // I2S 드라이버 초기화
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  
  // 핀 설정
  setupI2SPins();
  
  // 블루투스 A2DP 초기화 및 시작
  // 주의: 최신 라이브러리에서는 set_i2s_config() 대신 내부 설정 사용
  a2dp_sink.start("ESP32-CAM_Audio");
  
  Serial.println("블루투스 A2DP 오디오 초기화 완료");
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-CAM 블루투스 오디오 시작");
  
  // 블루투스 스택을 위한 별도 태스크 생성 (코어 0에서 실행)
  xTaskCreatePinnedToCore(
    bluetoothTask,         // 태스크 함수
    "BluetoothTask",       // 태스크 이름
    4096,                  // 스택 크기
    NULL,                  // 파라미터
    1,                     // 우선순위 (낮음)
    &bluetoothTask_handle, // 태스크 핸들
    0                      // 코어 0에서 실행
  );
  
  // 메인 코드는 코어 1에서 실행
  initializeBluetoothAudio();
  
  Serial.println("설정 완료. 블루투스 장치에서 'ESP32-CAM_Audio'를 찾아 연결하세요.");
}

void loop() {
  // 메인 루프는 코어 1에서 실행됨
  // 여기서는 다른 작업(센서 읽기, 디스플레이 등) 수행 가능
  delay(1000);
}
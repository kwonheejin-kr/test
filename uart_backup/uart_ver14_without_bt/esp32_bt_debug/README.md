# ESP32 블루투스 오디오 디버그 모듈

## 변경 사항 기록

### 버전 0.1.4 (2025-04-01)

#### 변경사항
1. MCLK(SCK) 핀 설정 추가
   - MCLK: GPIO32 - 마스터 클럭 연결
   - Arduino IDE 호환 방식으로 간단하게 구현 (pinMode 사용)

2. ESP32 Arduino 지원 범위 내 기능만 사용하도록 변경
   - ESP-IDF 전용 함수(ledc, periph_module 등) 대신 기본 GPIO 기능 사용
   - 모든 Arduino IDE 환경에서 호환되도록 설계

3. 코드 안정성 개선
   - 컴파일 오류 해결
   - 간소화된 MCLK 설정 방식 적용

### 버전 0.1.3 (2025-04-01)

#### 변경사항
1. 원래 사용하던 I2S 핀 번호로 복원
   - BCLK: GPIO33 (이전: GPIO26)
   - LRCLK: GPIO19 (이전: GPIO25)
   - DOUT: GPIO18 (이전: GPIO33)

2. 구버전 BluetoothA2DPSink 라이브러리와 호환되도록 수정
   - `set_pin_config()`, `set_i2s_config()` 함수 호출 제거
   - 직접 `i2s_set_pin()` 함수 사용
   - 불필요한 i2s_config 구성 코드 제거

3. 코드 안정성 개선
   - 라이브러리 호환성 문제로 인한 컴파일 오류 해결

### 버전 0.1.2 (2025-04-01)

#### 변경사항
1. BluetoothA2DPSink 객체의 핀 설정 변경
   - `set_pin_config()` 함수를 사용하여 I2S 핀 설정 변경
   - `set_i2s_config()` 함수를 사용하여 I2S 설정 변경

2. ESP32 기본 I2S 핀 사용
   - BCLK: GPIO26 (이전: GPIO33)
   - LRCLK: GPIO25 (이전: GPIO19)
   - DOUT: GPIO33 (이전: GPIO22) - ESP32 기본 I2S 핀 사용으로 변경

3. 코드 단순화
   - delay() 함수 추가로 초기화 순서 안정화
   - I2S 설정 변경 (44.1kHz, 16bit, 스테레오 모드)

### 버전 0.1.1 (2025-04-01)

#### 변경사항
1. Guru Meditation Error (LoadProhibited) 오류 해결
   - BluetoothA2DPSink 객체 선언 위치 변경
   - 사용자 정의 I2S 핀 배치 대신 ESP32 기본 I2S 핀 사용
     - BCLK: GPIO26 (이전: GPIO33)
     - LRCLK: GPIO25 (이전: GPIO19)
     - DOUT: GPIO22 (이전: GPIO18)

2. set_i2s_config 함수 호출 제거
   - 구버전 BluetoothA2DPSink 라이브러리에서 지원하지 않는 함수 사용 제거
   - 기본 내장 I2S 설정 사용

3. 코드 단순화
   - 코드 구조 간소화
   - delay() 추가로 초기화 순서 안정화

### 버전 0.1.0 (2025-04-01)

#### 변경사항
1. ESP32_uart.ino에서 사용하는 I2S 핀 정의 적용 (PCM5102 오디오 출력용)
   - SCK: GPIO32
   - BCK: GPIO33
   - DIN: GPIO18
   - LRCK: GPIO19

2. 구버전 BluetoothA2DPSink 라이브러리 호환 기능 구현
   - 수동 I2S 핀 설정 방식 사용
   - 블루투스 A2DP 싱크 초기화 직접 설정

3. 디버깅 기능 추가
   - 블루투스 상태 모니터링 (5초마다 갱신)
   - 시리얼 명령어 처리:
     - 'd': 디버깅 로그 켜기/끄기
     - 'r': 블루투스 재시작
     - 'i': I2S 핀 정보 출력

#### 참고사항
- ESP32-WROOM-32 모듈 사용 시 고려사항:
  - 블루투스와 Wi-Fi 동시 사용 시 메모리 제약 가능성
  - 오디오 처리를 위한 DMA 버퍼 크기 최적화 필요
  - I2S 출력 핀은 다른 용도로 사용 불가
- PCM5102 DAC와 TPA3116 앰프 연결 시 결선 참고:
  - PCM5102 VCC: 3.3V
  - PCM5102 GND: GND
  - TPA3116 전원: 12-24V DC 필요

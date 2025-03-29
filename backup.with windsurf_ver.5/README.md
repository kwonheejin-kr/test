# ESP32 스마트팜 프로젝트

## 펌웨어 변경 이력
### 버전 1.2.5 (2025-03-26)
- Bluetooth A2DP 설정 변경 (ESP_I2S 설정 추가)
- I2S 설정 변경 및 안정성 향상
- 오디오 출력 안정성 향상

### 버전 1.2.4 (2025-03-26)
- UI 요소 선언 추가 (ui_HeartResult 변수 ui.h에 추가)
- 이벤트 핸들러 수정 (ui_event_start_measuring → HeartBeatMeasuringStart)
- BluetoothA2DPSink 설정 방식 개선 (set_pin_config → set_i2s_port 및 set_i2s_config)
- 컴파일 오류 해결 및 코드 안정성 향상

### 버전 1.2.3 (2025-03-27)
- 심박 센서 변수명 통일 (heartRateSensor → particleSensor)
- 중복된 measure_heart_rate 함수 제거
- HeartBeatMeasuringStart 함수 개선 - 센서 초기화 코드 제거 (이미 ESP32_SmartFarm.ino에서 초기화됨)
- check_sensor_data 함수 변수명 수정

### 버전 1.2.2 (2025-03-26)
- uc628uc2b5ub3c4 uc13cuc11c ub77cuc774ube0cub7ecub9ac uc218uc815 (DHT → SHT31)
- uc2ecubc15 uc13cuc11c ub77cuc774ube0cub7ecub9ac uc218uc815 (Adafruit_MAX30102 → SparkFun MAX3010x)
- uc13cuc11c ubcc0uc218uba85 ud1b5uc77c (dht → sht31)
- SD uce74ub4dc CS ud540 ubc88ud638 uc218uc815 (13 → 5)
- I2C SDA/SCL ud540 uc815uc758 ucd94uac00
- ud130uce58 uc0acuc6b4ub4dc ud30cuc77c uc774ub984 uc218uc815 (touch_sound.wav → touch.wav)

### 버전 1.2.1 (2025-03-26)
- LVGL ub77cuc774ube0cub7ecub9ac uacbdub85c uc218uc815 (ui.h ud30cuc77cuc758 ud558ub4dcucf54ub529ub41c uacbdub85c uc81cuac70)
- ucf54ub4dc uc774uc2dduc131 uac1cuc120

### 버전 1.2.0 (2025-03-26)
- SD 카드에 touch.wav 파일 업로드 uc2dcuc2a4ud15c uc644uc131
- uc2dcuc2a4ud15c ucd5cuc801ud654 ubc0f uc624ub958 ucc98ub9ac uac15ud654
- uc804ub825 uad00ub9ac uae30ub2a5 ucd94uac00
- uc911ubcf5 ucf54ub4dc uc815ub9ac ubc0f ucd5cuc801ud654

### 버전 1.1.0 (2025-03-26)
- ube14ub8e8ud22cuc2a4 A2DP uc2f1ud06c uae30ub2a5 uad6cud604 (ud734ub300ud3f0 uc624ub514uc624 uc218uc2e0)
- I2S uc624ub514uc624 ucd9cub825 uc2dcuc2a4ud15c uad6cud604
- ud130uce58 uc0acuc6b4ub4dc ud6a8uacfc uac1cuc120

### 버전 1.0.0 (2025-03-26)
- ucd08uae30 ub9b4ub808uc774 uc81cuc5b4 uc2dcuc2a4ud15c uad6cud604
- ESP32-Arduino I2C ud1b5uc2e0 uad6cud604
- uc2ecubc15uc218/SpO2 uce21uc815 uae30ub2a5 uad6cud604

## 구현된 기능
### 1. uc2ecubc15uc218/SpO2 uce21uc815 uae30ub2a5
- 5ucd08uac04 uce21uc815 uc9c4ud589
- uc2e4uc2dcuac04 uc9c4ud589ub960 ud45cuc2dc
- uce21uc815 uacb0uacfc ud45cuc2dc ubc0f SD uce74ub4dc uc800uc7a5
  
### 2. ub9b4ub808uc774 uc81cuc5b4 uc2dcuc2a4ud15c
- ESP32-Arduino I2C ud1b5uc2e0 uad6cud604
- ud32c, LED, uac00uc2b5uae30 uc81cuc5b4 uae30ub2a5
- uc0c1ud0dc ud655uc778 ubc0f UI uc5c5ub370uc774ud2b8 ub85cuc9c1

### 3. ube14ub8e8ud22cuc2a4 uc624ub514uc624 uc2dcuc2a4ud15c
- ube14ub8e8ud22cuc2a4 A2DP uc2f1ud06c uae30ub2a5 uad6cud604
- ud734ub300ud3f0uacfc uc5f0uacb0ud558uc5ec uc624ub514uc624 uc2a4ud2b8ub9acubc0d uc218uc2e0
- I2Sub97c ud1b5ud55c uc624ub514uc624 ucd9cub825
- ud130uce58 uc0acuc6b4ub4dc ud6a8uacfc uc9c0uc6d0

### 4. uc2dcuc2a4ud15c ucd5cuc801ud654 ubc0f uc624ub958 ucc98ub9ac
- SD uce74ub4dc ucd08uae30ud654 uc624ub958 ucc98ub9ac
- uc13cuc11c uc5f0uacb0 uc2e4ud328 uc2dc ub300uc751 ub85cuc9c1

## 하드웨어 연결 정보
### ESP32 핀 할당
- **I2C 마스터**:
  * SDA: GPIO21
  * SCL: GPIO22
- **SHT31 온습도 센서**:
  * SDA: GPIO21 (공유 I2C 버스)
  * SCL: GPIO22 (공유 I2C 버스)
- **MAX30102 심박/SpO2 센서**:
  * SDA: GPIO21 (공유 I2C 버스)
  * SCL: GPIO22 (공유 I2C 버스)
- **SD 카드**:
  * MOSI: GPIO23
  * MISO: GPIO19
  * SCK: GPIO18
  * CS: GPIO5
- **오디오 출력 (I2S)**:
  * BCLK: GPIO26
  * LRCLK: GPIO25
  * DIN: GPIO33
- **TFT LCD 디스플레이**:
  * MOSI: GPIO13
  * MISO: GPIO12
  * SCK: GPIO14
  * CS: GPIO15
  * DC: GPIO2
  * RST: GPIO4
  * BACKLIGHT: GPIO0
  * TOUCH_CS: GPIO27
  * TOUCH_IRQ: GPIO32

### Arduino 핀 할당
- **I2C 슬레이브**:
  * SDA: A4
  * SCL: A5
- **릴레이 제어**:
  * 팬: D2
  * LED: D3
  * 가습기: D4

## 코드 변경사항
### ui_events.c
- uc2ecubc15uc218/SpO2 uce21uc815 ub85cuc9c1 ucd94uac00
- ub9b4ub808uc774 uc81cuc5b4 uc774ubca4ud2b8 ud578ub4e4ub7ec ucd94uac00
- I2C uba85ub839 uc804uc1a1 ud568uc218 uad6cud604
- ube14ub8e8ud22cuc2a4 uc5f0uacb0 ubc0f A2DP uc2f1ud06c uae30ub2a5 uad6cud604
- uc624ub958 ucc98ub9ac ubc0f ubcf5uad6c uba54ucee4ub2c8uc998 uac15ud654

### ui_bluetooth.c
- ube14ub8e8ud22cuc2a4 ud654uba74 uc5f0uacb0 uc0c1ud0dc ud45cuc2dc uae30ub2a5
- uc0c1ud0dc ud45cuc2dc ub808uc774ube14 ud55cuae00ud654

### ui_heartmeasuring.c
- uce21uc815 uacb0uacfc ud45cuc2dc ub808uc774ube14 ucd94uac00
- ud55cuad6duc5b4 uba54uc2dcuc9c0 uc801uc6a9

### ESP32_SmartFarm.ino
- uc13cuc11c ucd08uae30ud654 ubc0f I2C uc124uc815
- ud0c0uc774uba38 ucc98ub9ac ubc0f uc13cuc11c uc5c5ub370uc774ud2b8 ub85cuc9c1
- I2S uc624ub514uc624 uc2dcuc2a4ud15c ucd08uae30ud654 ucd94uac00
- BluetoothA2DPSink ub77cuc774ube0cub7ecub9ac uc801uc6a9

### Arduino_Relay.ino (새 파일)
- I2C uc2acub808uc774ube0c ubaa8ub4dc uc124uc815
- ub9b4ub808uc774 uc81cuc5b4 ub85cuc9c1 uad6cud604
- uc0c1ud0dc uc751ub2f5 uae30ub2a5

## 다음 작업 계획
- uc804uccb4 uc2dcuc2a4ud15c ud1b5ud569 ud14cuc2a4ud2b8
- ub77cuc988ubca0ub9acud30cuc774 ub370uc774ud130 uc804uc1a1 uae30ub2a5 uad6cud604
- ubc30ud130ub9ac ubaa8ub2c8ud130ub9c1 ubc0f uc800uc804ub825 ubaa8ub4dc uad6cud604

## 적용 방법
1. ESP32_SmartFarm 폴더의 코드를 ESP32에 업로드
2. Arduino_Relay 폴더의 코드를 Arduino에 업로드
3. I2C 연결 확인 (ESP32: GPIO21/22, Arduino: A4/A5)
4. 릴레이 연결 확인 (Arduino: D2/D3/D4)
5. SD 카드에 touch.wav 파일 업로드
6. 시스템 전원 공급 및 동작 확인
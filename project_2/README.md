# ESP32 스마트팜 프로젝트

## 프로젝트 개요
- **프로젝트명**: ESP32 SmartFarm
- **버전**: 1.0.9 (2025-03-27)
- **주요 기능**: 온습도 모니터링, 심박/SpO2 측정, 릴레이 제어, SD 카드 저장, 블루투스 연결
- **하드웨어**: ESP32 Dev Module(마스터), Arduino(슬레이브), LCD 모니터, 센서들, 릴레이 제어 장치

## 하드웨어 구성 및 핀 번호
### ESP32 핀 할당
- **I2C 마스터**: SDA(GPIO21), SCL(GPIO22)
- **SHT31 온습도 센서**: SDA(GPIO21), SCL(GPIO22) - 공유 I2C 버스 사용
- **MAX30102 심박/SpO2 센서**: SDA(GPIO21), SCL(GPIO22) - 공유 I2C 버스 사용
- **SD 카드**: MOSI(GPIO23), MISO(GPIO19), SCK(GPIO18), CS(GPIO5)
- **오디오 출력(I2S)**: BCLK(GPIO26), LRCLK(GPIO25), DIN(GPIO33)
- **TFT LCD 디스플레이**: 
  - MOSI(GPIO13), MISO(GPIO12), SCK(GPIO14)
  - CS(GPIO15), DC(GPIO2), RST(GPIO4)
  - BACKLIGHT(GPIO17)
  - TOUCH_CS(GPIO27), TOUCH_IRQ(GPIO32)

### Arduino 핀 할당
- **I2C 슬레이브**: SDA(A4), SCL(A5)
- **릴레이 제어**: 팬(D2), LED(D3), 가습기(D4)

## 사용된 라이브러리
- Adafruit_SHT31 v2.2.2 (온습도 센서)
- SparkFun MAX3010x v1.1.2 (심박/SpO2 센서)
- LVGL v9.2.2 (UI 라이브러리)
- TFT_eSPI (디스플레이 드라이버)
- BluetoothSerial (블루투스 연결)
- ESP_I2S (오디오 출력)
- BluetoothA2DPSink (블루투스 오디오)
- SD (SD 카드 인터페이스)

## 소프트웨어 구성
### 주요 파일 구조
- **ESP32_SmartFarm.ino**: 메인 setup 및 loop 함수, 센서 초기화, 블루투스, I2S 오디오 설정
- **ui_events.cpp**: UI 이벤트 처리 함수, 센서 데이터 처리, 릴레이 제어 로직
- **ui_events.h**: 함수 선언 및 전역 변수 정의
- **ui.h**: UI 요소 선언
- **ui_heartmeasuring.c**: 심박 측정 UI 관련 함수
- **lv_conf.h**: LVGL 구성 파일

## UI 구조 및 기능
### 심박 측정 UI
- 측정 진행률 표시
- 데이터 SD 카드 저장
- 뒤로가기 버튼 기능

### 홈 UI
- 온습도 표시 (5-10초 새로고침)
- 블루투스 페어링 인터페이스
- 릴레이 장치 제어판

### 측정 과정
- 터치로 측정 시작
- 5초 측정 기간
- 실시간 진행률 표시
- 정확도를 위한 평균 계산

## 빌드 설정
### 권장 ESP32 설정
- **보드**: ESP32 Dev Module
- **파티션 스키마**: Huge APP (3MB No OTA/1MB SPIFFS)
- **PSRAM**: Enabled (권장)
- **CPU 주파수**: 240MHz
- **Flash Mode**: QIO
- **Flash 크기**: 4MB

## ESP32 하드웨어 제한 사항 고려 사항
- 제한된 메모리 용량 (520KB SRAM)
- 전력 관리 고려 필요
- I2C 통신 속도 제한
- 동시 작업 수 제한

## 펌웨어 버전 기록

### v1.0.9 (2025-03-27)
- ESP32 파티션 스키마 변경 (Huge APP 3MB)
- lv_conf.h 파일에 __ASSEMBLER__ 조건 추가하여 컴파일러 처리 오류 해결
- LVGL 메모리 파일시스템 설정 추가 (LV_FS_MEMFS_LETTER='M')
- 성공적인 컴파일 및 업로드 완료

### v1.0.8 (2025-03-27)
- lv_conf.h 파일 인코딩 문제 해결 (유니코드 이스케이프 시퀀스를 한글로 복원)
- LVGL 메모리 파일시스템 설정 추가 (LV_FS_MEMFS_LETTER='M')
- "LV_FS_MEMFS_LETTER must be set to a valid value" 오류 수정

### v1.0.7 (2025-03-27)
- TFT_eSPI 라이브러리 설정 문제 해결 (User_Setups 폴더에 설정 파일 추가)
- TOUCH_CS 및 TOUCH_IRQ 핀 설정으로 터치 기능 활성화
- TFT_BL 핀을 GPIO17로 설정하여 백라이트 제어 개선
- getTouch 함수 관련 컴파일 오류 해결
- **변경 사항**: TFT LCD 백라이트 핀 설정 변경 및 주석 수정
  - 백라이트 핀을 GPIO17로 명확하게 정의하고 관련 주석을 보강
  - LCD 초기화 및 백라이트 켜기에 대한 설명 추가
  - 시리얼에 LCD 상태 메시지 추가 및 개선

### v1.0.6 (2025-03-27)
- LVGL v9.2.2 API와 호환되도록 디스플레이 초기화 코드 업데이트
- ESP32 메모리 최적화를 위한 동적 버퍼 할당 방식 적용
- TFT_eSPI 라이브러리 터치 함수 호환성 수정
- 디스플레이 및 터치 콜백 함수 개선
- 메모리 효율성 향상을 위한 버퍼 크기 최적화

### v1.0.5 (2025-03-27)
- TFT LCD 디스플레이 설정 변경 (ESP32_SmartFarm.ino에서 설정 변경)
- LCD 백라이트 설정 변경 (GPIO0에서 GPIO17으로 변경)
- TFT_eSPI_Setup.h 파일에서 TFT_eSPI 설정 변경
- LVGL과 TFT_eSPI의 호환성 설정 추가
- setup() 함수에서 LCD 초기화 코드 추가
- loop() 함수에서 LVGL 타이머 핸들러 추가
- 버전 1.0.4에서 1.0.5로 변경

## 현재 처리 중인 문제
1. **LCD 디스플레이 깜빡임**:
   - 가능성 원인: 백라이트 핀과 LCD VCC 공급 전압 차이
   - 가능성 원인: ESP32 리소스 제한으로 인한 LVGL 업데이트 불안정
   - 가능성 원인: 전원 불안정 및 오디오 회로와의 간섭

2. **오디오 노이즈**:
   - 오디오 출력 시 실주파수와 노이즈 발생

## 다음 작업 계획
1. **LCD 디스플레이 안정화**:
   - LCD VCC 전압을 3.3V로 변경 테스트
   - LVGL 타이머 및 버퍼 설정 최적화
   - 전원 공급 안정성 개선

2. **오디오 노이즈 문제 해결**:
   - 오디오 회로 접지 개선
   - 노이즈 필터링 추가

## 사용 방법
1. Arduino IDE에서 ESP32 프로젝트 열기
2. 권장 ESP32 설정 적용
3. 업로드 실행
4. 심박 측정 시 손가락을 센서에 올려 5초간 유지
5. 환경 제어는 홈 화면에서 각 버튼으로 조작

## 데이터 저장 및 관리
- 심박수와 SpO2 데이터는 SD 카드에 CSV 형식으로 저장됨
- 온도 및 습도 데이터도 주기적으로 SD 카드에 기록
- 추후 WiFi를 통해 Raspberry Pi에 데이터 전송 계획

## 주의 사항 및 다음 단계
- 터치 화면 보정이 필요할 수 있음
- SD 카드는 사용 전 FAT32로 포맷 필요
- 블루투스 페어링 시 "ESP32_SmartFarm" 기기 선택
- MAX30102 센서 측정 시 움직임 최소화하여 정확도 확보
- 추후 WiFi 기능 추가를 통한 Raspberry Pi 연동 계획
# ESP32 스마트팜 프로젝트

## 펌웨어 변경 이력
### 버전 1.0.0 (2025-03-26)
- 초기 릴레이 제어 시스템 구현
- ESP32-Arduino I2C 통신 구현
- 심박수/SpO2 측정 기능 구현

## 구현된 기능
### 1. 심박수/SpO2 측정 기능
- 5초간 측정 진행
- 실시간 진행률 표시
- 측정 결과 표시 및 SD 카드 저장
  
### 2. 릴레이 제어 시스템
- ESP32-Arduino I2C 통신 구현
- 팬, LED, 가습기 제어 기능
- 상태 확인 및 UI 업데이트 로직

## 코드 변경사항
### ui_events.c
- 심박수/SpO2 측정 로직 추가
- 릴레이 제어 이벤트 핸들러 추가
- I2C 명령 전송 함수 구현

### ui_heartmeasuring.c
- 측정 결과 표시 레이블 추가
- 한국어 메시지 적용

### ESP32_SmartFarm.ino
- 센서 초기화 및 I2C 설정
- 타이머 처리 및 센서 업데이트 로직

### Arduino_Relay.ino (새 파일)
- I2C 슬레이브 모드 설정
- 릴레이 제어 로직 구현
- 상태 응답 기능

## 다음 작업 계획
- SD 카드 데이터 저장 시스템 완성
- 블루투스 오디오 재생 기능 구현
- 라즈베리파이 데이터 전송 기능

## 적용 방법
1. ESP32_SmartFarm 폴더의 코드를 ESP32에 업로드
2. Arduino_Relay 폴더의 코드를 Arduino에 업로드
3. I2C 연결 확인 (ESP32: GPIO21/22, Arduino: A4/A5)
4. 릴레이 연결 확인 (Arduino: D2/D3/D4)

## 하드웨어 연결
### ESP32
- I2C 마스터: SDA(GPIO21), SCL(GPIO22)
- SHT31 센서: SDA(GPIO21), SCL(GPIO22)
- MAX30102 센서: SDA(GPIO21), SCL(GPIO22)
- SD 카드: MOSI(GPIO23), MISO(GPIO19), SCK(GPIO18), CS(GPIO5)
- 오디오: BCLK(GPIO26), LRCLK(GPIO25), DIN(GPIO33)

### Arduino
- I2C 슬레이브: SDA(A4), SCL(A5)
- 릴레이 제어: 팬(D2), LED(D3), 가습기(D4)
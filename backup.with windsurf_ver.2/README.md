# ESP32 스마트팜 프로젝트

## 펌웨어 변경 이력
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

## 코드 변경사항
### ui_events.c
- uc2ecubc15uc218/SpO2 uce21uc815 ub85cuc9c1 ucd94uac00
- ub9b4ub808uc774 uc81cuc5b4 uc774ubca4ud2b8 ud578ub4e4ub7ec ucd94uac00
- I2C uba85ub839 uc804uc1a1 ud568uc218 uad6cud604
- ube14ub8e8ud22cuc2a4 uc5f0uacb0 ubc0f A2DP uc2f1ud06c uae30ub2a5 uad6cud604

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
- SD uce74ub4dc ub370uc774ud130 uc800uc7a5 uc2dcuc2a4ud15c uc644uc131
- ub77cuc988ubca0ub9acud30cuc774 ub370uc774ud130 uc804uc1a1 uae30ub2a5
- uc804uccb4 uc2dcuc2a4ud15c ud1b5ud569 ud14cuc2a4ud2b8

## 적용 방법
1. ESP32_SmartFarm 폴더의 코드를 ESP32에 업로드
2. Arduino_Relay 폴더의 코드를 Arduino에 업로드
3. I2C 연결 확인 (ESP32: GPIO21/22, Arduino: A4/A5)
4. 릴레이 연결 확인 (Arduino: D2/D3/D4)
5. SD 카드에 touch.wav 파일 업로드

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



# 백업 정보
- 날짜: 2025-03-26
- 버전: 1.1.0
- 내용: 블루투스 A2DP 싱크 기능 및 오디오 시스템 구현 완료


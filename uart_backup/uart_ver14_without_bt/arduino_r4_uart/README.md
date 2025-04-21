# Arduino R4 UART 통신 코드 변경 이력

## 펌웨어 버전: v1.0.2 (2025-04-02)

### 변경 사항
1. **자동/수동 모드 통신 버그 수정**
   - 자동 모드 버튼을 눌렀을 때 ESP32로 모드 변경 정보를 전송하는 코드 추가
   - 모드 변경 시 `Serial1.print(MODE_PREFIX)`와 `Serial1.println(autoModeEnabled ? "AUTO" : "MANUAL")` 호출
   - 이전에는 로컬 변수만 변경하고 ESP32에 알리지 않아 UI가 업데이트되지 않았음

### 최적화 효과
- ESP32 UI에서 자동/수동 모드가 올바르게 표시됨
- 사용자가 버튼을 눌렀을 때 즉시 피드백 제공
- 아두이노와 ESP32 간 모드 동기화 개선

## 펌웨어 버전: v1.0.1 (2025-04-01)

### 변경 사항
1. **센서 관련 코드 제거**
   - 아두이노에는 실제로 센서가 연결되어 있지 않음에도 불구하고 센서 값을 읽고 전송하는 코드가 있었음
   - 센서 핀 정의(TEMP_SENSOR_PIN, HUMID_SENSOR_PIN, DISTANCE_SENSOR_PIN) 제거
   - 더미 센서 값 생성 함수(readTemperature, readHumidity, readDistance) 제거
   - 센서 데이터 전송 함수(sendTemperature, sendHumidity, sendDistance) 제거

2. **통신 구조 개선**
   - 센서 데이터 요청 명령(CMD_GET_TEMP, CMD_GET_HUMIDITY, CMD_GET_DISTANCE)에 대해 오류 메시지 반환
   - 릴레이 제어 명령 실행 후 ACK 메시지 명시적 전송 추가
   - 메인 루프에서 센서 데이터 정기 전송 기능 제거
   - UART 수신 코드 개선 (한 글자씩 읽어 명령 완성 후 처리)

3. **코드 정리**
   - 불필요한 지연(delay) 제거
   - 릴레이 상태 전송 코드 간소화
   - 자동 모드 상태도 함께 전송하도록 변경

### 최적화 효과
- 불필요한 더미 데이터 전송으로 인한 통신 부하 제거
- ESP32와 Arduino 간 통신 충돌 가능성 감소
- 실제 하드웨어 구성에 맞는 코드로 정리

### 기술 스택
- Arduino R4 보드 사용
- UART 통신 (Serial1, 핀 0, 1 사용)
- 릴레이 제어 (팬, LED, 가습기)
- 버튼 입력 처리 (INPUT_PULLUP 모드)

#include <Wire.h>

void setup() {
  Wire.begin(21, 22); // ESP32의 SDA(GPIO21), SCL(GPIO22)
  Serial.begin(115200);
  while (!Serial) {}
  
  Serial.println("\nI2C 버스 스캐너");
}

void loop() {
  byte error, address;
  int devices = 0;

  Serial.println("스캐닝 중...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C 장치 발견: 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      
      // 알려진 I2C 장치 식별
      if (address == 0x29) {
        Serial.print(" (VL53L0X 거리 센서)");
      } else if (address == 0x44) {
        Serial.print(" (SHT31 온습도 센서)");
      } else if (address == 0x57) {
        Serial.print(" (MAX30102 심박/산소포화도 센서)");
      }
      
      Serial.println();
      devices++;
    } else if (error == 4) {
      Serial.print("알 수 없는 오류(0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(")");
    }
  }
  
  if (devices == 0) {
    Serial.println("I2C 장치를 찾을 수 없음\n");
  } else {
    Serial.print("발견된 장치 수: ");
    Serial.println(devices);
  }
  
  delay(5000); // 5초마다 반복
}

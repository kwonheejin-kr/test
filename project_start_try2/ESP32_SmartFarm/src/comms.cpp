#include "comms.h"
#include "sensors.h"

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* mqttServer = "YOUR_MQTT_BROKER";
const int mqttPort = 1883;

WiFiClient espClient;
PubSubClient client(espClient);
BluetoothSerial SerialBT;

void init_comms() {
    SerialBT.begin("ESP32_SmartFarm");

    WiFi.begin(ssid, password);
    unsigned long startAttemptTime = millis();
    
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
        delay(500);
        Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi 연결 성공");
    } else {
        Serial.println("WiFi 연결 실패 (타임아웃)");
    }

    client.setServer(mqttServer, mqttPort);
    mqtt_connect();
}

void reconnect_wifi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 연결 끊어짐. 재연결 중...");
        WiFi.reconnect();
        
        unsigned long startAttemptTime = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi 재연결 성공");
        } else {
            Serial.println("WiFi 재연결 실패 (타임아웃)");
        }
    }
}

void mqtt_connect() {
    unsigned long startTime = millis();
    while (!client.connected() && millis() - startTime < 10000) {
        Serial.print("MQTT 연결 중...");
        if (client.connect("ESP32_Client")) {
            Serial.println("MQTT 연결 성공!");
            return;
        } else {
            Serial.print("실패, 상태 코드=");
            Serial.println(client.state());
            delay(2000);
        }
    }
    Serial.println("MQTT 연결 실패 (타임아웃), 재시도 예약됨");
}

void check_mqtt_connection() {
    if (!client.connected()) {
        Serial.println("MQTT 연결 끊어짐. 재연결 시도...");
        mqtt_connect();
    }
    client.loop();
}

void send_mqtt_message(const char* topic, const char* message) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi 연결 문제로 전송 실패. WiFi 재연결 시도...");
        reconnect_wifi();
    }

    if (!client.connected()) {
        Serial.println("MQTT 연결 문제로 전송 실패. MQTT 재연결 시도...");
        mqtt_connect();
    }

    int retry_count = 0;
    while (retry_count < 3) {
        if (client.publish(topic, message)) {
            Serial.println("MQTT 메시지 전송 성공");
            return;
        } else {
            Serial.println("MQTT 메시지 전송 실패, 재시도 중...");
            retry_count++;
            delay(2000);
        }
    }
    Serial.println("MQTT 메시지 전송 실패 (최대 재시도 횟수 초과)");
}

#include "comms.h"

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
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("WiFi 연결 성공");

    client.setServer(mqttServer, mqttPort);
    mqtt_connect();
}

void mqtt_connect() {
    while (!client.connected()) {
        Serial.print("MQTT 연결 중...");
        if (client.connect("ESP32_Client")) {
            Serial.println("MQTT 연결 성공!");
        } else {
            Serial.print("실패, 상태 코드=");
            Serial.println(client.state());
            delay(2000);
        }
    }
}

void send_mqtt_message(const char* topic, const char* message) {
    client.publish(topic, message);
}

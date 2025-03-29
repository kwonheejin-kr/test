#ifndef COMMS_H
#define COMMS_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <BluetoothSerial.h>

// WiFi 설정
extern const char* ssid;
extern const char* password;
extern WiFiClient espClient;
extern PubSubClient client;

// Bluetooth 설정
extern BluetoothSerial SerialBT;

// 통신 초기화 함수
void init_comms();
void mqtt_connect();
void send_mqtt_message(const char* topic, const char* message);

#endif

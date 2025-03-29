#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BluetoothSerial.h>
#include <BluetoothA2DPSink.h>
#include <State.h>
#include <Config.h>
#include "driver/i2s.h"  // I2S 관련 정의를 위해 추가

class CommunicationManager {
public:
    CommunicationManager();
    bool init(const char* wifi_ssid, const char* wifi_password,
              const char* mqtt_server, int mqtt_port);
    bool connect_wifi();
    bool connect_mqtt();
    bool send_mqtt_message(const char* topic, const char* message);
    void handle_mqtt_callback(char* topic, byte* payload, unsigned int length);
    void check_connections();
    void process();
    bool initBluetooth();
    bool isBluetoothConnected();
    void handleBluetooth();

private:
    static const uint32_t WIFI_TIMEOUT = 10000;
    static const uint32_t MQTT_TIMEOUT = 5000;
    static const uint8_t MAX_RETRY_COUNT = 3;

    WiFiClient wifi_client;
    PubSubClient mqtt_client;
    BluetoothSerial bt_serial;
    BluetoothA2DPSink a2dp_sink;

    const char* wifi_ssid;
    const char* wifi_password;
    const char* mqtt_server;
    int mqtt_port;

    bool wifi_connected;
    bool mqtt_connected;
    bool bt_audio_connected;
    unsigned long last_reconnect_attempt;

    bool reconnect_wifi();
    bool reconnect_mqtt();
};

extern CommunicationManager commManager;

#endif

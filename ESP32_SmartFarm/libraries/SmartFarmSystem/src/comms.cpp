#include "comms.h"
#include "../sensors/sensors.h"
#include "Config.h"
#include "State.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <BluetoothSerial.h>

CommunicationManager commManager;

CommunicationManager::CommunicationManager() 
    : wifi_connected(false), mqtt_connected(false), last_reconnect_attempt(0) {
    mqtt_client.setClient(wifi_client);
}

bool CommunicationManager::init(const char* ssid, const char* password,
                              const char* mqtt_srv, int mqtt_prt) {
    wifi_ssid = ssid;
    wifi_password = password;
    mqtt_server = mqtt_srv;
    mqtt_port = mqtt_prt;

    bt_serial.begin("ESP32_SmartFarm");
    
    bool wifi_success = connect_wifi();
    if (wifi_success) {
        mqtt_client.setServer(mqtt_server, mqtt_port);
        mqtt_client.setCallback([this](char* topic, byte* payload, unsigned int length) {
            this->handle_mqtt_callback(topic, payload, length);
        });
        return connect_mqtt();
    }
    return false;
}

bool CommunicationManager::connect_wifi() {
    if (WiFi.status() == WL_CONNECTED) {
        wifi_connected = true;
        return true;
    }

    Serial.print("WiFi 연결 중...");
    WiFi.begin(wifi_ssid, wifi_password);

    unsigned long start_attempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start_attempt < WIFI_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();

    wifi_connected = (WiFi.status() == WL_CONNECTED);
    if (wifi_connected) {
        Serial.println("WiFi 연결됨");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("WiFi 연결 실패");
    }

    return wifi_connected;
}

bool CommunicationManager::connect_mqtt() {
    if (!wifi_connected) return false;

    if (mqtt_client.connected()) {
        mqtt_connected = true;
        return true;
    }

    Serial.print("MQTT 브로커 연결 중...");
    
    String client_id = "ESP32Client-";
    client_id += String(random(0xffff), HEX);

    if (mqtt_client.connect(client_id.c_str())) {
        Serial.println("연결됨");
        mqtt_client.subscribe("farm/control/#");
        mqtt_connected = true;
        return true;
    }

    Serial.print("실패, rc=");
    Serial.println(mqtt_client.state());
    mqtt_connected = false;
    return false;
}

bool CommunicationManager::send_mqtt_message(const char* topic, const char* message) {
    if (!wifi_connected || !mqtt_connected) {
        check_connections();
    }

    if (!mqtt_client.connected()) {
        return false;
    }

    return mqtt_client.publish(topic, message);
}

void CommunicationManager::handle_mqtt_callback(char* topic, byte* payload, unsigned int length) {
    char message[length + 1];
    memcpy(message, payload, length);
    message[length] = '\0';

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.println(message);

    // 여기에 메시지 처리 로직 추가
}

void CommunicationManager::check_connections() {
    if (WiFi.status() != WL_CONNECTED) {
        wifi_connected = false;
        reconnect_wifi();
    }

    if (!mqtt_client.connected()) {
        mqtt_connected = false;
        reconnect_mqtt();
    }
}

bool CommunicationManager::reconnect_wifi() {
    if (millis() - last_reconnect_attempt < WIFI_TIMEOUT) {
        return false;
    }

    last_reconnect_attempt = millis();
    return connect_wifi();
}

bool CommunicationManager::reconnect_mqtt() {
    if (millis() - last_reconnect_attempt < MQTT_TIMEOUT) {
        return false;
    }

    last_reconnect_attempt = millis();
    return connect_mqtt();
}

void CommunicationManager::process() {
    if (mqtt_connected) {
        mqtt_client.loop();
    }
}

bool CommunicationManager::initBluetooth() {
    // Bluetooth 장치 이름으로 시작
    a2dp_sink.start(Config::BT_DEVICE_NAME);
    
    // I2S 설정
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 64,
        .use_apll = false,
        .tx_desc_auto_clear = true,
        .fixed_mclk = 0
    };
    
    // I2S 핀 설정
    i2s_pin_config_t pin_config = {
        .bck_io_num = Config::I2S_BCLK,
        .ws_io_num = Config::I2S_LRC,
        .data_out_num = Config::I2S_DOUT,
        .data_in_num = I2S_PIN_NO_CHANGE
    };
    
    // I2S 드라이버 설치
    esp_err_t err = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("I2S 드라이버 설치 실패");
        return false;
    }
    
    // I2S 핀 설정 적용
    err = i2s_set_pin(I2S_NUM_0, &pin_config);
    if (err != ESP_OK) {
        Serial.println("I2S 핀 설정 실패");
        return false;
    }
    
    return true;
}

bool CommunicationManager::isBluetoothConnected() {
    return a2dp_sink.is_connected();
}

void CommunicationManager::handleBluetooth() {
    // 필요한 경우 여기에 추가 처리 로직
}

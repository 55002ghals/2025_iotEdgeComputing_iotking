/*
 * [AWS 통신 테스트용]
 * 센서/AI 없이 30초마다 가짜(Random) 데이터 전송
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>            
#include "arduino_secrets.h"

WiFiClientSecure net;
MQTTClient client(1024);

// 테스트용 타이머 변수
unsigned long lastTestTime = 0;
const unsigned long TEST_INTERVAL = 30000; // 30초 (ms 단위)

// 랜덤 상태 후보
String states[] = {"LOW", "MID", "HIGH"};

void setup() {
    Serial.begin(115200);
    
    // 랜덤 시드 초기화 
    randomSeed(analogRead(0));

    // 1. 와이파이 연결
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\nConnected!");

    // 2. AWS 보안 설정
    net.setCACert(AWS_CERT_CA);
    net.setCertificate(AWS_CERT_CRT);
    net.setPrivateKey(AWS_CERT_PRIVATE);

    // 3. MQTT 연결
    client.begin(MQTT_BROKER, 8883, net);
    connectAWS();
}

void loop() {
    client.loop(); 

    if (!client.connected()) {
        connectAWS();
    }

    // --- 30초마다 실행되는 테스트 로직 ---
    if (millis() - lastTestTime > TEST_INTERVAL) {
        lastTestTime = millis(); // 타이머 갱신

        // 1. 가짜 데이터 생성
        int randIndex = random(0, 3); 
        String randomState = states[randIndex];
        
        // 신뢰도 랜덤 (50% ~ 99%)
        float randomConf = random(50, 100) / 100.0; 

        Serial.println("\n-----------------------------");
        Serial.println("[TEST MODE] Generating Fake Data...");
        Serial.print("State: "); Serial.println(randomState);
        Serial.print("Conf : "); Serial.println(randomConf);

        // 2. AWS로 전송 
        sendToAWS(randomState, randomConf);
    }
}

// AWS 전송 함수
void sendToAWS(String densityState, float confidence) {
    StaticJsonDocument<200> doc;
    doc["device_id"] = DEVICE_ID;
    doc["density"] = densityState;     
    doc["confidence"] = confidence;    
    doc["duration"] = 30; 

    String jsonString;
    serializeJson(doc, jsonString);

    Serial.print("Sending MQTT Payload: ");
    Serial.println(jsonString);

    client.publish(MQTT_TOPIC, jsonString);
}

// AWS 재연결 함수
void connectAWS() {
    Serial.print("Connecting to AWS IoT");
    while (!client.connect(DEVICE_ID)) {
        Serial.print(".");
        delay(1000);
    }
    Serial.println("\nAWS IoT Connected!");
}
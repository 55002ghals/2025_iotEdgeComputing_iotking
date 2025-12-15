// ==========================================
// Arduino UNO R4 LCD Alert
// ==========================================

#include <WiFiS3.h>            
#include <PubSubClient.h>      
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

#include "arduino_secrets.h" 

// WiFi 설정
const char* WIFI_SSID = SECRET_SSID;
const char* WIFI_PASSWORD = SECRET_PASS;

// MQTT 브로커 설정
IPAddress MQTT_SERVER(SECRET_MQTT_IP);  
const int   MQTT_PORT       = SECRET_MQTT_PORT;
const char* MQTT_CLIENT_ID  = MQTT_LCD_ID;
const char* MQTT_TOPIC      = MQTT_TOPIC_STRING;    

// I2C LCD 설정
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BUZZER_PIN = 8;  

WiFiClient net;              
PubSubClient mqttClient(net);

void playAlarmBuzzer() {
  for (int i = 0; i < 3; i++) {   
    tone(BUZZER_PIN, 2000);       
    delay(300);
    tone(BUZZER_PIN, 1200);     
    delay(300);
  }
  noTone(BUZZER_PIN);          
}


// LCD 알림 함수
void showTriggeredLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("!!TRIGGERED!!");
  lcd.setCursor(0, 1);
  lcd.print("density: HIGH");

  playAlarmBuzzer();  

  delay(5000); 
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Waiting MQTT...");
}


// MQTT 콜백
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  String jsonStr;
  for (unsigned int i = 0; i < length; i++) {
    jsonStr += (char)payload[i];
  }
  Serial.println(jsonStr);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, jsonStr);

  if (error) {
    Serial.print("JSON Error: ");
    Serial.println(error.c_str());
    return;
  }

  const char* density = doc["density"];
  
  if (density && String(density) == "HIGH") {
    Serial.println("WARNING: People density is too high!");
    showTriggeredLCD();
  } else {
    Serial.println("Status: Crowd level is normal");
  }
}

// WiFi 연결
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// MQTT 연결
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection... ");
    
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("CONNECTED!");
      mqttClient.subscribe(MQTT_TOPIC);
      Serial.print("Subscribed to: ");
      Serial.println(MQTT_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(); 
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting R4...");

  connectWiFi();
  
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);

  connectMQTT();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  Serial.println("Status: Crowd level is normal");

  pinMode(BUZZER_PIN, OUTPUT);

}

void loop() {
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();
}
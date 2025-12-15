/*************************************************
 * ESP32 + Edge Impulse Inference 
 * Features:
 *  - fsr_mean
 *  - distance_mean
 *  - pir_count
 *  - sound_rms
 *************************************************/

#include <Arduino.h>  
#include <math.h>
#include <population_density_classifier.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"

// AWS 
WiFiClientSecure awsNet;
MQTTClient awsClient(1024);

// Local MQTT
WiFiClient localNet;
MQTTClient localClient(512);
IPAddress LOCAL_MQTT_IP(SECRET_MQTT_IP);

// 핀
const int PIN_TRIG  = 12;
const int PIN_ECHO  = 14;
const int PIN_PIR   = 27;
const int PIN_SOUND = 34;
const int PIN_FSR   = 39;

// 샘플링
#define FS_HZ       10
#define WINDOW_SEC  10
#define N_SAMPLES   (FS_HZ * WINDOW_SEC)

float fsr_buf[N_SAMPLES];
float dist_buf[N_SAMPLES];
float sound_buf[N_SAMPLES];
float pir_buf[N_SAMPLES];
int idx = 0;

float feature_vec[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

int get_signal_data(size_t offset, size_t length, float *out_ptr) {
  memcpy(out_ptr, feature_vec + offset, length * sizeof(float));
  return 0;
}

float mean(const float *x) {
  float s = 0;
  for (int i = 0; i < N_SAMPLES; i++) s += x[i];
  return s / N_SAMPLES;
}

float rms(const float *x) {
  float s = 0;
  for (int i = 0; i < N_SAMPLES; i++) s += x[i] * x[i];
  return sqrt(s / N_SAMPLES);
}

float read_distance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  long d = pulseIn(PIN_ECHO, HIGH, 30000);
  float dist = d * 0.034 / 2;
  if (dist < 2 || dist > 400) dist = 0;
  return dist;
}

// MQTT 전송
void sendBoth(String label, float conf, float fsr, float dist, float sound, int pir) {
  StaticJsonDocument<200> doc;
  doc["device_id"] = DEVICE_ID;
  doc["density"] = label;
  doc["confidence"] = conf;
  doc["fsr_mean"] = fsr;
  doc["dist_mean"] = dist;
  doc["sound_rms"] = sound;
  doc["pir_count"] = pir;
  doc["window_sec"] = WINDOW_SEC;

  String payload;
  serializeJson(doc, payload);

  awsClient.publish(MQTT_TOPIC, payload);
  localClient.publish(MQTT_TOPIC, payload);
}

// SETUP
void setup() {
  Serial.begin(115200);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) delay(300);

  awsNet.setCACert(AWS_CERT_CA);
  awsNet.setCertificate(AWS_CERT_CRT);
  awsNet.setPrivateKey(AWS_CERT_PRIVATE);

  awsClient.begin(MQTT_BROKER, 8883, awsNet);
  awsClient.connect(DEVICE_ID);

  localClient.begin(LOCAL_MQTT_IP, SECRET_MQTT_PORT, localNet);
  localClient.connect(DEVICE_ID);

  Serial.println("READY");
}

void loop() {
  awsClient.loop();
  localClient.loop();

  fsr_buf[idx]   = analogRead(PIN_FSR);
  dist_buf[idx]  = read_distance();
  sound_buf[idx] = analogRead(PIN_SOUND);
  pir_buf[idx]   = digitalRead(PIN_PIR);

  idx++;

  if (idx >= N_SAMPLES) {
    idx = 0;

    float fsr_mean = mean(fsr_buf);
    float dist_mean = mean(dist_buf);
    float sound_rms = rms(sound_buf);

    int pir_count = 0;
    for (int i = 0; i < N_SAMPLES; i++) if (pir_buf[i] > 0.5) pir_count++;

    feature_vec[0] = fsr_mean;
    feature_vec[1] = dist_mean;
    feature_vec[2] = pir_count;
    feature_vec[3] = sound_rms;

    signal_t signal;
    signal.total_length = 4;
    signal.get_data = get_signal_data;

    ei_impulse_result_t result;
    run_classifier(&signal, &result, false);

    Serial.println("---- FEATURE ----");
    Serial.printf("fsr_mean: %.2f\n", fsr_mean);
    Serial.printf("distance_mean: %.2f\n", dist_mean);
    Serial.printf("pir_count: %d\n", pir_count);
    Serial.printf("sound_rms: %.2f\n", sound_rms);

    for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
      Serial.print(result.classification[i].label);
      Serial.print(": ");
      Serial.println(result.classification[i].value, 4);

      if (strcmp(result.classification[i].label, "HIGH") == 0 &&
          result.classification[i].value > 0.6) {
        sendBoth("HIGH", result.classification[i].value, fsr_mean, dist_mean, sound_rms, pir_count);
      }
    }
  }

  delay(100);
}

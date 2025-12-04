// 이 파일 이름을 'arduino_secrets.h'로 바꾸고 값을 채우세요.
#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H
#define SECRET_SSID "YOUR_WIFI_SSID"
#define SECRET_PASS "YOUR_WIFI_PASSWORD"

// MQTT 서버 정보
// MQTT Broker IP (쉼표로 구분, 예: 192,168,0,1)
#define SECRET_MQTT_IP 127, 0, 0, 1 
#define SECRET_MQTT_PORT 1883
#define MQTT_TOPIC "YOUR_TOPIC"
#define MQTT_LCD_ID "DEVICE_ID_1"
#define MQTT_PUBLISH_ID "DEVICE_ID_2"

// AWS 인증서 (AmazonRootCA1.pem)
const char AWS_CERT_CA[] = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

// 디바이스 인증서 (xxxx-certificate.pem.crt)
const char AWS_CERT_CRT[] = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";

// 프라이빗 키 (xxxx-private.pem.key)
const char AWS_CERT_PRIVATE[] = R"EOF(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
)EOF";

#endif

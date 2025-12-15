# IoT 기반 다중 센서 융합 인구 밀집도 모니터링 및 경보 시스템
### (IoT-based Crowd Density Monitoring System using Multi-Sensor Fusion)

## 1. 프로젝트 개요
본 프로젝트는 공공장소 및 특정 구역의 인구 밀집도를 실시간으로 파악하여 안전사고를 예방하기 위해 설계되었습니다. 
고가의 장비 대신 4종의 저비용 센서(PIR, FSR, 초음파, 사운드)를 융합하여 데이터를 수집하고, Edge Device(ESP32) 내 경량 AI 모델을 통해 밀집도(HIGH, MED, LOW)를 판별합니다.

위험 수준(HIGH)이 지속적으로 감지될 경우 AWS IoT Core 및 Node-RED 대시보드와 연동하여 관리자에게 알림을 전송하며, 현장 LCD를 통해 즉각적인 경고 메시지를 송출합니다.

## 2. 시스템 아키텍처
전체 시스템은 **데이터 수집/판단(Edge)**, **클라우드 관제(Cloud)**, **현장 피드백(Local Feedback)**의 3단계로 구성됩니다.

1.  **Sensing & Inference (Edge):** 4개의 센서 데이터를 수집 후 AI 모델을 통해 밀집도 분류.
2.  **Logic & Transmission:** 'HIGH' 등급이 5회 연속 감지될 경우, MQTT 프로토콜을 통해 AWS IoT Core로 데이터 전송.
3.  **Monitoring (Cloud):** Node-RED 서버가 데이터를 수신하여 공공기관 관리용 대시보드 시각화 및 통제 인력 배치 알림 생성.
4.  **Immediate Feedback (Local):** 위험 상황 발생 시, 현장 디스플레이(LCD)에 즉각적인 경고 메시지 표출.

## 3. 하드웨어 구성
* **Main Board:** Wemos D1 R32(ESP32)
* **Sensors:**
    * PIR Sensor (움직임 감지)
    * FSR Sensor (압력 감지 - 바닥 매트 등 활용)
    * Ultrasonic Sensor (거리/공간 점유 감지)
    * Sound Sensor (소음 레벨 감지)
* **Actuators:** I2C LCD (16x2) for Local Alert

## 4. 소스 코드 구성

본 리포지토리는 다음과 같은 파일들로 구성되어 있습니다.

### 📂 Main Logic
* **`population_classifier.ino`**
    * **기능:** 현장 데이터 수집 및 AI 모델 추론 수행, 클라우드 전송 담당.
    * **로직:** 센서 데이터 분석 후 `HIGH`로 판별될 경우 AWS IoT Core로 MQTT 메시지 발행.
    
* **`mqtt_lcd_response.ino`**
    * **기능:** 현장 상황 즉각적 전파를 위한 시각적 피드백 담당.
    * **로직:** AWS IoT Core 또는 로컬 브로커로부터 위험 신호를 수신(Subscribe)하여 LCD에 경고 문구("WARNING") 표출 및 소리 알림.

### 📂 Data Acquisition
* **`data_collect.ino`**
    * **기능:** AI 학습 모델 생성을 위한 초기 데이터 수집용 코드.
    * **로직:** 센서의 Raw Data를 시리얼로 로깅. csv로 추출 후 Edge Impulse 업로드.

### 📂 Configuration & Security
* **`arduino_secrets_template.h`**
    * 보안 설정을 위한 템플릿 파일입니다. (하단 설치 방법 참조)
* **`arduino_secrets.h`**
    * 실제 WiFi 정보 및 API Key가 포함된 파일로, 보안상 **Git에 업로드되지 않습니다.** (.gitignore 처리됨)

## 5. 설치 및 실행 방법

### 보안 설정 (필수)
본 프로젝트는 보안을 위해 민감한 정보를 소스 코드에 직접 포함하지 않습니다.

1. `arduino_secrets_template.h` 파일을 다운로드합니다.
2. 파일명을 **`arduino_secrets.h`**로 변경합니다.
3. 해당 파일 내에 본인의 네트워크 및 서버 정보등을 입력합니다.
    ```cpp
    #define SECRET_SSID "YOUR_WIFI_SSID"
    #define SECRET_PASS "YOUR_WIFI_PASSWORD"
    #define SECRET_MQTT_IP 127, 0, 0, 1  // AWS IoT or Broker IP
    ...
    ```


## 6. 관제 시스템 (Node-RED Dashboard)
* **역할:** 공공기관 및 안전 관리자용 모니터링 도구.
* **기능:**
    * 실시간 인구 밀집도 그래프 시각화.
    * 임계치 초과 시 경보 알림(Alert).
    * 위험 구역 통제 인력 배치 지시.

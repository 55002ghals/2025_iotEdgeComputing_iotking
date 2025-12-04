// Wemos D1 R32 (ESP32) Data Collection Code
// 1. 핀 정의 
const int PIN_TRIG = 12;
const int PIN_ECHO = 14;
const int PIN_PIR  = 27;
const int PIN_SOUND = 34; // Analog
const int PIN_FSR   = 39; // Analog (압력)

void setup() {
  Serial.begin(115200);

  // 핀 모드 설정
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);
  
  delay(2000);
  Serial.println("FSR,Distance,PIR,Sound"); // 헤더 
}

void loop() {
  // --- 1. 초음파 거리 측정 ---
  long duration;
  float distance;
  
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  duration = pulseIn(PIN_ECHO, HIGH);
  distance = duration * 0.034 / 2; // cm 단위 변환
  
  if (distance > 400 || distance < 2) distance = 0; // 이상치 처리

  // --- 2. 나머지 센서 읽기 ---
  int pirVal = digitalRead(PIN_PIR);      // 0 or 1
  int soundVal = analogRead(PIN_SOUND);   // 0 ~ 4095
  int fsrVal = analogRead(PIN_FSR);       // 0 ~ 4095

  // --- 3. CSV 포맷으로 출력 ---
  // 순서: 압력, 거리, PIR, 소리
  Serial.print(fsrVal);
  Serial.print(",");
  Serial.print(distance);
  Serial.print(",");
  Serial.print(pirVal);
  Serial.print(",");
  Serial.println(soundVal);

  // --- 4. 샘플링 주기 ---
  delay(100); 
}
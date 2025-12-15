// ==========================================
// ESP32 Data Collection
// ==========================================

// 핀 
const int PIN_TRIG  = 12;
const int PIN_ECHO  = 14;
const int PIN_PIR   = 27;
const int PIN_SOUND = 34;
const int PIN_FSR   = 39;

// 샘플링 설정
#define FS_HZ        10
#define WINDOW_SEC  10
#define N_SAMPLES   (FS_HZ * WINDOW_SEC)

float fsr_buf[N_SAMPLES];
float dist_buf[N_SAMPLES];
float sound_buf[N_SAMPLES];
float pir_buf[N_SAMPLES];
int idx = 0;

// 통계 함수
float mean(float *x) {
  float s = 0;
  for (int i = 0; i < N_SAMPLES; i++) s += x[i];
  return s / N_SAMPLES;
}


float rms(float *x) {
  float s = 0;
  for (int i = 0; i < N_SAMPLES; i++) s += x[i] * x[i];
  return sqrt(s / N_SAMPLES);
}


float pir_count() {
  int c = 0;
  for (int i = 0; i < N_SAMPLES; i++) if (pir_buf[i] > 0.5) c++;
  return c;
}



// 초음파
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

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);

  delay(2000);

  // CSV 헤더 
  Serial.println("fsr_mean,distance_mean,pir_count,sound_rms,label");
}

void loop() {
  fsr_buf[idx]   = analogRead(PIN_FSR);
  dist_buf[idx]  = read_distance();
  sound_buf[idx] = analogRead(PIN_SOUND);
  pir_buf[idx]   = digitalRead(PIN_PIR);

  idx++;

  if (idx >= N_SAMPLES) {
    idx = 0;

    // Feature 계산
    float fsr_m = mean(fsr_buf);
    float dist_m = mean(dist_buf);
    float snd_r = rms(sound_buf);
    float pir_c = pir_count();

    // CSV 출력
    Serial.print(fsr_m); Serial.print(",");
    Serial.print(dist_m); Serial.print(",");
    Serial.print(pir_c); Serial.print(",");
    Serial.print(snd_r); Serial.print(",");

    Serial.println("HIGH"); 
  }

  delay(100); // 10 Hz
}

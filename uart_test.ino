// 1초에 한 번씩 'A'에서 'Z'까지를 를 보냅니다.
// 선루프 상태에 따라서 보내는 데이터의 예시 
// millis를 이용하면 delay와 다르게 자원을 점유하지 않는다

char msg = 'A';
unsigned long time_previous, time_current;

void setup() {
  time_previous = millis();
  Serial.begin(9600, SERIAL_8N1); // 실전
}

void loop() {
  time_current = millis();
  if (time_current >= time_previous + 1000) {
    time_previous = time_current;
    Serial.write(msg);
    msg++;
    if (msg > 'Z') msg = 'A';
  }
}

void serialEvent(){
  char echo = Serial.read();
  Serial.write(echo);
}

// 이 코드가 하는 일 : 글자를 받으면 에코하고 1초에 한 번 'A'를 보낸다.

char msg;
unsigned long time_previous, time_current;

void setup() {
  time_previous = millis();
  Serial.begin(9600, SERIAL_8N1); // 실전
  // Serial.begin(9600); // PC
  //Serial.begin(9600, SERIAL_8E1); // 예시
  //Serial.begin(speed, config);
  //config는 생략될 경우 Data 8bit, Parity 없음, Stop 1bit
  // SERIAL_8E1 : 8비트, Even Parity, 정지비트수 1
  // 비트 수는 5~8, 패리티는 O(odd), E(even), N(non), 정지비트 수는 1~2.
  // 스타트비트는 무조건 하나라서 설정하지 않음.
}

void loop() {
  // 1초에 한 번씩 'A'를 보냅니다.
  // 선루프 상태에 따라서 보내는 데이터의 예시 
  // millis를 이용하면 delay와 다르게 자원을 점유하지 않는다
  time_current = millis();
  if (time_current >= time_previous + 1000) {
    time_previous = time_current;
    msg = 'A';
    Serial.write(msg);
  }
}

// 메세지가 들어와야 움직입니다
void serialEvent(){
  msg = Serial.read();
  Serial.write(msg);
}

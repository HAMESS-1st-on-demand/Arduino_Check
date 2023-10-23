void setup() {
  // Serial.begin(9600, SERIAL_8N1);
  Serial.begin(9600);
  //Serial.begin(9600, SERIAL_8E1);
  //Serial.begin(speed, config);
  //config는 생략될 경우 Data 8bit, Parity 없음, Stop 1bit
  // SERIAL_8E1 : 8비트, Even Parity, 정지비트수 1
  // 비트 수는 5~8, 패리티는 O(odd), E(even), N(non), 정지비트 수는 1~2.
  // 스타트비트는 무조건 하나라서 설정하지 않음.
}

void loop() {
  
}

void serialEvent(){
  Serial.print(Serial.readString());
}

#include <Servo.h>

Servo myServo;
//button핀은 2번으로 설정, 우노의 경우 인터럽트를 사용하기 위해서는 핀 2, 3에 연결해야 각각 인터럽트 0, 1로 매핑됨
const byte button = 2;

//서보모터 제어 각도
volatile int angle = 0;


void setup() {
  //버튼은 2번핀에 연결되어 있고 input모드, pull_up모드로 설정
  pinMode(button, INPUT_PULLUP);

  //서보모터는 8번 핀에 연결된 상태
  myServo.attach(8);
  
  //digitalPinToInterrupt()를 통해 핀과 인터럽트 번호 매핑
  //인터럽트가 걸리면 pauseMotor() 함수가 실행됨
  //LOW 모드로 설정하여 전압이 0일때 실행
  //우노는 HIGH모드가 안된다고 본거같음...
  attachInterrupt(digitalPinToInterrupt(button), pauseMotor, LOW);
}

//인터럽트가 발생하면 실행되는 함수
//angle이 전역변수이기 때문에 모터를 angle각 만큼 유지
void pauseMotor(){
  myServo.write(angle);
}

void loop() {
  //0 ~ 180도 만큼 서보모터 회전
  for (angle = 0; angle <= 180; angle++) {
    myServo.write(angle);
    delay(10);
  }

  //180 ~ 0도 만큼 서보모터 회전
  for (angle = 180; angle >= 0; angle--) {
    myServo.write(angle);
    delay(10);
  }
}

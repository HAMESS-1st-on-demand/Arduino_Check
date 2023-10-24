#include<Servo.h>

Servo myServo;

//썬루프가 완전히 닫혔을 때 눌리는 버튼
const byte closed_btn = 12;

//썬루프가 완전히 열렸을 때 눌리는 버튼
const byte opened_btn = 13;

//닫힘 버튼
const byte close = 10;

//열림 버튼
const byte open = 11;

//손끼임 측정 버튼
const byte obstacle_btn = 2;

//썬루프 방향, 1이면 열림 방향, 0이면 닫힘 방향
volatile int direction = 1;

//손끼임 감지 상태, 1이면 감지
volatile int isCollision = 0;

void setup() {
  //9번핀에 서보모터 장착
  myServo.attach(9);

  //버튼들 핀 설정, 풀업저항이 걸려있음
  pinMode(closed_btn, INPUT_PULLUP);
  pinMode(opened_btn, INPUT_PULLUP);
  pinMode(close, INPUT_PULLUP);
  pinMode(open, INPUT_PULLUP);
  pinMode(obstacle_btn, INPUT_PULLUP);

  //인터럽트 설정, 손끼임 방지버튼을 인터럽트로 처리
  //손끼임 감지버튼이 HIGH -> LOW 되면 호출
  attachInterrupt(digitalPinToInterrupt(obstacle_btn), gotObstacle, FALLING);
}

//인터럽트가 발생하면 실행되는 함수
void gotObstacle(){
  //서보모터를 중지
  myServo.write(90);

  //손끼임 상태를 1로 바꿈
  isCollision = 1;
}

void loop() {
  //손끼임이 발생했다면
  if(isCollision == 1){

    //선루프가 완전히 닫힐때까지(opened_btn이 눌릴때까지)
    //열리는 방향으로 선루프 제어
    while(digitalRead(opened_btn) == HIGH){
      direction = 1;
      myServo.write(90 + (40 * direction));
    }

    //선루프가 완전히 개방되면 손끼임 상태를 0으로 바꿈
    isCollision = 0;
  }

  //손끼임이 발생하지 않았다면
  else{
    //사용자가 열림버튼을 눌렀다면
    if(digitalRead(open) == LOW){
      //선루프가 완전히 열려있지 않다면
      if(digitalRead(opened_btn) == HIGH){
        //열림방향으로 선루프를 오픈
        direction = 1;
        myServo.write(90 + (40 * direction));
      }

      //선루프가 완전히 열려있다면
      else{
        //모터 정지
        myServo.write(90);
      }
    }

    //사용자가 닫힘버튼을 눌렀다면
    if(digitalRead(close) == LOW){
      //선루프가 완전히 닫혀있지 않다면
      if(digitalRead(closed_btn) == HIGH){
        //닫힘방향으로 선루프를 닫음
        direction = -1;
        myServo.write(90 + (40 * direction));
      }

      //선루프가 완전히 닫혀있다면
      else{
        //모터 정지
        myServo.write(90);
      }
    }

    //사용자가 아무버튼도 누르지 않는다면 모터 정지
    else if(digitalRead(open) == HIGH && digitalRead(close) == HIGH){
      myServo.write(90);
    }
  }
}

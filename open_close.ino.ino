#include<Servo.h>

Servo myServo;
const byte closed_btn = 2;
const byte opened_btn = 3;
const byte close = 6;
const byte open = 7;
volatile int direction;

//0 == closed, 1 == opened
volatile int state;

void setup() {
  myServo.attach(9);
  pinMode(closed_btn, INPUT_PULLUP);
  pinMode(opened_btn, INPUT_PULLUP);
  pinMode(close, INPUT_PULLUP);
  pinMode(open, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(closed_btn), closed, FALLING);
  attachInterrupt(digitalPinToInterrupt(opened_btn), opened, FALLING);
}

void closed(){
  myServo.write(90);
  state = 0;
}

void opened(){
  myServo.write(90);
  state = 1;
}

void loop() {
  if(digitalRead(open) == LOW && state == 0){
    myServo.write(90 + 40);
  }
  if(digitalRead(close) == LOW && state == 1){
    myServo.write(90 - 40);
  }

}

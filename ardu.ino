#include <Servo.h>
#include <SevSeg.h>
#define bool unsigned char
#define false 0
#define true 1

Servo myServo; // Servo 객체 생성
SevSeg sevseg; // seven segment 객체 생성
uint8_t segs[4]; // 4bit 7-segment에 들어갈 데이터

const byte closed_btn = 13; // 선루프가 완전히 닫혔을 때 눌리는 버튼
const byte opened_btn = 12; // 선루프가 완전히 열렸을 때 눌리는 버튼
const byte close_btn = 10; // 닫힘 버튼
const byte open_btn = 11; // 열림 버튼
const byte obstacle_btn = 2; // 손끼임 측정 버튼
volatile int open_LED = 7; // 모터가 열리는 방향으로 돌면 켜지는 LED
volatile int close_LED = 6; // 모터가 닫히는 방향으로 돌면 켜지는 LED

volatile int direction = 1; // 선루프 방향, 1이면 열림 방향, 0이면 닫힘 방향
volatile int isCollision = 0; // 손끼임 감지 상태, 1이면 감지, 다 열고서도 3초 동안 true로 유지.
bool isOpened = false, isMoving = false, letsOpen = false, letsClose = false;
unsigned char priority = 0;
unsigned char inProgress = 0; // 수행중인 움직임
unsigned long collisionTime, currentTime;

void setup() {
  // 모터 구동 setup
  myServo.attach(9); // 9번핀에 서보모터 장착
  //버튼들 핀 설정, 풀업저항이 걸려있음
  pinMode(closed_btn, INPUT_PULLUP);
  pinMode(opened_btn, INPUT_PULLUP);
  pinMode(close_btn, INPUT_PULLUP);
  pinMode(open_btn, INPUT_PULLUP);
  pinMode(obstacle_btn, INPUT_PULLUP);
  pinMode(open_LED, OUTPUT);
  pinMode(close_LED, OUTPUT);

  //손끼임 방지버튼을 인터럽트로 처리, 손끼임 감지버튼이 HIGH -> LOW 되면 호출
  attachInterrupt(digitalPinToInterrupt(obstacle_btn), gotObstacle, FALLING);

  //FND setup
  byte numDigits = 4; 
  byte digitPins[] = {8, 3, 1, A5}; // 4자리를 선정하는 비트
  byte segmentPins[] = {5, 0, A3, A1, A0, 4, A4, A2}; //각 자리의 led 하나씩을 선정하는 비트
  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins); 
  sevseg.setBrightness(90);
  controlFND(-1); // blank 
  sevseg.setSegments(segs);
  sevseg.refreshDisplay(); // controlFND() 후 refreshDisplay()를 실행해야 문자가 표시됨

  Serial.begin(9600, SERIAL_8N1); // baudrate 9600, data 8bit, 정지비트 1bit.
}

void loop() {

  // 자석스위치 끝에 닿으면 동작 종료 및 동작 종료 메세지 발송
  if (letsOpen && digitalRead(opened_btn) == LOW) {
      letsOpen = false;
      reply(inProgress, true, true);
      inProgress = 0;
  } else if (letsClose && digitalRead(closed_btn) == LOW) {
      letsClose = false;
      reply(inProgress, true, true);
      inProgress = 0;
  }
    
  // isOpened 제어, opened_btn 눌려있으면 열렸다 처리
  if (digitalRead(opened_btn) == LOW) isOpened = true;
  else if (digitalRead(closed_btn) == LOW) isOpened = false;

  // 10초 내에 손끼임이 발생했었다면
  if(isCollision == 1){

    //선루프가 완전히 닫힐때까지(opened_btn이 눌릴때까지)
    //열리는 방향으로 선루프 제어
    while(digitalRead(opened_btn) == HIGH) open();

    //손끼임 발생 후 10초 내에는 닫히지 않음 
    currentTime = millis();
    if (currentTime >= collisionTime + 10000) {
      isCollision = 0;
    }
  }

  //손끼임이 발생하지 않았다면
  else {
    // 사용자가 두 버튼을 다 누르면 모터 정지
    if(digitalRead(open_btn) == LOW && digitalRead(close_btn) == LOW) stop();
    //사용자가 열림버튼만 눌렀거나 라즈베리 파이 지시로 열고 있는 중이라면
    else if(digitalRead(open_btn) == LOW || letsOpen) {
      //선루프가 완전히 열려있지 않다면
      if(digitalRead(opened_btn) == HIGH) {
        priority = 14 << 3; 
        controlFND(priority); // FND 제어
        sevseg.setSegments(segs);
        sevseg.refreshDisplay();
        open();
      }
      //선루프가 완전히 열려있다면
      else stop();
    }

    //사용자가 닫힘버튼만 눌렀거나 라즈베리 파이 지시로 닫고 있는 중이라면
    else if(digitalRead(close_btn) == LOW || letsClose){
      //선루프가 완전히 닫혀있지 않다면
      if(digitalRead(closed_btn) == HIGH) {
        priority = 14 << 3;
        controlFND(priority); // FND 제어
        sevseg.setSegments(segs);
        sevseg.refreshDisplay();
        close();
      }

      //선루프가 완전히 닫혀있다면
      else stop();
    }

    //사용자가 아무버튼도 누르지 않는다면 모터 정지
    else if(digitalRead(open_btn) == HIGH && digitalRead(close_btn) == HIGH) stop();
  }
}

// **************
// 동작 함수
// **************

// 인터럽트가 발생하면 실행되는 함수
void gotObstacle() {
  priority = 15 << 3;
  controlFND(priority); // FND 제어
  sevseg.setSegments(segs);
  sevseg.refreshDisplay(); 

  myServo.write(90); // 서보모터를 중지
  isMoving = false;
  isCollision = 1; // 손끼임 상태를 1로 바꿈
  collisionTime = millis(); // 부딪친 시간 확인
}

// 선루프를 여는 함수
void open() {
  direction = 1;
  myServo.write(90 + (20 * direction));
  isMoving = true;

  //열리는 led를 키고 닫히는 led를 끔
  digitalWrite(open_LED, HIGH);
  digitalWrite(close_LED, LOW);
}

//선루프를 닫는 함수
void close() {
  //닫힘방향으로 선루프를 닫음
  direction = -1;
  myServo.write(90 + (20 * direction));
  isMoving = true;

  //열리는 led를 끄고 닫히는 led를 켬
  digitalWrite(open_LED, LOW);
  digitalWrite(close_LED, HIGH);
}

//선루프를 정지하는 함수
void stop() {
  myServo.write(90); // 모터 정지
  isMoving = false;
  digitalWrite(open_LED, LOW);
  digitalWrite(close_LED, LOW);
}

void controlFND(int currentPriority){
  //현재 우선순위 그에 맞는 led를 표시해주는 함수
  // 15 << 3 : 손끼임 (obstacle)
  // 14 << 3 : 사용자 입력 (user)
  // 13 << 3 : 침수 (flooding)
  // 12 << 3 : 비 (rain)
  // 11 << 3 : 미세먼지 (dust)
  // 10 << 3 : 온도 (닫아)
  // 9 << 3 : 온도 (열어)

  switch (currentPriority) {
    case 15 << 3: //obs
      segs[0] = 92;
      segs[1] = 124;
      segs[2] = 109;
      segs[3] = 0;
      break;
    case 14 << 3: //btn
      segs[0] = 124;
      segs[1] = 120;
      segs[2] = 84;
      segs[3] = 0;
      break;
    case 13 << 3: // flod
      segs[0] = 113;
      segs[1] = 56;
      segs[2] = 92;
      segs[3] = 94;
      break;
    case 12 << 3: // rain
      segs[0] = 80;
      segs[1] = 119;
      segs[2] = 6;
      segs[3] = 84;
      break;
    case 11 << 3: // dust
      segs[0] = 94;
      segs[1] = 28;
      segs[2] = 109;
      segs[3] = 120;
      break;
    case 10 << 3: // Cool
      segs[0] = 57;
      segs[1] = 92;
      segs[2] = 92;
      segs[3] = 56;
      break;
    case 9 << 3: // Hot
      segs[0] = 118;
      segs[1] = 92;
      segs[2] = 120;
      segs[3] = 0;
      break;
    default:
      segs[0] = segs[1] = segs[2] = segs[3] = 0;
      break;
  }
}

// **************
// 이하 통신 함수
// **************

void serialEvent(){ // 메세지가 들어와야 움직입니다
  // 들어온 명령과 현재 아두이노의 상태를 비교해서 명령을 따르거나 거부합니다.
  // 명령을 따를 경우 모터를 움직이고, FND를 조작하고, 답장용 메시지를 만들어 답장을 합니다.
  // 명령을 거부할 경우 답장용 메시지를 만들어 답장을 합니다.

  unsigned char received = Serial.read();
  if (!urgentThan(received) && !sameDir(received)) {
    inProgress = received;
    // 동작 개시를 알리는 메세지 발송
    reply(received, false, true);

    // FND 제어
    priority = received & (15 << 3);
    controlFND(priority);
    sevseg.setSegments(segs);
    sevseg.refreshDisplay();

    // 모터 제어
    bool dir = false; // 여는 것이 1, 닫는 것이 0
    if (received & (1 << 2)) dir = true;
    if (dir) letsOpen = true;
    else letsClose = true;

  }
  else reply(received, true, false);
}

// 송신코드
void reply(unsigned char received, bool isDone, bool isObey) {
  unsigned char msg = received;
  if (isDone) msg |= 1 << 7;
  if (isOpened) msg |= 1 << 2;
  if (isMoving) msg |= 1 << 1;
  if (isObey) msg |= 1;
  Serial.write(msg);
}

bool urgentThan(unsigned char received) {
  // 라즈베리 파이의 명령과 아두이노의 우선순위를 비교합니다.
  return priority >= (received & (15 << 3)) ? true : false;
}

bool sameDir(unsigned char received) {
  // 열라는 명령 -> isOpened != isMoving 면 무시, 같으면 수행 / 닫으라는 명령이면 반대
  bool order = false;
  if (1 << 2 & received) order = true;
  bool target = (isOpened == isMoving) ? true : false;
  return (order == target) ? true : false;
}

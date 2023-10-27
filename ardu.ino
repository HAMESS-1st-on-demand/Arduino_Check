#include <Servo.h>
#include <SevSeg.h>
#define bool unsigned char
#define false 0
#define true 1

// 구동파트 전역변수
Servo myServo; // Servo 객체 생성
SevSeg sevseg; // seven segment 객체 생성

const byte closed_btn = 12; // 선루프가 완전히 닫혔을 때 눌리는 버튼
const byte opened_btn = 13; // 선루프가 완전히 열렸을 때 눌리는 버튼
const byte close_btn = 10; // 닫힘 버튼
const byte open_btn = 11; // 열림 버튼
const byte obstacle_btn = 2; // 손끼임 측정 버튼
volatile int direction = 1; // 선루프 방향, 1이면 열림 방향, 0이면 닫힘 방향
volatile int isCollision = 0; // 손끼임 감지 상태, 1이면 감지, 다 열고서도 3초 동안 true로 유지.
volatile int open_LED = 7; // 모터가 열리는 방향으로 돌면 켜지는 LED
volatile int close_LED = 6; // 모터가 닫히는 방향으로 돌면 켜지는 LED

// 통신파트 전역변수
//unsigned long time_previous, time_current; @todo 살려야함
bool emergency = false; // 손끼임 방지 후 3초 동안 true인 flag
//@todo 현재 선루프 위치를 항상 알아서 업데이트 해야함
bool isOpened = false;
bool isMoving = false;

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

  //인터럽트 설정, 손끼임 방지버튼을 인터럽트로 처리
  //손끼임 감지버튼이 HIGH -> LOW 되면 호출
  attachInterrupt(digitalPinToInterrupt(obstacle_btn), gotObstacle, FALLING);

  //FND setup
  byte numDigits = 4; 
  byte digitPins[] = {8, 3, 1, A5}; // 4자리를 선정하는 비트
  byte segmentPins[] = {5, 0, A3, A1, A0, 4, A4, A2}; //각 자리의 led 하나씩을 선정하는 비트

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins); 
  sevseg.setBrightness(90);
  controlFND(-1); // blank 
  sevseg.refreshDisplay(); // controlFND() 후 refreshDisplay()를 실행해야 문자가 표시됨

  // 통신 setup
  //time_previous = millis();
  Serial.begin(9600, SERIAL_8N1); // baudrate 9600, data 8bit, 정지비트 1bit.
}

void loop() {

  //손끼임이 발생했다면
  if(isCollision == 1){

    //선루프가 완전히 닫힐때까지(opened_btn이 눌릴때까지)
    //열리는 방향으로 선루프 제어
    while(digitalRead(opened_btn) == HIGH) open();

    //선루프가 완전히 개방되면 손끼임 상태를 0으로 바꿈
    isCollision = 0;
    // @todo
    // 아래 millis() 관리 코드 사용해 손 끼임 상태 3초간 유지하게 변경예정
    // time_current = millis();
    // if (time_current >= time_previous + 1000) {
    //   time_previous = time_current;
    // }
  }

  //손끼임이 발생하지 않았다면
  else {
    //사용자가 열림버튼을 눌렀다면
    if(digitalRead(open_btn) == LOW) {
      //선루프가 완전히 열려있지 않다면
      if(digitalRead(opened_btn) == HIGH) open();
      //선루프가 완전히 열려있다면
      else stop();
    }

    //사용자가 닫힘버튼을 눌렀다면
    if(digitalRead(close_btn) == LOW){
      //선루프가 완전히 닫혀있지 않다면
      if(digitalRead(closed_btn) == HIGH) close();

      //선루프가 완전히 닫혀있다면
      else stop();
    }

    //사용자가 아무버튼도 누르지 않는다면 모터 정지
    else if(digitalRead(open_btn) == HIGH && digitalRead(close_btn) == HIGH) stop();
    // @todo 사용자가 두 버튼을 다 눌러도 모터 정지
  }
}

// **************
// 동작 함수
// **************

// 인터럽트가 발생하면 실행되는 함수
void gotObstacle() {
  myServo.write(90); // 서보모터를 중지
  isCollision = 1; // 손끼임 상태를 1로 바꿈
}

// 선루프를 여는 함수
void open() {
  direction = 1;
  myServo.write(90 + (20 * direction));

  //열리는 led를 키고 닫히는 led를 끔
  digitalWrite(open_LED, HIGH);
  digitalWrite(close_LED, LOW);
}

//선루프를 닫는 함수
void close() {
  //닫힘방향으로 선루프를 닫음
  direction = -1;
  myServo.write(90 + (20 * direction));

  //열리는 led를 끄고 닫히는 led를 켬
  digitalWrite(open_LED, LOW);
  digitalWrite(close_LED, HIGH);
}

//선루프를 정지하는 함수
void stop() {
  myServo.write(90); //모터 정지
  digitalWrite(open_LED, LOW);
  digitalWrite(close_LED, LOW);
}

void controlFND(int state){
  //state를 외부에서 받아서 그에 맞는 led를 표시해주는 함수
  // @todo 모든 경우의 수 채우기
  // state == 15 << 3 : 손끼임 (obstacle)
  // state == 14 << 3 : 사용자 입력 (user)
  // state == 13 << 3 : 침수 (flooding)
  // state == 12 << 3 : 비 (rain)
  // state == 11 << 3 : 미세먼지 (dust)
  // state == 10 << 3 : 온도 (닫아)
  // state == 9 << 3 : 온도 (열어)
  // state == 8 << 3 : user preference

  switch (state) {
    case 15 << 3:
      sevseg.setChars("obs");
      break;
    case 14 << 3:
      sevseg.setChars("user");
      break;
    case 13 << 3:
      sevseg.setChars("flod");
      break;
    case 12 << 3:
      sevseg.setChars("rain");
      break;
    case 11 << 3:
      sevseg.setChars("dust");
      break;
  // @todo
  // state == 10 << 3 : 온도 (닫아)
  // state == 9 << 3 : 온도 (열어)
    case 8 << 3:
      sevseg.setChars("pref");
      break;
    default:
      sevseg.blank();
      break;
  }
}

// **************
// 이하 통신 함수
// **************

// 메세지가 들어와야 움직입니다
// MSB가 1일 때와 0일때 다른 함수를 부릅니다.
// MSB가 1일 때는 명령을 처리하고,
// MSB가 0일 때는 현재 상황을 보고합니다.
void serialEvent(){
  char received = Serial.read(); // 이건 전역변수로 선언하지 말고 인자 전달하자. 수시로 바뀌니까.
  if (received & (1 << 7)) func1(received);
  else report(received);
}

void func1(char received){
  // 들어온 명령과 현재 아두이노의 상태를 비교해서 명령을 따르거나 거부합니다.
  // 명령을 따를 경우 모터를 움직이고, FND를 조작하고, 답장용 메시지를 만들어 답장을 합니다.
  // 명령을 거부할 경우 답장용 메시지를 만들어 답장을 합니다.
  
  if (!urgentThan(received) && !sameDir(received)) {
    bool dir = false; // 여는 것이 1, 닫는 것이 0
    char speed = 15; // 그냥 예시
    if (received | 1 << 2) dir = true;
    reply(received, true);
    if (dir) open();
    else close();
    controlFND(received | 15 << 3);
    sevseg.refreshDisplay();
  }
  else reply(received, false);
}

// 명령을 따를 경우 마지막 비트를 1로,
// 명령을 따르지 않을 경우 마지막 비트를 0으로 바꿔 송신합니다.
void reply(char received, bool isObey) {
  char msg;
  if (isObey) msg = received | 1;
  else msg = received & ~1;
  Serial.write(msg);
}

void report(char received) {
  // 현재 상황을 보고합니다.
  char msg = getState();
  Serial.write(msg);
}

bool urgentThan(char received) {
  // 라즈베리 파이의 명령과 아두이노의 우선순위를 비교합니다.
  received |= 15 << 3;
  char priority = getPriority();
  return priority >= received ? true : false;
}

bool sameDir(char received) {
  // 열라는 명령 -> isOpened != isMoving 면 무시, 같으면 수행 / 닫으라는 명령이면 반대
  bool order = false;
  if (1 << 2 && received) order = true;
  bool target = isOpened == isMoving ? true : false;
  return order == target ? true : false;
}

//@todo
char getPriority() {
  return 15 << 3;
}

//@todo
char getState() {
  // 액추에이터 제어 코드와 통합 예정인 부분.
  // 아래는 그냥 예시일 뿐.
  return 255;
}

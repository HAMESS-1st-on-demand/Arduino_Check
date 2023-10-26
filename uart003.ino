#define false 0
#define true 1
#define bool unsigned char
//unsigned long time_previous, time_current;
bool emergency = false; // 손끼임 방지 후 3초 동안 true인 flag 
bool unableSSR = false;
bool isOpened = false;
bool isMoving = false;
// 열라는 명령 -> isOpened != isMoving 면 무시, 같으면 수행 / 닫으라는 명령이면 반대겠죠?

char msg = 0b00000000;

void setup() {
  //time_previous = millis();
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
  // 통신파트는 loop()문을 사용하지 않게 리팩토링했습니다.
  // 완전한 마스터-슬레이브 형태로 바뀌었기 때문에 시간을 잴 필요가 없어졌습니다.

  // 1초에 한 번씩 'A'에서 'Z'까지를 를 보냅니다.
  // 선루프 상태에 따라서 보내는 데이터의 예시 
  // millis를 이용하면 delay와 다르게 자원을 점유하지 않는다
  // time_current = millis();
  // if (time_current >= time_previous + 1000) {
  //   time_previous = time_current;
  //   Serial.write(msg);
  //   msg++;
  //   if (msg > 'Z') msg = 'A';
  // }
}

// 메세지가 들어와야 움직입니다
// MSB가 1일 때와 0일때 다른 함수를 부릅니다.
// MSB가 1일 때는 명령을 처리하고,
// MSB가 0일 때는 현재 상황을 보고합니다.
void serialEvent(){
  char received = Serial.read(); // 이건 전역변수로 선언하지 말고 인자 전달하자. 수시로 바뀌니까.
  if (received & (1 << 7)) func1(received);
  else report(received);
  //Serial.write(echo);
}

void func1(char received){
  // 들어온 명령과 현재 아두이노의 상태를 비교해서 명령을 따르거나 거부합니다.
  // 명령을 따를 경우 모터를 움직이고, FND를 조작하고, 답장용 메시지를 만들어 답장을 합니다.
  // 명령을 거부할 경우 답장용 메시지를 만들어 답장을 합니다.
  
  if (!urgentThan(received) && !sameDir(received)) {
    // 일단 여는 것을 정방향으로 가정했습니다.
    bool dir = false;
    char speed = 15; // 그냥 예시
    if (received |= 1 << 2) dir = true;
    reply(received, true);
    move(dir, speed); // move의 문제점 : 자석스위치로 인해 논블록킹 코드 어렵.
    fnd(received);
    // 조도 관련 명령일 경우 틴팅을 해버립니당. 틴팅...
  }
  else {
    reply(received, false);
  }
}

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
  bool order = false;
  if (1 << 2 && received) order = true;
  bool target = isOpened == isMoving ? true : false;
  return order == target ? true : false;
}

void move(bool dir, char speed) {
  // 모터 구동 코드와 통합 예정인 부분.
}
void fnd(char received) {
  // FND 세그먼트 제어 코드와 통합 예정인 부분.
}
char getPriority() {
  // 버튼 제어 코드와 통합 예정인 부분.
  // 아래는 그냥 예시일 뿐.
  return 15 << 3;
}
char getState() {
  // 액추에이터 제어 코드와 통합 예정인 부분.
  // 아래는 그냥 예시일 뿐.
  return 255;
}

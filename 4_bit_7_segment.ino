#include <SevSeg.h>

//seven segment 객체 생성
SevSeg sevseg; 

 //digitPin : 4자리를 선정하는 비트
 //segmentPin : 각 자리의 led 하나씩을 선정하는 비트
void setup() {
  byte numDigits = 4; 
  byte digitPins[] = {8, 3, 1, A5};
  byte segmentPins[] = {5, 0, A3, A1, A0, 4, A4, A2};

  sevseg.begin(COMMON_CATHODE, numDigits, digitPins, segmentPins); 
  sevseg.setBrightness(90);
}

//state == 0 : user
//state == 1 : dust
//state == 2 : rain
//state == 3 : flooding
//state == 4 : user preference
//state == 5 : obstacle
//state를 외부에서 받아서 그에 맞는 led를 표시해주는 함수
void write(int state){
  switch (state) {
    case 0:
      sevseg.setChars("user");
      break;
    case 1:
      sevseg.setChars("dust");
      break;
    case 2:
      sevseg.setChars("rain");
      break;
    case 3:
      sevseg.setChars("flod");
      break;
    case 4:
      sevseg.setChars("pref");
      break;
    case 5:
      sevseg.setChars("obs");
      break;
    default:
      sevseg.blank();
      break;
  }
}

//write() 후 refreshDisplay()를 실행해야 문자가 표시됨
void loop() {
  write(6);
  sevseg.refreshDisplay();
}
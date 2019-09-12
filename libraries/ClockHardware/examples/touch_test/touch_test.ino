/* 

Word clock project test sketch
This is a debug & calibration program for the 4 touch sensors.

*/

// Uncomment to get the sensors readout on serial monitor (115200 bauds)
#define SERIAL_OUTPUT

// Adjust this threshold so that touching the touchpads light up the internal LED.
#define TOUCH_THRESHOLD 76

#define LED_BUILTIN 2
#define TOUCH_1 T4
#define TOUCH_2 T5
#define TOUCH_3 T6
#define TOUCH_4 T7

bool touchDetected[4] = {false, false, false, false};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  // This is a bit useless here, but in the main program we 
  // will be using similar interrupts.
  touchAttachInterrupt(TOUCH_1, gotTouch1, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_2, gotTouch2, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_3, gotTouch3, TOUCH_THRESHOLD);
  touchAttachInterrupt(TOUCH_4, gotTouch4, TOUCH_THRESHOLD);
  
  #ifdef SERIAL_OUTPUT
  Serial.begin(115200);
  delay(1000); // give me time to bring up serial monitor
  Serial.println("Word Clock capacitive touch test program");
  #endif
}

void loop(){
  #ifdef SERIAL_OUTPUT
  String separator = "__";
  Serial.println(String(touchRead(TOUCH_1)) + separator +
      String(touchRead(TOUCH_2)) + separator +
      String(touchRead(TOUCH_3)) + separator +
      String(touchRead(TOUCH_4)));
  delay(100);
  #endif
  digitalWrite(LED_BUILTIN, LOW);
  if(isTouchDetected()){
    clearTouch();
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

bool isTouchDetected() {
  bool isIt = false;
  for (byte i = 0; i < (sizeof(touchDetected) / sizeof(touchDetected[0])); i++) {
    if (touchDetected[i]) {
      isIt = true;
      break;
    }
  }
  return isIt;
}

void clearTouch() {
  for (byte i = 0; i < (sizeof(touchDetected) / sizeof(touchDetected[0])); i++) {
    touchDetected[i] = false;
  }
}

void gotTouch(int n) {
  touchDetected[n - 1] = true;
}

void gotTouch1(){
 gotTouch(1);
}

void gotTouch2(){
 gotTouch(2);
}

void gotTouch3(){
 gotTouch(3);
}

void gotTouch4(){
 gotTouch(4);
}

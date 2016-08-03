#include <pt.h>
#include <LiquidCrystal.h>

#define PT_DELAY(pt, ms, ts) \
    ts = millis(); \
    PT_WAIT_WHILE(pt, millis()-ts < (ms));
#define SMOKE A0
#define SOUND A1

// struct pt pt_taskSmoke;
// struct pt pt_taskSound;
struct pt pt_taskSerialEvent;
struct pt pt_taskSendSerial;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
String dataToServer = "";
String recieveData = "";
int smokeData = 10;
int soundData = 0;
String people = "low";

PT_THREAD(taskSerialEvent(struct pt* pt)){
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1){
    if(Serial1.available()) {
      String str = Serial1.readStringUntil('\r');
      str.replace("\r","");
      str.replace("\n","");
      Serial.println(str);
    }
    if(Serial.available()) {
      recieveData = Serial.readStringUntil('\r');
    }
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskSendSerial(struct pt* pt)){
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1){
    Serial1.println(recieveData+","+people+","+String(smokeData));
    PT_DELAY(pt, 500, ts);
  }
  PT_END(pt);
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(115200);
  lcd.begin(16, 2);

  PT_INIT(&pt_taskSerialEvent);
  PT_INIT(&pt_taskSendSerial);
}

void loop() {
  taskSerialEvent(&pt_taskSerialEvent);
  taskSendSerial(&pt_taskSendSerial);
}

void SerialEvent() {
  if (Serial1.available()) {
      String str = Serial1.readStringUntil('\r');
      str.replace("\r","");
      str.replace("\n","");
      Serial.println(str);
  }
}

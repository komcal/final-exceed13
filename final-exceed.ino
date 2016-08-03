#include <pt.h>
#include <LiquidCrystal.h>

#define PT_DELAY(pt, ms, ts) \
    ts = millis(); \
    PT_WAIT_WHILE(pt, millis()-ts < (ms));
#define SMOKE A0
#define SOUND A1
#define TEMP A2
#define GAS A5

struct pt pt_taskSmoke;
struct pt pt_taskSound;
struct pt pt_taskGas;
struct pt pt_taskSerialEvent;
struct pt pt_taskSendSerial;
struct pt pt_taskTemp;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
String dataToServer = "";
String recieveData = "";
int smokeData = 0;
int soundData = 0;
int tempData = 0;
String people = "";
String name = "Bangkok%20saimai%201111";
int humidData = 0;
int height = 0;
int gasData = 0;
int temp;

PT_THREAD(taskSound(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    //lcd.clear();
    soundData = analogRead(SOUND);
    //lcd.print(soundData);
    if (soundData <= 200) {
      people = "Low";
    }
    else if (201 <= soundData && soundData <= 500) {
      people = "Meduim";
    }
    else if (501 <= soundData) {
        people = "High";
    }
    else {
      people = "soundData error";
    }
    PT_DELAY(pt, 500, ts);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskSmoke(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    //lcd.clear();
    smokeData = analogRead(SMOKE);
    //lcd.print(smokeData);
    PT_DELAY(pt, 500, ts);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskGas(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    //lcd.clear();
    gasData = analogRead(GAS);
    //lcd.print(gasData);
    PT_DELAY(pt, 500, ts);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskTemp(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    lcd.clear();
    temp = analogRead(TEMP);
    tempData = (temp*25-2050)/100;
    lcd.print(tempData);
    PT_DELAY(pt, 500, ts);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskSerialEvent(struct pt* pt)){
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1){
    if(Serial1.available()) {
      String str = Serial1.readStringUntil('\r');
      str.replace("\r","");
      str.replace("\n","");
      Serial.println(str);
      if(str != "") recieveData = str;
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
    Serial1.println(recieveData+","+name+","+tempData+","+people+","+humidData+","+height+","+String(smokeData)+","+gasData);
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
  PT_INIT(&pt_taskSound);
  PT_INIT(&pt_taskSmoke);
  PT_INIT(&pt_taskGas);
  PT_INIT(&pt_taskTemp);
}

void loop() {
  lcd.setCursor(0, 0);
  taskSerialEvent(&pt_taskSerialEvent);
  taskSendSerial(&pt_taskSendSerial);
  taskSound(&pt_taskSound);
  taskSmoke(&pt_taskSmoke);
  //taskGas(&pt_taskGas);
  taskTemp(&pt_taskTemp);
}

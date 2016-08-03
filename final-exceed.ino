#include <pt.h>
#include <LiquidCrystal.h>

#define PT_DELAY(pt, ms, ts) \
    ts = millis(); \
    PT_WAIT_WHILE(pt, millis()-ts < (ms));
#define SMOKE A0
#define SOUND A1
#define TEMP A2
#define GAS A4
#define BUZZER 10
#define BTN 2
#define LIGHT 13

struct pt pt_taskSmoke;
struct pt pt_taskSound;
struct pt pt_taskGas;
struct pt pt_taskSerialEvent;
struct pt pt_taskSendSerial;
struct pt pt_taskTemp;
struct pt pt_taskBeep;
struct pt pt_taskHumid;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);
String dataToServer = "";
String recieveData = "";
int smokeData = 0;
int soundData = 0;
int tempData = 0;
String people = "";
String name = "Bangkok%20saimai%201111";
int humidData = 20;
int gasData = 0;
int temp;
int sw;

long iPPM_LPG = 0;
long iPPM_Smoke = 0;
int CALIBARAION_SAMPLE_TIMES=50;                    //define how many samples you are going to take in the calibration phase
int CALIBRATION_SAMPLE_INTERVAL=500;                //define the time interal(in milisecond) between each samples in the
                                                    //cablibration phase
int READ_SAMPLE_INTERVAL=50;                        //define how many samples you are going to take in normal operation
int READ_SAMPLE_TIMES=5;                            //define the time interal(in milisecond) between each samples in


const int calibrationLed = 13;                      //when the calibration start , LED pin 13 will light up , off when finish calibrating
int RL_VALUE=5;                                     //define the load resistance on the board, in kilo ohms
float RO_CLEAN_AIR_FACTOR=9.83;                     //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO,

#define         GAS_LPG             0
#define         GAS_CO              1
#define         GAS_SMOKE           2

/*****************************Globals***********************************************/
float           LPGCurve[3]  =  {2.3,0.21,-0.47};   //two points are taken from the curve.
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.21), point2: (lg10000, -0.59)
float           COCurve[3]  =  {2.3,0.72,-0.34};    //two points are taken from the curve.
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.72), point2: (lg10000,  0.15)
float           SmokeCurve[3] ={2.3,0.53,-0.44};    //two points are taken from the curve.
                                                    //with these two points, a line is formed which is "approximately equivalent"
                                                    //to the original curve.
                                                    //data format:{ x, y, slope}; point1: (lg200, 0.53), point2: (lg10000,  -0.22)
float           Ro           =  9;                 //Ro is initialized to 10 kilo ohms

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
      people = "Medium";
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
    if (iPPM_Smoke > 50) {
      analogWrite(BUZZER, 100);
      analogWrite(LIGHT, 100);
    }
    else {
      analogWrite(BUZZER, 0);
      analogWrite(LIGHT,  0);
    }
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

PT_THREAD(taskHumid(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    //lcd.clear();
    sw = digitalRead(BTN);
    if (sw == 0) {
      if(humidData == 20) humidData = 70;
      else humidData = 20;
    }
    //lcd.print(humidData);
    PT_DELAY(pt, 700, ts);
    PT_YIELD(pt);
  }
  PT_END(pt);
}

PT_THREAD(taskTemp(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  while (1) {
    //lcd.clear();
    temp = analogRead(TEMP);
    tempData = (temp*25-2050)/100;
    //lcd.print(tempData);
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
      if(str != "" && str.indexOf("/") < 0) {
        recieveData = str;
        analogWrite(LIGHT, 100);
        taskBeep(&pt_taskBeep);
        analogWrite(LIGHT, 0);
      }

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
    Serial1.println(name+","+tempData+","+people+","+humidData+","+String(smokeData)+","+gasData);
    PT_DELAY(pt, 500, ts);
  }
  PT_END(pt);
}

PT_THREAD(taskBeep(struct pt* pt)) {
  static uint32_t ts;
  PT_BEGIN(pt);
  tone(BUZZER, 800,2000);
  delay(1000);
  noTone(BUZZER);
  tone(BUZZER, 800,2000);
  delay(1000);
  noTone(BUZZER);
  tone(BUZZER, 800,2000);
  delay(1000);
  noTone(BUZZER);
  PT_END(pt);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(115200);
  lcd.begin(16, 2);
  pinMode(BTN, INPUT);
  pinMode(LIGHT, OUTPUT);

  PT_INIT(&pt_taskSerialEvent);
  PT_INIT(&pt_taskSendSerial);
  PT_INIT(&pt_taskSound);
  PT_INIT(&pt_taskSmoke);
  PT_INIT(&pt_taskGas);
  PT_INIT(&pt_taskTemp);
  PT_INIT(&pt_taskBeep);
  PT_INIT(&pt_taskHumid);
}

void loop() {
  lcd.setCursor(0, 0);;

  iPPM_LPG = MQGetGasPercentage(MQRead(SMOKE)/Ro,GAS_LPG);
  iPPM_Smoke = MQGetGasPercentage(MQRead(SMOKE)/Ro,GAS_SMOKE);


   lcd.setCursor( 0 , 0 );
   lcd.print("LPG: ");
   lcd.print(iPPM_LPG);
   lcd.print(" ppm");   
   
//   lcd.setCursor( 0, 2 );
//   lcd.print("CO: ");
//   lcd.print(iPPM_CO);
//   lcd.print(" ppm");    
//
   lcd.setCursor( 0,1 );
   lcd.print("Smoke: ");
   lcd.print(iPPM_Smoke);
   lcd.print(" ppm");  



  taskSerialEvent(&pt_taskSerialEvent);
  taskSendSerial(&pt_taskSendSerial);
  taskSound(&pt_taskSound);
  taskSmoke(&pt_taskSmoke);
  taskGas(&pt_taskGas);
  taskTemp(&pt_taskTemp);
  taskHumid(&pt_taskHumid);
}

float MQResistanceCalculation(int raw_adc)
{
  return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
}
float MQRead(int mq_pin)
{
  int i;
  float rs=0;

  for (i=0;i<READ_SAMPLE_TIMES;i++) {
    rs += MQResistanceCalculation(analogRead(mq_pin));
    delay(READ_SAMPLE_INTERVAL);
  }

  rs = rs/READ_SAMPLE_TIMES;

  return rs;
}
long MQGetGasPercentage(float rs_ro_ratio, int gas_id)
{
  if ( gas_id == GAS_LPG ) {
     return MQGetPercentage(rs_ro_ratio,LPGCurve);
  } else if ( gas_id == GAS_CO ) {
     return MQGetPercentage(rs_ro_ratio,COCurve);
  } else if ( gas_id == GAS_SMOKE ) {
     return MQGetPercentage(rs_ro_ratio,SmokeCurve);
  }

  return 0;
}
long  MQGetPercentage(float rs_ro_ratio, float *pcurve)
{
  return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
}

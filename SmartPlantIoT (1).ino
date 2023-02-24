//***********************||เรียกใช้ไลบรารี่||***********************
#include <TimeLib.h>
#include<WiFiManager.h>
#define BLYNK_TEMPLATE_ID   "TMPLRWV8iueG"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char auth[] = "TGpVZzBTSIbhlX5oSFFLyWb4syK9w2n3";
char ssid[] = "**********";
char pass[] = "**********";
//***********************||กำหนดตัวแปร||***********************
int WifiReset      = 16;//D0
int relay          = D2;
int sensor         = A0;
const int dry = 720;
const int wet = 300;
int mois_raw_value = 0;
int mois_percent   = 0;
int custom_moisture = 40;
int switchMode = 0;
int manualControl = 0;
int timerMode = 0;
String currentDay;
String currentTime;
String startTime, startTime2;
String stopTime, stopTime2;
BlynkTimer timer;
//***********************||เขียนคำสั่งการทำงาน||***********************
BLYNK_CONNECTED() {
    Blynk.syncAll();
    Blynk.sendInternal("rtc", "sync");
}
BLYNK_WRITE(V7){ //รับค่าจากการปรับค่าความชื้นในแอพพลิเคชั่น
  custom_moisture = param.asInt();
}
BLYNK_WRITE(V5){ // รับค่าสวิซจากแอพพลิเคชั่น
  switchMode = param.asInt();
}
BLYNK_WRITE(V1){ //รับค่าเวลาจากแอพพลิเคชั่นโดยการใช้งาน Time Library
  TimeInputParam t(param);
  if(t.hasStartTime()){// ต่าค่าเวลาเริ่ม
    startTime = String(t.getStartHour()) + ":" + t.getStartMinute() + ":" + t.getStartSecond();
    Serial.println("Start At: " + startTime);
  } else if (t.isStartSunrise()) {
    Serial.println("Start at sunrise");
  } else if (t.isStartSunset()) {
    Serial.println("Start at sunset");
  } else {   
  }
  if(t.hasStopTime()){// ต่าค่าเวลาหยุด
    stopTime = String(t.getStopHour()) + ":" + t.getStopMinute() + ":" + t.getStopSecond();
    Serial.println("Stop At: " + stopTime);
  } else if(t.isStopSunrise()) {
    Serial.println("Stop at sunrise");
  } else if (t.isStopSunset()) {
    Serial.println("Stop at sunset");
  } else {
  }
  Serial.println(String("Time zone: ") + t.getTZ());// เรียกใช้ Time zone
  Serial.println(String("Time zone offset: " ) + t.getTZ_Offset());// เรียกใช้ Time zone แบบวินาที
  for(int i = 1; i <= 7; i++){//เชควัน
    if(t.isWeekdaySelected(i)){
      Serial.println(String("Day ") + i + " is selected");
    }
  }
}
BLYNK_WRITE(V2){ //รับค่าเวลาจากแอพพลิเคชั่นโดยการใช้งาน Time Library สำหรับการตั้งเวลาครั้งที่ 2
  TimeInputParam t(param);
  if(t.hasStartTime()){ // ต่าค่าเวลาเริ่ม
     startTime2 = String(t.getStartHour()) + ":" + t.getStartMinute() + ":" + t.getStartSecond();
    Serial.println("Start At: " + startTime2);
  } else if (t.isStartSunrise()) {
    Serial.println("Start at sunrise");
  } else if (t.isStartSunset()) {
    Serial.println("Start at sunset");
  } else {   
  }
  if(t.hasStopTime()){ // ต่าค่าเวลาหยุด
    stopTime2 = String(t.getStopHour()) + ":" + t.getStopMinute() + ":" + t.getStopSecond();
    Serial.println("Stop At: " + stopTime2);
  } else if(t.isStopSunrise()) {
    Serial.println("Stop at sunrise");
  } else if (t.isStopSunset()) {
    Serial.println("Stop at sunset");
  } else {
  }
  Serial.println(String("Time zone: ") + t.getTZ()); // เรียกใช้ Time zone
  Serial.println(String("Time zone offset: " ) + t.getTZ_Offset()); // เรียกใช้ Time zone แบบวินาที
  for(int i = 1; i <= 7; i++){ //เชควัน
    if(t.isWeekdaySelected(i)){
      Serial.println(String("Day ") + i + " is selected");
    }
  }
}

BLYNK_WRITE(V6) { // รับค่าสวิซจากแอพพลิเคชั่น เมื่อกดปุ่ม Manual
  manualControl = param.asInt();
}
BLYNK_WRITE(V3){ // รับค่าสวิซจากแอพพลิเคชั่น เมื่อกดปุ่ม ตั้งเวลา
  timerMode = param.asInt();
}

BLYNK_WRITE(InternalPinRTC) { // การเปลงค่าเวลา Unix Time ให้เป็น Date time โดยการใช้ ฟังชั่นของ Time Library ที่มีมาให้
  long DefaultTime = 1357041600; // Jan 01 2013 
  long t = param.asLong(); 
  if(t >= DefaultTime){
    setTime(t);
    Serial.println(t);
    currentTime = String(hour()) + ":" + minute() + ":" + second();
  Serial.print("Current time: "); Serial.println(currentTime);
  }
}
//***********************||สรัางฟังชั่นเพื่อเรียกใช้||***********************
void clockDisplay(){ //ฟังชั่นใช้แสดงเวลา
  Blynk.sendInternal("rtc", "sync");
}

void moisture () { //ฟังชั่นแสดงค่าความชื้น
    mois_raw_value = analogRead(sensor);
    mois_percent   = map(mois_raw_value, wet, dry, 100, 0);
    Serial.print("Moisture: "); Serial.print(mois_percent); Serial.println("%");
    Serial.print("Moisture in dirt: "); Serial.print(custom_moisture);
    Blynk.virtualWrite(V4, mois_percent);
}

void plantMode () { //ฟังชั่นในการเปลี่ยนโหมด
  if(switchMode == 0) { //โหมดรดน้ำอัตโนมัติ
    Serial.println("\nAuto Plant Mode Activated!!");
    if(mois_percent <= custom_moisture) {
       digitalWrite(relay, LOW);
    } else {
      digitalWrite(relay, HIGH);
    }
  } else if (timerMode == 1) { //โหมดตั้งเวลา
    Serial.println("\nTimer Mode Activated!!");
    if(startTime == currentTime){
      Serial.println("Pump is on!!");
      digitalWrite(relay, LOW);
    } else if (stopTime == currentTime){
      Serial.println("Pump off!!");
      digitalWrite(relay, HIGH);
    }
    if(startTime2 == currentTime){
      Serial.println("Pump is on!!");
      digitalWrite(relay, LOW);
    } else if (stopTime2 == currentTime){
      Serial.println("Pump off!!");
      digitalWrite(relay, HIGH);
    }     
  } else { //โหมดสั่งทำงานด้วยมือ
    Serial.println("\nManual Mode Activated!!");
    digitalWrite(relay, HIGH);
    if(manualControl == 1) {
      digitalWrite(relay, LOW); 
    } 
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
 //***********************||ติดตั้ง WifiManager||***********************
  pinMode(WifiReset, INPUT_PULLUP);
  WiFiManager wifiManager;
  if(digitalRead(WifiReset) == LOW){
    Serial.println("Reset wifi!!");
    wifiManager.resetSettings();
  }
  wifiManager.autoConnect("Project A");
  //***********************||สิ้นสุดการติดตั้ง||***********************
  Blynk.config(auth);
  Blynk.virtualWrite(V1, currentTime);
  Blynk.virtualWrite(V2, currentTime); 
  setSyncInterval(10*60);
  timer.setInterval(1000L, moisture); // เรียกใช้ฟังชั่นแสดงค่าความชื้น
  timer.setInterval(1000L, plantMode); // เรียกใช้ฟังชั่นในการเปลี่ยนโหมด
  timer.setInterval(1000L, clockDisplay); // เรียกใช้ฟังชั่นแสดงเวลา
}
  //***********************||สั่งทำงาน||***********************
void loop()
{
  Blynk.run();
  timer.run();
}

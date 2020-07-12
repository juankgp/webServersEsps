#include <Arduino.h>
#include <FS.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <DS3232RTC.h>     
#include <Streaming.h>      
#include <Wire.h>
#include "ota.h"

int addr = 0;
int totalHor = 0;
int totalMin = 0;
int totalSeg = 0;
int totaltEspera = 0;

int cambioluz=0;


void setup() {

#pragma region //Configuracio
  Serial.begin(115200);
  pinMode(ozono,OUTPUT);//ozono
  pinMode(humo,OUTPUT);
  pinMode(luces,OUTPUT);
  pinMode(vent1,OUTPUT);
  pinMode(vent2,OUTPUT);
  pinMode(vent3,OUTPUT);
  pinMode(vent4,OUTPUT);
 
  digitalWrite(humo,LOW);
  digitalWrite(ozono,LOW);
  digitalWrite(luces,LOW);
  digitalWrite(vent1,LOW);
  digitalWrite(vent2,LOW);
  digitalWrite(vent3,LOW);
  digitalWrite(vent4,LOW);
  
 
  EEPROM.begin(512);
  
  char host[16];
  Serial.println("Iniciando");

  Wire.begin(9, 10);

  //AsyncWiFiManager wifiManager(&server,&dns);
  //wifiManager.resetSettings();
 // wifiManager.autoConnect("testAPJK", "juank123");
if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }
 //writetxt("FLA GUTIERREZ:KYARALIBY:");
 //writetxt("");

readtxt();

  int sepssid = wifiData.indexOf(":");
  int seppass = wifiData.indexOf(":",sepssid+1);
  String ssidtext = wifiData.substring(0,sepssid);
  String passtext = wifiData.substring(sepssid+1,seppass);
  //Serial.println("datos" + ssidtext + "---" +passtext);
  //ssidtext = "FLA GUTIERREZ";
  //passtext = "KYARALIBY";


 #ifdef ESP8266
  snprintf(host, 12, "ESP%08X", ESP.getChipId());
  #else
  snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  #endif



   if(!wifiConnect(host,readtxt(),ssidtext,passtext)) {
     Serial.println("Connection failed");
     //digitalWrite(redLed,LOW);
     return;
   }
   //digitalWrite(redLed,HIGH);
  MDNS.begin(host);
  if(webInit()) MDNS.addService("http", "tcp", 80);
  /*  tmElements_t tm;
     tm.Hour = 00;               // set the RTC to an arbitrary time
     tm.Minute = 05;
     tm.Second = 00;
     tm.Day = 04;
     tm.Month = 07;
     tm.Year = 2020 - 1970;      // tmElements_t.Year is the offset from 1970
     RTC.write(tm);*/     
  setSyncProvider(RTC.get);
	setSyncInterval(300);

   if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");


  #pragma endregion
 
  requests();
  //*****************************************************
  Alarm.timerOnce(timeInit*multipliSeg, AlarmFunctionON);
  alarmWork=Alarm.timerOnce(timeTrabajo*multipliSeg, AlarmDisableWork);
  
  /*digitalWrite(ozono,HIGH);
  digitalWrite(vent1,HIGH);
  digitalWrite(vent2,HIGH);
  digitalWrite(vent3,HIGH);
  digitalWrite(vent4,HIGH);*/
  Serial.println("Alarmas creadas");
}

void loop(){

  Alarm.delay(1000);
  
  Serial.println(String(hour()) + ":" + String(minute()) + ":" +String(second()));

}


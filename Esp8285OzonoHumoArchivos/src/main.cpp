#include <Arduino.h>

#include <Updater.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <FS.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
  
#include <Streaming.h>      
#include <Wire.h>

#include "ota.h"
int totalHor = 0;
int totalMin = 0;
int totalSeg = 0;
int totaltEspera = 0;

//DNSServer dns;

int cambioluz=0;

#define MYSSID "FLA GUTIERREZ"
#define PASSW "KYARALIBY"

void setup() {
#pragma region //Configuracio
  Serial.begin(115200);
  pinMode(ozono,OUTPUT);//ozono
  pinMode(humo,OUTPUT);
 
  pinMode(vent1,OUTPUT);
  pinMode(vent2,OUTPUT);
  pinMode(vent3,OUTPUT);
  //pinMode(vent4,OUTPUT);
 
  digitalWrite(humo,LOW);
  digitalWrite(ozono,LOW);
  
  digitalWrite(vent1,LOW);
  digitalWrite(vent2,LOW);
  digitalWrite(vent3,LOW);
  //digitalWrite(vent4,LOW);
  
 
  EEPROM.begin(512);
  char host[16];
  Serial.println("Iniciando");

  Wire.begin(9, 10);


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

  setSyncProvider(RTC.get);
	setSyncInterval(300);

   if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");


  #pragma endregion
 
requests();
  //*****************************************************
  //Alarm.timerOnce(timeInit*multipliSeg, AlarmFunctionON);
  alarmWork=Alarm.timerOnce(timeTrabajo*multipliSeg, AlarmDisableWork);
  
  //digitalWrite(ozono,LOW);
  /*digitalWrite(vent1,HIGH);
  digitalWrite(vent2,HIGH);
  digitalWrite(vent3,HIGH);
  digitalWrite(vent4,HIGH);*/
  Serial.println("Alarmas creadas Ozono On");
}

void loop(){

  Alarm.delay(1000);
  if(analogRead(A0)>=1000)
    AlarmFunctionON();
   else
    AlarmFunctionOFF();
      
  Serial.println("Analog: " + String(analogRead(A0)));
  Serial.println(String(hour()) + ":" + String(minute()) + ":" +String(second()));

}


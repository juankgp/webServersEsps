#include <Arduino.h>
#include <FS.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <DS3232RTC.h>     
#include <Streaming.h>      
#include <Wire.h>
#include "ota.h"


int totalHor = 0;
int totalMin = 0;
int totalSeg = 0;
int totaltEspera = 0;



void setup() {

#pragma region //Configuracio
  Serial.begin(115200);
  pinMode(ozono,OUTPUT);//ozono

  digitalWrite(ozono,LOW);
 
  EEPROM.begin(512);
  ton = EEPROM.read(addr);
  toff = EEPROM.read(addr+1);
  
  char host[16];
  Serial.println("Iniciando: " + String(ton) + "-------"+ String(toff));
  Serial.println("Tiempos guardados");
  Wire.begin(9, 10);

  //AsyncWiFiManager wifiManager(&server,&dns);
  //wifiManager.resetSettings();
 // wifiManager.autoConnect("testAPJK", "juank123");
if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }


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
  
  
  /*digitalWrite(ozono,HIGH);
  digitalWrite(vent1,HIGH);
  digitalWrite(vent2,HIGH);
  digitalWrite(vent3,HIGH);
  digitalWrite(vent4,HIGH);*/
 
}

void loop(){

  Alarm.delay(1000);
  
  Serial.println(String(hour()) + ":" + String(minute()) + ":" +String(second()));

}


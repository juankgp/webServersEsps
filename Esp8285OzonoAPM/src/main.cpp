#include <Arduino.h>
#include <FS.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
//#include <DS3232RTC.h>     
#include <Streaming.h>      
#include <Wire.h>
#include "ota.h"


void setup() {


  Serial.begin(115200);
  pinMode(ozono,OUTPUT);//ozono
  pinMode(led,OUTPUT);//led
 Wire.begin(9, 10);
  digitalWrite(ozono,LOW);
  digitalWrite(led,LOW);
   setSyncProvider(RTC.get);
	setSyncInterval(300);

   if(timeStatus() != timeSet)
        Serial.println("Unable to sync with the RTC");
    else
        Serial.println("RTC has set the system time");

 
  EEPROM.begin(512);
  ton = EEPROM.read(addr);
  toff = EEPROM.read(addr+1);
  tHoraIni = EEPROM.read(addr+2);
  tMinIni = EEPROM.read(addr+3);
  tHoraFin = EEPROM.read(addr+4);
  tMinFin = EEPROM.read(addr+5);
  estadoHorario = EEPROM.read(addr+6);
  Serial << "Estado de horario " << estadoHorario << endl;
  if(estadoHorario == 1){
    Serial << "Alarmas creadas inicio: " << "Hora: " << tHoraIni <<
    "Minuto: " << tMinIni << "FIN: " << tHoraFin << "Minuto: " << tMinFin << endl;
    alarmHorarioOn =  Alarm.alarmRepeat(tHoraIni,tMinIni,0, AlarmHorarioON);  
    alarmHorarioOff =  Alarm.alarmRepeat(tHoraFin,tMinFin,0, AlarmHorarioOFF);  
    estadoHorario = true;
  }
  else
  {
    Serial << "Alarmas no creadas horario desactivad" << endl;
    estadoHorario = false;
    Alarm.disable(alarmHorarioOn);
    Alarm.disable(alarmHorarioOff);
  }
  
  char host[2];
  /*Serial.println("Iniciando: " + String(ton) + "-------"+ 
  String(toff) + "Horario" + String(tHoraIni) + "-------"+ 
  String(tMinIni) + "-------"+ String(tHoraFin) + "-------"+
  String(tMinFin));*/
  Serial << "Iniciando" << ton << "---" << toff <<
  "Horario: " << tHoraIni << "----" << tMinIni <<
  "---" << tHoraFin << "---" << tMinFin << endl;
  //Serial.println("Tiempos guardados");
  Serial << "Tiempos Guardados" <<endl;
 

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
  snprintf(host, 12, "ESP%08X", "1");
  #else
  snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  #endif



   if(!wifiConnect(host,readtxt(),ssidtext,passtext)) {
     Serial <<"Connection failed" << endl;
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
  
  Serial << hour() << ":" << minute() << ":" << second() << endl;

}

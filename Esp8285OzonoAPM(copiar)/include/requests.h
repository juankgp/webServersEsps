#include <DS3232RTC.h>   
#include <ESPAsyncWebServer.h> 
#include "pinout.h"
const char* PARAM_INPUT_1 = "hola";
const char* PARAM_INPUT_2 = "adios";
DS3232RTC RTC;
String timeValue = "10";
bool work = false;
AlarmId alarmWork, alarmEspera,alarmHorarioOn,alarmHorarioOff;
AsyncWebServer server(80);
File myFile;

const char* myFilePath = "/wifi.txt";


bool estadoHorario = false;
int multipliSeg = 1;
int timeTrabajo = 20;
int addr = 0;
int ton = 0;
int toff = 0;
int tHoraIni = 0;
int tMinIni = 0;
int tHoraFin = 0;
int tMinFin = 0;

void writetxt(String datos){
   myFile = SPIFFS.open(myFilePath, "w");
  if (myFile.print(datos)){
    Serial << "Message successfully written" << endl;  
  }
  else{
    Serial << "Writting message failled!!" << endl;
  }
  myFile.close();
}

 String processor(const String& var){
 //Serial.println("VAr"+var);
  // if(var == "STATE"){
  //   if(digitalRead(ledPin)){
  //     ledState = "ON";
  //   }
  //   else{
  //     ledState = "OFF";
  //   }
  //   //Serial.println(ledState);
  //   return ledState;
  // }
  // else 
  if(var == "TIMEUC"){
    String data;
    return data;
  }
  else if(var == "STATET"){
 
    String data1;
    return data1;
  }
  return String();
}
void AlarmFunctionOFF();
void AlarmFunctionON();
void AlarmHorarioOFF();
void AlarmHorarioON();

void AlarmHorarioON(){
  digitalWrite(ozono,HIGH);
  Serial << "Prendo Por Horario" << endl;
}
void AlarmHorarioOFF(){
  digitalWrite(ozono,LOW);
  Serial << "Apago Por Horario" << endl;
}
void AlarmFunctionON(){
  if (work)
  {
    digitalWrite(ozono,HIGH);
    Serial << "PrendoOzono" << endl;
  
    //Alarm.timerRepeat(timeHumo*60, AlarmFunctionOFF);
    alarmWork=Alarm.timerOnce(ton*multipliSeg, AlarmFunctionOFF);  
  }
  
  
}
void AlarmFunctionOFF(){
  if(work){
   digitalWrite(ozono,LOW);
    Serial << "ApagoOzono" << endl;
    alarmEspera = Alarm.timerOnce(toff*multipliSeg, AlarmFunctionON);
  }
  
}

void AlarmDisableWork(){
  work = false;
 // digitalWrite(humo,LOW);
  digitalWrite(ozono,LOW);
 
}
 void requests(){
     server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });

 /////////////////////////////////////////////////////////
 //***************paginas y procesos*********************
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
   server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/stylecfg.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/stylecfg.css", "text/css");
  });
  server.on("/onozono", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial << "Prende ozono" << endl;
    digitalWrite(ozono,HIGH);
    
   
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offozono", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial <<"apaga ozono" << endl;
    digitalWrite(ozono,LOW);
    
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  

 
server.on("/arranque", HTTP_GET, [](AsyncWebServerRequest *request){
    work=true;
    //Alarm.timerOnce(toff*multipliSeg, AlarmFunctionOFF);
    alarmEspera = Alarm.timerOnce(1, AlarmFunctionON);
    Serial << "Alarmas creadas" << endl;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    work=false;
    Alarm.disable(alarmWork);
    Alarm.disable(alarmEspera);
    //Alarm.timerOnce(toff*multipliSeg, AlarmFunctionOFF);
    digitalWrite(ozono,LOW);
    Serial << "Paro" << endl;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });


 server.on("/verifHorario", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(estadoHorario)
      cadena = "true";
    else
      cadena = "false";
  request->send_P(200,"text/plain",cadena.c_str());
  });


server.on("/horasHorario", HTTP_GET, [](AsyncWebServerRequest *request){
    String cadena;
    
    cadena = "Hora ON: " + String(tHoraIni) + ":" + String(tMinIni) +"<br>"+ "Hora OFF: " +
     String(tHoraFin) + ":" + String(tMinFin);
    
  request->send_P(200,"text/plain",cadena.c_str());
});
  server.on("/indicadorOZ", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(digitalRead(ozono))
      cadena = "led-green-on";
    else
      cadena = "led-red-off";
  request->send_P(200,"text/plain",cadena.c_str());
  });
  


////funcion para capturar las credenciales de wifi

server.on("/wifidata", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      String wifiData = String(inputMessage);
      
    /*int sepssid = wifiData.indexOf(":");
    int seppass = wifiData.indexOf(":",sepssid+1);
    String ssidtext = wifiData.substring(0,sepssid);
    String passtext = wifiData.substring(sepssid+1,seppass);
     Serial.println("Datos Wifi" + ssidtext + "----" + passtext);*/
     Serial << "Datos Wifi: " << wifiData << endl;
    
     writetxt(wifiData);
     ESP.restart();
    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });


server.on("/tiempodata", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      String tdata = String(inputMessage);
      
    int septon = tdata.indexOf(":");
    int septoff = tdata.indexOf(":",septon+1);
    String ston = tdata.substring(0,septon);
    String stoff = tdata.substring(septon+1,septoff);
     Serial <<"Datos de tiempo" << ston + "----"<< stoff << endl;
    
     //writetxt(wifiData);
      EEPROM.write(addr,ston.toInt());
      EEPROM.write(addr+1, stoff.toInt());
      EEPROM.commit();
      ton = ston.toInt();
      toff =stoff.toInt();
    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });

  server.on("/dataHorario", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      String tdata = String(inputMessage);
      //Serial << "Horario: " <<  tdata << endl;
      int sepHoraIni= tdata.indexOf(":");
      int sepMinIni = tdata.indexOf(":",sepHoraIni+1);
      int sepHoraFin= tdata.indexOf(":",sepMinIni+1);
      int sepMinFin = tdata.indexOf(":",sepHoraFin+1);
      String sHoraIni = tdata.substring(0,sepHoraIni);
      String sMinIni = tdata.substring(sepHoraIni+1,sepMinIni);
      String sHoraFin = tdata.substring(sepMinIni+1,sepHoraFin);
      String sMinFin = tdata.substring(sepHoraFin+1,sepMinFin);
      Serial << "Datos de Horario: "  << sHoraIni << "----" << sMinIni << "------" 
      << sHoraFin << "----" << sMinFin;
    
     //writetxt(wifiData);
      EEPROM.write(addr +2,sHoraIni.toInt());
      EEPROM.write(addr+3, sMinIni.toInt());
      EEPROM.write(addr +4,sHoraFin.toInt());
      EEPROM.write(addr+5, sMinFin.toInt());
      EEPROM.commit();
      tHoraIni = sHoraIni.toInt();
      tMinIni = sMinIni.toInt();
      tHoraFin = sHoraFin.toInt();
      tMinFin = sMinFin.toInt();
      Alarm.disable(alarmHorarioOn);
      Alarm.disable(alarmHorarioOff);
      if(estadoHorario == true){
        alarmHorarioOn =  Alarm.alarmRepeat(tHoraIni,tMinIni,0, AlarmHorarioON);  
        alarmHorarioOff =  Alarm.alarmRepeat(tHoraFin,tMinFin,0, AlarmHorarioOFF);  
      }
      
    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });

server.on("/activeHorario", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      String tdata = String(inputMessage);
     Serial << "ACTIVAR: "<< tdata << endl;
     if(tdata == "activado"){
       estadoHorario = true;

      alarmHorarioOn =  Alarm.alarmRepeat(tHoraIni,tMinIni,0, AlarmHorarioON);  
      alarmHorarioOff =  Alarm.alarmRepeat(tHoraFin,tMinFin,0, AlarmHorarioOFF);  
      EEPROM.write(addr+6, 1);
      EEPROM.commit();
      Serial <<"Alarmas creadas de horario"<< endl;
     }
     else
     {
       Serial <<"Alarmas desabilitadas de horario"<< endl;
       EEPROM.write(addr+6, 0);
      EEPROM.commit();
       estadoHorario = false;
       Alarm.disable(alarmHorarioOn);
       Alarm.disable(alarmHorarioOff);
     }

    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/index.html", String(), false, processor);
  });


 

 }
 
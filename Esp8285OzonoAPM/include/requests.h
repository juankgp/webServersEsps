#include <DS3232RTC.h>   
#include <ESPAsyncWebServer.h> 
#include "pinout.h"
const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
DS3232RTC RTC;
String timeValue = "10";
boolean work = false;
AlarmId alarmWork, alarmEspera;
AsyncWebServer server(80);
File myFile;

const char* myFilePath = "/wifi.txt";
int minActual = 0;

int timeInit = 2;
int timeOzono = 1;
int timeEspera = 1;
int timeVent = 5;
int multipliSeg = 1;
int timeTrabajo = 20;
int addr = 0;
int ton = 0;
int toff = 0;

void writetxt(String datos){
   myFile = SPIFFS.open(myFilePath, "w");
  if (myFile.print(datos)){
    Serial.println("Message successfully written");  
  }
  else{
    Serial.print("Writting message failled!!");
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

void AlarmFunctionON(){
  if (work)
  {
    digitalWrite(ozono,HIGH);
    Serial.println("PrendoOzono");
  
    //Alarm.timerRepeat(timeHumo*60, AlarmFunctionOFF);
    alarmWork=Alarm.timerOnce(ton*multipliSeg, AlarmFunctionOFF);  
  }
  
  
}
void AlarmFunctionOFF(){
  if(work){
   digitalWrite(ozono,LOW);
    Serial.println("ApagoOzono");
    alarmEspera = Alarm.timerOnce(toff*multipliSeg, AlarmFunctionON);
  }
  
}
void AlarmApagoVent(){
 
   Serial.println("ApagoVentiladores");
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
    Serial.println("Prende ozono y vents");
    digitalWrite(ozono,HIGH);
    
   
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offozono", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("apaga ozono y vents");
    digitalWrite(ozono,LOW);
    
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  

 
server.on("/arranque", HTTP_GET, [](AsyncWebServerRequest *request){
    work=true;
    //Alarm.timerOnce(toff*multipliSeg, AlarmFunctionOFF);
    alarmEspera = Alarm.timerOnce(1, AlarmFunctionON);
    Serial.println("Alarmas creadas");
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    work=false;
    Alarm.disable(alarmWork);
    Alarm.disable(alarmEspera);
    //Alarm.timerOnce(toff*multipliSeg, AlarmFunctionOFF);
    digitalWrite(ozono,LOW);
    Serial.println("Paro");
    request->send(SPIFFS, "/index.html", String(), false, processor);
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
     Serial.println("Datos Wifi: " + wifiData);
    
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
     Serial.println("Datos de tiempo" + ston + "----" + stoff);
    
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
      Serial.println("Horario: " + tdata);
      int sepHoraIni= tdata.indexOf(":");
      int sepMinIni = tdata.indexOf(":",sepHoraIni+1);
      int sepHoraFin= tdata.indexOf(":",sepMinIni+1);
      int sepMinFin = tdata.indexOf(":",sepHoraFin+1);
      String sHoraIni = tdata.substring(0,sepHoraIni);
      String sMinIni = tdata.substring(sepHoraIni+1,sepMinIni);
      String sHoraFin = tdata.substring(sepMinIni+1,sepHoraFin);
      String sMinFin = tdata.substring(sepHoraFin+1,sepMinFin);
      Serial.println("Datos de Horario: " + sHoraIni + "----" + sMinIni +"------"+ sHoraFin + "----" + sMinFin);
    
     //writetxt(wifiData);
      /*EEPROM.write(addr,ston.toInt());
      EEPROM.write(addr+1, stoff.toInt());
      EEPROM.commit();
      ton = ston.toInt();
      toff =stoff.toInt();*/
    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });

server.on("/ini", HTTP_GET, [](AsyncWebServerRequest *request){//sincronizoReloj
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      timeValue = String(inputMessage);
      
     // Serial.println("PARAM_INPUT2 "+ timeValue);
      int sepAnio = timeValue.indexOf(":");
      int sepMes = timeValue.indexOf(":",sepAnio+1);
      int sepDia = timeValue.indexOf(":",sepMes+1);
      int sepHora = timeValue.indexOf(":",sepDia+1);
      int sepMin = timeValue.indexOf(":",sepHora+1);
      int sepSeg = timeValue.indexOf(":",sepMin+1);
      String anio = timeValue.substring(0,sepAnio);
      String mes = timeValue.substring(sepAnio+1,sepMes);
      String dia = timeValue.substring(sepMes+1,sepDia);
      String hora = timeValue.substring(sepDia+1,sepHora);
      String minuto = timeValue.substring(sepHora+1,sepMin);
      String segundo = timeValue.substring(sepMin+1,sepSeg);
      
      
      Serial.println(hora + "-" + minuto + "-" + segundo + "-" + dia + "-" +  (mes.toInt()+1) + "-" + anio);

      tmElements_t tm;
        tm.Hour = hora.toInt();               // set the RTC to an arbitrary time
       tm.Minute = minuto.toInt();
       tm.Second = segundo.toInt()+1;
       tm.Day = dia.toInt();
       tm.Month = mes.toInt()+1;//el mas es mas 1 porque el html empiza los mese desde 0
       tm.Year = anio.toInt() - 1970;      // tmElements_t.Year is the offset from 1970
       RTC.write(tm);     

      setSyncProvider(RTC.get);
	    setSyncInterval(300);
     
     
    }
    else {
      inputMessage = "No message sent"; 
    }

   request->send(SPIFFS, "/config.html", String(), false, processor);
  });
 }
 
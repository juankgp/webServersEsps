#include <DS3232RTC.h>   
#include <ESPAsyncWebServer.h> 
#include "pinout.h"
#include <Separador.h>
#include <vector>
using std::vector;

using VS=vector<String>;
vector<byte> vEeprom;
Separador s;


const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
DS3232RTC RTC;
String timeValue = "10";
bool work = false;
AlarmId alarmWork, alarmEspera,alarmHorarioOn,alarmHorarioOff;
AsyncWebServer server(80);
File myFile;

const char* myFilePath = "/wifi.txt";


//bool estadoHorario = false;
byte multipliSeg = 60;
byte timeTrabajo = 20;
byte addr = 0;
/*byte ton = 0;
byte toff = 0;
byte tHoraIni = 0;
byte tMinIni = 0;
byte tHoraFin = 0;
byte tMinFin = 0;*/

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
  digitalWrite(ac1,HIGH);
  digitalWrite(ac2,HIGH);
  Serial << "Prendo Por Horario" << endl;
}
void AlarmHorarioOFF(){
  digitalWrite(ac1,LOW);
  digitalWrite(ac2,LOW);
  Serial << "Apago Por Horario" << endl;
}
void AlarmFunctionON(){
  if (work)
  {
    digitalWrite(ac1,HIGH);
    digitalWrite(ac2,HIGH);
    Serial << "PrendoOzono" << endl;
  
    //Alarm.timerRepeat(timeHumo*60, AlarmFunctionOFF);
    alarmWork=Alarm.timerOnce(vEeprom[0]*multipliSeg, AlarmFunctionOFF);  
  }
  
  
}
void AlarmFunctionOFF(){
  if(work){
   digitalWrite(ac1,LOW);
   digitalWrite(ac2,LOW);
    Serial << "ApagoOzono" << endl;
    alarmEspera = Alarm.timerOnce(vEeprom[1]*multipliSeg, AlarmFunctionON);
  }
  
}

void AlarmDisableWork(){
  work = false;
 // digitalWrite(humo,LOW);
  digitalWrite(ac1,LOW);
  digitalWrite(ac2,LOW);
 
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
    digitalWrite(ac1,HIGH);
    digitalWrite(ac2,HIGH);
    
   
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offozono", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial <<"apaga ozono" << endl;
    digitalWrite(ac1,LOW);
    digitalWrite(ac2,LOW);
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
    digitalWrite(ac1,LOW);
    digitalWrite(ac2,LOW);
    Serial << "Paro" << endl;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });


 server.on("/verifHorario", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(vEeprom[6])
      cadena = "true";
    else
      cadena = "false";
  request->send_P(200,"text/plain",cadena.c_str());
  });


server.on("/horasHorario", HTTP_GET, [](AsyncWebServerRequest *request){
    String cadena;
    
    cadena = "Hora ON: " + String(vEeprom[2]) + ":" + String(vEeprom[3]) +"<br>"+ "Hora OFF: " +
     String(vEeprom[4]) + ":" + String(vEeprom[5]);
    
  request->send_P(200,"text/plain",cadena.c_str());
});
server.on("/horasH", HTTP_GET, [](AsyncWebServerRequest *request){
    String cadena;
    
    cadena = String(hour()) + ":" + String(minute()) + ":" + String(second());
    
  request->send_P(200,"text/plain",cadena.c_str());
});
  server.on("/indicadorOZ", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(digitalRead(ac1))
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
      vector<String> vComandos;
      //String vComandos[2];
    for(int i=0;i<2;i++){
      String dato = s.separa(tdata,':',i);
      //vComandos[i]=dato;
    
       vComandos.push_back(dato);
    }
    /*for(int i=0;i<2;i++){
     
      Serial << "Llego: " <<vComandos[i] << endl;
       
    }*/
      
      EEPROM.write(addr,vComandos[0].toInt());
      EEPROM.write(addr+1, vComandos[1].toInt());
      EEPROM.commit();
      vEeprom[0] = vComandos[0].toInt();
      vEeprom[1] =vComandos[1].toInt();
      Serial << "Ton: " << vEeprom[0] << " Toff: " << vEeprom[1] << endl;
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

    vector<String> vComandos;
    for(int i=0;i<4;i++){
      String dato = s.separa(tdata,':',i);
      vComandos.push_back(dato);
       
    }
    
      Serial << "Datos de Horario: "  << vComandos[0] << ":" << vComandos[1] << "------" 
      << vComandos[2] << ":" << vComandos[3];
    
     //writetxt(wifiData);
      EEPROM.write(addr +2,vComandos[0].toInt());
      EEPROM.write(addr+3, vComandos[1].toInt());
      EEPROM.write(addr +4,vComandos[2].toInt());
      EEPROM.write(addr+5, vComandos[3].toInt());
      EEPROM.commit();
      vEeprom[2] = vComandos[0].toInt();
      vEeprom[3] = vComandos[1].toInt();
      vEeprom[4] = vComandos[2].toInt();
      vEeprom[5] = vComandos[3].toInt();
      Alarm.disable(alarmHorarioOn);
      Alarm.disable(alarmHorarioOff);
      if(vEeprom[6] == true){
        alarmHorarioOn =  Alarm.alarmRepeat(vEeprom[2],vEeprom[3],0, AlarmHorarioON);  
        alarmHorarioOff =  Alarm.alarmRepeat(vEeprom[4],vEeprom[5],0, AlarmHorarioOFF);  
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
       vEeprom[6] = true;

      alarmHorarioOn =  Alarm.alarmRepeat(vEeprom[2],vEeprom[3],0, AlarmHorarioON);  
      alarmHorarioOff =  Alarm.alarmRepeat(vEeprom[4],vEeprom[5],0, AlarmHorarioOFF);  
      EEPROM.write(addr+6, 1);
      EEPROM.commit();
      Serial <<"Alarmas creadas de horario"<< endl;
     }
     else
     {
       Serial <<"Alarmas desabilitadas de horario"<< endl;
       EEPROM.write(addr+6, 0);
      EEPROM.commit();
       vEeprom[6] = false;
       Alarm.disable(alarmHorarioOn);
       Alarm.disable(alarmHorarioOff);
     }

    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/index.html", String(), false, processor);
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
    vector<String> vComandos;
    for(int i=0;i<6;i++){
      String dato = s.separa(timeValue,':',i);
      vComandos.push_back(dato);
       
    }
    
      
      Serial << "Hora Actual: "<< vComandos[3]<< "-" << vComandos[4] << "-" << vComandos[5] << 
      "-" << vComandos[2] << "-" <<  (vComandos[1].toInt()+1) << "-" << vComandos[0] << endl;

      tmElements_t tm;00000000000000000000000000000000000000000000000000000000000000000000000000000000000
      tm.Hour = vComandos[3].toInt();               // set the RTC to an arbitrary time
       tm.Minute = vComandos[4].toInt();
       tm.Second = vComandos[5].toInt()+1;
       tm.Day = vComandos[2].toInt();
       tm.Month = vComandos[1].toInt()+1;//el mas es mas 1 porque el html empiza los mese desde 0
       tm.Year = vComandos[0].toInt() - 1970;      // tmElements_t.Year is the offset from 1970
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
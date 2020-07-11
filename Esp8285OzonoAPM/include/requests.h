#include <DS3232RTC.h>    
const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
DS3232RTC RTC;
String timeValue = "10";
boolean work = true;
AlarmId alarmWork, alarmEspera;
AsyncWebServer server(80);
File myFile;
const char* myFilePath = "/wifi.txt";
int minActual = 0;
int humo = 2;
int vent1 = 14;
int vent2 =16;
int vent3 = 12;
int vent4 = 13;
int luces = 4;
int ozono = 5;//ozono
int timeInit = 2;
int timeHumo = 1;
int timeEspera = 1;
int timeVent = 5;
int multipliSeg = 60;
int timeTrabajo = 20;

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
    digitalWrite(humo,HIGH);
    minActual = minute();
    Serial.println("PrendoHumo");
  
    //Alarm.timerRepeat(timeHumo*60, AlarmFunctionOFF);
    Alarm.timerOnce(timeHumo*multipliSeg, AlarmFunctionOFF);  
  }
  
  
}
void AlarmFunctionOFF(){
  if(work){
    digitalWrite(humo,LOW);
    Serial.println("ApagoHumo");
    alarmEspera = Alarm.timerOnce(timeEspera*multipliSeg, AlarmFunctionON);
  }
  
}
void AlarmApagoVent(){
  digitalWrite(vent1,LOW);
  digitalWrite(vent2,LOW);
  digitalWrite(vent3,LOW);
  digitalWrite(vent4,LOW);
   Serial.println("ApagoVentiladores");
}
void AlarmDisableWork(){
  work = false;
  digitalWrite(humo,LOW);
  digitalWrite(ozono,LOW);
  Alarm.timerOnce(timeVent*multipliSeg, AlarmApagoVent);
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
    digitalWrite(vent1,HIGH);
    digitalWrite(vent2,HIGH);
    digitalWrite(vent3,HIGH);
    digitalWrite(vent4,HIGH);
   
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offozono", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("apaga ozono y vents");
    digitalWrite(ozono,LOW);
    digitalWrite(vent1,LOW);
    digitalWrite(vent2,LOW);
    digitalWrite(vent3,LOW);
    digitalWrite(vent4,LOW);
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  

  server.on("/timeuc", HTTP_GET, [](AsyncWebServerRequest *request){
    String cadena = "Hola Mundo JK!!!!!!!!!!!1";
  request->send_P(200,"text/plain",cadena.c_str());
  });
  server.on("/indicadorHU", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(digitalRead(humo))
      cadena = "led-green-on";
    else
      cadena = "led-red-off";
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
  server.on("/indicadorVT", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    if(digitalRead(vent1))
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
      
    /*int sepssid = wifiData.indexOf(":");
    int seppass = wifiData.indexOf(":",sepssid+1);
    String ssidtext = wifiData.substring(0,sepssid);
    String passtext = wifiData.substring(sepssid+1,seppass);
     Serial.println("Datos Wifi" + ssidtext + "----" + passtext);*/
     Serial.println("Datos Tiempo: " + tdata);
    
     //writetxt(wifiData);
     work =true;
     Alarm.disable(alarmWork);
     Alarm.timerOnce(timeInit*multipliSeg, AlarmFunctionON);
    alarmWork=Alarm.timerOnce(tdata.toInt()*multipliSeg, AlarmDisableWork);
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
 
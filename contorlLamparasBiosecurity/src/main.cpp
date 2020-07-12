#include <Arduino.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <ESP8266mDNS.h>
#include <Updater.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <Separador.h>
#include <vector>
#include<string.h>
#include <sstream>

using std::vector;

using VS=vector<String>;
Separador s;
//vector<String> vComandos(20);
String vComandos[20];
String inputString="";
boolean strComplete=false;
int addr = 0;
bool ffsec=false,ffmin=false,ffhor=false;
bool l1=false,l2=false,l3=false,l4=false;
bool starttime = false;
String l5 = "READY";
//const char* ssid = "MiMix2s";
//const char* password = "wlanjuank1";
const char* ssid = "biosecurity";
const char* password = "lampsBISOP";
const int ledPin = 2;
String ledState;
IPAddress ip(192,168,1,155);  //Node static IP
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);
AsyncWebServer server(80);
size_t content_len;
const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";



String timerSliderValue = "300";
String timeValue = "10";
String data ="hola";
String data1 ="hola";
int totalHor = -1;
int totalMin = -1;
int totalSeg = -1;
int totaltEspera = -1;

bool firsttime=true;
int iHora, iMin, iSeg, itEspera;
int vHora, vMin, vSeg, vtEspera;
void handleRoot(AsyncWebServerRequest *request) {
  request->redirect("/update");
}

void handleUpdate(AsyncWebServerRequest *request) {
  char* html = "<form method='POST' action='/doUpdate' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  request->send(200, "text/html", html);
}

void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index){
    Serial.println("Update");
    content_len = request->contentLength();
    // if filename includes spiffs, update the spiffs partition
    int cmd = (filename.indexOf("spiffs") > -1) ?  100 : U_FLASH ;

    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {

      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);

  } else {
    Serial.printf("Progress: %d%%\n", (Update.progress()*100)/Update.size());

  }

  if (final) {
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
    response->addHeader("Refresh", "20");  
    response->addHeader("Location", "/");
    request->send(response);
    if (!Update.end(true)){
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      Serial.flush();
      ESP.restart();
    }
  }
}

void printProgress(size_t prg, size_t sz) {
  Serial.printf("Progress: %d%%\n", (prg*100)/content_len);
}

boolean webInit() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {request->redirect("/update");});
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){handleUpdate(request);});
  server.on("/doUpdate", HTTP_POST,
    [](AsyncWebServerRequest *request) {},
    [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data,
                  size_t len, bool final) {handleDoUpdate(request, filename, index, data, len, final);}
  );
  server.onNotFound([](AsyncWebServerRequest *request){request->send(404);});
  server.begin();

  Update.onProgress(printProgress);

}

String processor(const String& var){
 //Serial.println("VAr"+var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    //Serial.println(ledState);
    return ledState;
  }
  else if(var == "TIMEUC"){
   // totalMin = timeValue.toInt() +minute();
    //totalSeg = second();
    data = String(iHora) + "h: " + String(iMin) +"m: " + String(iSeg) + "s" + "<br>T evacuación:" + String(itEspera);
   // Serial.println(data);
    
    return data;
  }
  else if(var == "STATET"){
 
    int HorPresen = vHora;
    int minPrsen = vMin;
    int segPresen = vSeg;
    
    data1 = String(HorPresen) + "h: " + String(minPrsen) +"m: " + String(segPresen) + "s" ;
    //Serial.println(data1);
    return data1;
  }else if(var == "ESTADOS"){
 
    data1 = String(l1) + "&" + String(l2) + "&" + String(l3) + "&" + String(l4);
    //Serial.println(data1);
    return data1;
  }
  else if(var == "LAMP1"){
    String estado;
    if(l1) estado="Lon";
    else estado="Loff";
    data1 = "<img src="+ estado + ">";
    return data1; 
  }
  else if(var == "LAMP2"){
    String estado;
    if(l2) estado="Lon";
    else estado="Loff";
    data1 = "<img src="+ estado + ">";
    return data1; 
  }
  else if(var == "LAMP3"){
    String estado;
    if(l3) estado="Lon";
    else estado="Loff";
    data1 = "<img src="+ estado + ">";
    return data1; 
  }
  else if(var == "LAMP4"){
    String estado;
    if(l4) estado="Lon";
    else estado="Loff";
    data1 = "<img src=" + estado + ">";
    return data1; 
  }
  else if(var == "COUNTDOWN"){
    
    data1 = String(vtEspera);
    return data1; 
  }
  else if(var == "ESTADOEST"){
    
    String estado;
    String texto;
    if(l5 == "READY"){
      estado="green";
      texto = "Esterilizador Listo para operar";
    }    
    else if (l5 == "WAIT")
    {
      estado="orange";
      texto = "Evacuacion Del Lugar A Esterilizar";
    }
    else if (l5 == "ONWORK")
    {
      estado="red";
      texto = "Esterilizador en Funcionamineto";
    }
    data1 = "<img src=" + estado + "> <br>" + texto;

    

    return data1;
  }
  
  return String();
 
}
String outputState(){
  if(digitalRead(ledPin)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}

void timerAlarm(){
  digitalWrite(ledPin,LOW);
  Serial.println("@@&RESET&<<");
}
void arranque(){
    Serial.println("@@&START&<<");
    totalSeg = iSeg + second();
    totalMin = iMin +minute();
    totalHor = iHora + hour(); 
    Alarm.alarmOnce(totalHor,totalMin,totalSeg, timerAlarm); 
   
}
 
void serialEvent() {
	//@@&APM&STATE&1&1&0&0&<<
  int numSep=0;
  int numBegin=0;
	while (Serial.available()) {
		// get the new byte:

		char inChar = (char)Serial.read();

		inputString += inChar;
		//Serial.println(inChar);
		if (inChar == '<') {
			strComplete = true;
			Serial.read();
		}
    if (inChar == '&') {
			numSep++;
		}
	}

	if (strComplete) {

    for(int i=0;i<numSep;i++){
      String dato = s.separa(inputString,'&',i);
      vComandos[i]=dato;
       
    }
   for(int i=0;i<numSep;i++){
       if(vComandos[i]=="STATE"){
         numBegin=i;
         break;
       }
    }
    l1 = vComandos[numBegin+1].toInt();
    l2 = vComandos[numBegin+2].toInt();
    l3 = vComandos[numBegin+3].toInt();
    l4 = vComandos[numBegin+4].toInt();
    l5 = vComandos[numBegin+5];

    //Serial.println("-----------"+String(l1) + "-" + String(l2) + "-" + String(l3) + "-" +String(l4) + "-" + l5 );
  inputString="";
  strComplete=false;

	}
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
   char host[16];
  setTime(1,1,1,1,1,2020);
   EEPROM.begin(512);
  iHora = EEPROM.read(addr);
  iMin = EEPROM.read(addr+1);
  iSeg = EEPROM.read(addr+2);
  itEspera = EEPROM.read(addr+3);
  // Connect to Wi-Fi
  pinMode(ledPin, OUTPUT);
  //snprintf(host, 12, "control");
  WiFi.mode(WIFI_AP);

  while(!WiFi.softAP(ssid, password))
  {
   Serial.println(".");
    delay(100);
  }
  
  Serial.print("Iniciado AP ");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  // Print ESP32 Local IP Address
 MDNS.begin(host);
  if(webInit()) MDNS.addService("http", "tcp", 80);
 // Serial.printf("Ready! Open http://%s.local in your browser\n", host);
  //Serial.println(WiFi.localIP());

  //Serial.println("Ultimo tsetup: " + String(iHora) + "-" + String(iMin) + "-" + String(iSeg));

/////////////////////////
// Route for root / web page
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/timer.html", String(), false, processor);
  });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/nosotros", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/nosotros.html", String(), false, processor);
  });
  server.on("/apmlogolow", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/apmlogolow.png", "image/png");
  });
  server.on("/botonSetup", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/botonSetup.png", "image/png");
  });
  server.on("/botonTimer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/botonTimer.png", "image/png");
  });
  server.on("/botonNosotros", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/botonNosotros.png", "image/png");
  });
  server.on("/botonInicio", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/botonInicio.png", "image/png");
  });
  server.on("/cuadro_luzUV", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/cuadro_luzUV.png", "image/png");
  });
  server.on("/transparencia", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/transparencia.png", "image/png");
  });
  server.on("/Lon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Lon.png", "image/png");
  });
  server.on("/Loff", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/Loff.png", "image/png");
  });
  server.on("/green", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/green.png", "image/png");
  });
  server.on("/red", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/red.png", "image/png");
  });
  server.on("/orange", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/orange.png", "image/png");
  });

  server.on("/play", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/play.png", "image/png");
  });


  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
     Serial.println("@@&APM&LAMP&ON&");
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("@@&APM&LAMP&OFF&");
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
 
  server.on("/timeStart", HTTP_GET, [](AsyncWebServerRequest *request){
    
      
    request->send(SPIFFS, "/timeStart.html", String(), false, processor);
  });
  server.on("/timeStart", HTTP_GET, [](AsyncWebServerRequest *request){

    request->send(SPIFFS, "/nosotros.html", String(), false, processor);
  });
  server.on("/arranque", HTTP_GET, [](AsyncWebServerRequest *request){
      iHora = EEPROM.read(addr);
      iMin = EEPROM.read(addr+1);
      iSeg = EEPROM.read(addr+2);
      itEspera = EEPROM.read(addr+3);
      vHora = iHora;
      vMin = iMin;
      vSeg = iSeg;
      vtEspera = itEspera;
      
      //totalSeg = iSeg + second();
      //totalMin = iMin +minute();
      //totalHor = iHora + hour();
      Alarm.timerOnce(vtEspera,arranque); 
      //Alarm.alarmOnce(totalHor,totalMin,totalSeg, timerAlarm); 
    digitalWrite(ledPin, HIGH); 
     Serial.println("@@&APM&TIME&00:00&&<<");//al otro micro 
    starttime = true;   
    request->send(SPIFFS, "/timer.html", String(), false, processor);
  });
  server.on("/ini", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      timeValue = String(inputMessage);
      //Serial.println("PARAM_INPUT2 "+ timeValue);
      int sepHora = timeValue.indexOf(":");
      int sepMin = timeValue.indexOf(":",sepHora+1);
      int sepSeg = timeValue.indexOf(":",sepMin+1);
      String hora = timeValue.substring(0,sepHora);
      String minuto = timeValue.substring(sepHora+1,sepMin);
      String segundo = timeValue.substring(sepMin+1,sepSeg);
      String tEspera = timeValue.substring(sepSeg+1);
      //Serial.println(hora + "-" + minuto + "-" + segundo + "-" + sepSeg);
     
     

      iHora = hora.toInt();
      iMin = minuto.toInt();
      iSeg = segundo.toInt();
      itEspera = tEspera.toInt();

      EEPROM.write(addr, iHora);
      EEPROM.write(addr+1, iMin);
      EEPROM.write(addr+2, iSeg);
      EEPROM.write(addr+3, itEspera);
      EEPROM.commit();
      vHora = iHora;
      vMin = iMin;
      vSeg = iSeg;

      vtEspera = itEspera;
      totalSeg = iSeg + second();
      totalMin = iMin +minute();
      totalHor = iHora + hour();
      totaltEspera = itEspera + second();
      //Alarm.timerOnce(totaltEspera,arranque); 
      //Alarm.alarmOnce(totalHor,totalMin,totalSeg, timerAlarm); 
      starttime = false;
    }
    else {
      inputMessage = "No message sent";
     
      
      
    }
   
  
   request->send(SPIFFS, "/timer.html", String(), false, processor);
  });


}
 
void loop(){
 
   serialEvent();

  
   if(starttime){
      vtEspera--;
      Serial.println("@@&LAMP&<<");
     if(vtEspera<=0){
      vSeg--;
      vtEspera = 0;
      
      if(vSeg < 0){
        if(vMin>=1 || vHora>=1){
          vSeg = 59;
          vMin--;
      }
      else{
        vSeg = 0;
      }
      }
      if(vMin < 0){
      if(vHora>=1){
          vMin = 59;
          vHora--;
      }
      else
      {
        vMin=0;
      } 
      }
    }
   }
    
   
   Alarm.delay(1000);
}

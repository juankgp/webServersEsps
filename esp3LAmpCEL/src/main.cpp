#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "SPIFFS.h"
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <Separador.h>
#include <Update.h>
#include <ESPmDNS.h>
// Replace with your network credentials


//const int ledPin = 2;
const int ledGreen = 18;
const int ldr1 = 34;
const int ldr2 = 35;
const int ldr3 = 32;
const int ldr4 = 33;
const int lampSiren = 16;
const int orange = 4;
const int green = 2;
const int red = 15;
const int out1 = 5;
int ldr1Value = 0;
int ldr2Value = 0;
int ldr3Value = 0;
int ldr4Value = 0;
bool estadoled = 0;

//const char* ssid = "MiMix2s";
//const char* password = "wlanjuank1";
IPAddress ip(192,168,43,155);  //Node static IP
IPAddress gateway(192,168,43,1);
IPAddress subnet(255,255,255,0);
const char* ssid = "Biosecurity OP";
const char* password = "lampsBISOP";
size_t content_len;
bool l1=false,l2=false,l3=false,l4=false;
String l5 = "READY";
String data1 ="hola";
String data ="hola";
int iHora, iMin, iSeg, itEspera;
int vHora, vMin, vSeg, vtEspera;
bool controlArranque = false;
String inputString="";
boolean strComplete=false;
int addr = 0;
int totalHor = -1;
int totalMin = -1;
int totalSeg = -1;
int totaltEspera = -1;
Separador s;
String timeValue = "10";
// Set LED GPIO

bool starttime = false;
bool pausa = false;
AlarmId alarm1, arranque1, alarm2,alarm3;
// Stores LED state
String ledState;

const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

boolean wifiConnect(char* host) {
#ifdef MYSSID
  WiFi.begin(MYSSID,PASSWD);
  WiFi.waitForConnectResult();
#else
  WiFi.begin();
  WiFi.waitForConnectResult();
#endif
  Serial.println(WiFi.localIP());
  return (WiFi.status() == WL_CONNECTED);
}

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
    int cmd = (filename.indexOf("spiffs") > -1) ? U_SPIFFS : U_FLASH;
#ifdef ESP8266
    Update.runAsync(true);
    if (!Update.begin(content_len, cmd)) {
#else
    if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd)) {
#endif
      Update.printError(Serial);
    }
  }

  if (Update.write(data, len) != len) {
    Update.printError(Serial);
#ifdef ESP8266
  } else {
    Serial.printf("Progress: %d%%\n", (Update.progress()*100)/Update.size());
#endif
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
#ifdef ESP32
  Update.onProgress(printProgress);
#endif
}


// Replaces placeholder with LED state value
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
  else if(var == "LDRS"){
    data1 = "ldr1= " + String(analogRead(ldr1)) + "     ldr2= " + String(analogRead(ldr2)) + "     ldr3= " + String(analogRead(ldr3)) + "     ldr4= " + String(analogRead(ldr4));
    return data1;
  }
   else if(var == "OUTPUTS"){
    data1 = "green: " + String(digitalRead(green)) + "   orange: " + String(digitalRead(orange)) +
  " red: " + String(digitalRead(red)) + "  out: " + String(digitalRead(out1)) + " siren: " + String(digitalRead(lampSiren));
    return data1;
  }
  return String();
}
void timerAlarm(){
  //digitalWrite(ledPin,LOW);
  Serial.println("@@&RESET&<<");
  controlArranque=false;
  l5 = "READY";
  pausa = true;
  digitalWrite(orange,LOW);
  digitalWrite(red,LOW);
  digitalWrite(green,HIGH);
  digitalWrite(out1,LOW);
  digitalWrite(lampSiren,LOW);
  digitalWrite(lampSiren,HIGH);
  delay(3000);
  digitalWrite(lampSiren,LOW);
}

void arranque(){
    Serial.println("@@&START&<<");
    totalSeg = iSeg + second();
    totalMin = iMin +minute();
    totalHor = iHora + hour(); 
    alarm1 = Alarm.alarmOnce(totalHor,totalMin,totalSeg, timerAlarm); 
   
}
void arranqueresume(){
    Serial.println("@@&START&<<");
    alarm3 = Alarm.alarmOnce(hour()+vHora,minute()+vMin,second()+vSeg, timerAlarm); 
   
}
 
void setup(){
  // Serial port for debugging purposes

//hardware//////////

pinMode(ledGreen, OUTPUT);
pinMode(lampSiren, OUTPUT);
pinMode(orange, OUTPUT);
pinMode(green, OUTPUT);
pinMode(red, OUTPUT);
pinMode(out1, OUTPUT);
digitalWrite(green,HIGH);
digitalWrite(red,LOW);
digitalWrite(orange,LOW);
digitalWrite(out1,LOW);
digitalWrite(lampSiren,LOW);
/////////////



  Serial.begin(9600);
 
   char host[16];
   snprintf(host, 16, "ESP%012llX", ESP.getEfuseMac());
  setTime(1,1,1,1,1,2020);
   EEPROM.begin(512);
  iHora = EEPROM.read(addr);
  iMin = EEPROM.read(addr+1);
  iSeg = EEPROM.read(addr+2);
  itEspera = EEPROM.read(addr+3);
  

  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi


  //WiFi.mode(WIFI_AP_STA);
 //WiFi.begin(ssid, password);
  //WiFi.config(ip, gateway, subnet); 
  //WiFi.softAPdisconnect(true);
  int i=10;
   //while(i<10){
  /*   while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.println("WiFi failed, retrying.");
    delay(1000);
    i++;
  }*/
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

  //WiFi.begin(ssid, password);
  //while (WiFi.status() != WL_CONNECTED) {
  //  delay(1000);
  //  Serial.println("Connecting to WiFi..");
 // }

  // Print ESP32 Local IP Address
 // Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  // Route to load style.css file

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
  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
   // digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
 
  // Route to set GPIO to LOW
  

  server.on("/arranque", HTTP_GET, [](AsyncWebServerRequest *request){

    if (!controlArranque)//falso
    {
      controlArranque=true;
      Alarm.disable(alarm1);
      Alarm.disable(arranque1);
      Alarm.disable(alarm2);
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
      arranque1 = Alarm.timerOnce(vtEspera,arranque); 
          pausa=false;

      //Alarm.alarmOnce(totalHor,totalMin,totalSeg, timerAlarm); 
    
     Serial.println("@@&APM&TIME&00:00&&<<");//al otro micro 
      starttime = true;   
      
    }
    request->send(SPIFFS, "/index.html", String(), false, processor);

  });

server.on("/pausa", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("@@&STOP&<<");//al otro micro 
    pausa=true;
    //Alarm.alarmOnce(hour()+5,60,60, timerAlarm); 
    Alarm.disable(alarm1);
    Alarm.disable(arranque1);
    Alarm.disable(alarm2);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("@@&STOP&<<");//al otro micro 
    pausa=true;
    //Alarm.alarmOnce(hour()+5,60,60, timerAlarm); 
    Alarm.disable(alarm1);
    Alarm.disable(arranque1);
    Alarm.disable(alarm2);
     Alarm.disable(alarm3);
     controlArranque=false;
    vSeg=0;
    vMin=0;
    vHora=0;
    vtEspera=0;
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/resume", HTTP_GET, [](AsyncWebServerRequest *request){
     
    //digitalWrite(ledPin, HIGH); 
    if (vtEspera>0)
    {
      Serial.println("@@&APM&TIME&00:00&&<<");
    }
    else
    {
      if(vSeg == 0 && vMin == 0 && vHora == 0)
      {
        Serial.println("@@&STOP&<<");//al otro micro
         pausa=true;
      }
      else
      {
        Serial.println("@@&START&<<");//al otro micro 
         pausa=false;
      }
      
    }
   
    Serial.println("VtEspera= " + String(vtEspera));
    if (vtEspera<=0)
    {
       if(vSeg == 0 && vMin == 0 && vHora == 0)
        {
          Serial.println("@@&STOP&<<");//al otro micro
           pausa=true;
        }
      else
        {
          Serial.println("pausa false");
          alarm2 = Alarm.timerOnce(1,arranqueresume); 
           pausa=false;
        }
    }
    else
    {
      pausa=false;
      alarm2 = Alarm.timerOnce(vtEspera,arranqueresume); 
    }
    //Alarm.alarmOnce(hour()+vHora,minute()+vMin,second()+vSeg, timerAlarm); 
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/ini", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
     Serial.println("@@&STOP&<<");//al otro micro 
    pausa=true;
    //Alarm.alarmOnce(hour()+5,60,60, timerAlarm); 
    Alarm.disable(alarm1);
    Alarm.disable(arranque1);
    Alarm.disable(alarm2);
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
      iSeg = segundo.toInt();http://192.168.43.155/
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
   request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Start server
  //server.begin();
  // if(!wifiConnect(host)) {
   // Serial.println("Connection failed");
   // return;
 // }
   MDNS.begin(host);
   if(webInit()) MDNS.addService("http", "tcp", 80);
   Serial.printf("Ready! Open http://%s.local in your browser\n", host);
   Serial.println("@@&RESET&<<");
}
 
void loop(){
 
   Alarm.delay(1000);
  ldr1Value = analogRead(ldr1);
  ldr2Value = analogRead(ldr2);
  ldr3Value = analogRead(ldr3);
  ldr4Value = analogRead(ldr4);
  ldr1Value >= 400 ? l1=true : l1=false;
  ldr2Value >= 400 ? l2=true : l2=false;
  ldr3Value >= 400 ? l3=true : l3=false;
  ldr4Value >= 400 ? l4=true : l4=false;

  

  //Serial.println("ldr1: " + String(ldr1Value) + "    ldr2: " + String(ldr2Value) + "    ldr3: " + String(ldr3Value) + "  ldr4: " + String(ldr4Value) );
  //Serial.println("GREEN: " + String(digitalRead(green)) + "   orange: " + String(digitalRead(orange)) +
  //" red: " + String(digitalRead(red)) + "  out: " + String(digitalRead(out1)) + "siren: " + String(digitalRead(lampSiren)));

  if (!pausa)
  {
    if(starttime)
    {
        vtEspera--;
        if (vtEspera>0)
        {
          Serial.println("@@&LAMP&<<");
          l5 = "WAIT";
          digitalWrite(lampSiren,!digitalRead(lampSiren));
          digitalWrite(orange,HIGH);
          digitalWrite(red,LOW);
          digitalWrite(green,LOW);
          digitalWrite(out1,LOW);
        }
      if(vtEspera<=0){
        vSeg--;
        vtEspera = 0;
        l5 = "ONWORK";
        
        digitalWrite(orange,LOW);
          digitalWrite(red,HIGH);
          digitalWrite(green,LOW);
          digitalWrite(out1,HIGH);
          digitalWrite(lampSiren,LOW);
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
   
  }
   else
    {
      l5 = "READY";
      digitalWrite(orange,LOW);
      digitalWrite(red,LOW);
      digitalWrite(green,HIGH);
      digitalWrite(out1,LOW);
      digitalWrite(lampSiren,LOW);

    }
  
  
}
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Updater.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <FS.h>
#include <TimeLib.h>
#include <EEPROM.h>
#include <TimeAlarms.h>
#include <DS3232RTC.h>     
#include <Streaming.h>      
#include <Wire.h>
#include "requests.h"




int addr = 0;
int totalHor = 0;
int totalMin = 0;
int totalSeg = 0;
int totaltEspera = 0;


//DS3232RTC RTC;




//DNSServer dns;
size_t content_len;
int cambioluz=0;
String wifiData="";
#define MYSSID "FLA GUTIERREZ"
#define PASSW "KYARALIBY"
char* ssid = "esp8285";
char* password = "esp8285pass";
int pausa=5000;

//int redLed =16;
//int ozono = 14;
//int rly2 = 16;
//int ozono = 12;
//int rly2 = 13;


/*#define LOCALSERVER "yes"//comentar para conectarse a una redfs.h
 


#ifdef LOCALSERVER
 const char* ssid = "esp8285";
 const char* password = "esp8285pass";
#else
  #define MYSSID "FLA GUTIERREZ"
  #define PASSW "KYARALIBY"
  
#endif
*/

void appendtxt(String datos){
  myFile = SPIFFS.open(myFilePath, "a");
  if(myFile.print(datos)){
    Serial.println("Message successfully appended");  
  }
  else{
    Serial.print("Appending failled!");  
  }
  myFile.close();
}
boolean readtxt(){
  boolean haydata=false;
  myFile = SPIFFS.open(myFilePath, "r");
  wifiData="";
  while(myFile.available()) {
    //Serial.write(myFile.read());
    wifiData += (char)myFile.read();
  }
  Serial.println("\"");   
  //Serial.println("String completa" + wifiData);

  // Check file size
  Serial.print("File size: ");
  Serial.println(myFile.size());
  if(myFile.size()>=3) haydata = true;
  else haydata = false;
  myFile.close();
  return  haydata;
}
boolean wifiConnect(char* host,boolean caso, String ssidtxt ,String passtext) {
//#ifdef MYSSID
if(caso){
  Serial.println("Conectandose a una red");
  WiFi.mode(WIFI_AP_STA);
    //while(WiFi.waitForConnectResult() != WL_CONNECTED){
    for (int i = 0; i < 3; i++)
    {
      WiFi.begin(ssidtxt, passtext);
      Serial.println("WiFi failed, retrying.");
       if (WiFi.waitForConnectResult() == WL_CONNECTED) break;
     
    }
    if (WiFi.waitForConnectResult() != WL_CONNECTED)
    {
      writetxt("");
      Serial.println("No se pudo conectar");
      ESP.restart();
    }
     else
     {
       WiFi.softAPdisconnect(true);
      Serial.println(WiFi.localIP());
   return (WiFi.status() == WL_CONNECTED);
     }
      
      
    
  
  //#else
}
else{
  Serial.println("Creando Acces Point");
  WiFi.mode(WIFI_AP);
  //WiFi.begin();
  // WiFi.waitForConnectResult();
  while(!WiFi.softAP(ssid, password))
  {
   Serial.println(".");
    delay(1000);
  }
  Serial.print("Iniciado AP ");
  Serial.println(ssid);
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
  return WiFi.softAP(ssid, password);
  }
//#endif
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
    int cmd = (filename.indexOf("spiffs") > -1) ? U_FS : U_FLASH;//U_FS : U_FLASH;
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
    Serial.println("UpdateError:" + String(len) +"-----"+ String(Update.write(data, len) ));
#ifdef ESP8266
  } else {
    Serial.printf("Progress: %d%%\n", (Update.progress()*100)/Update.size());
    // digitalWrite(redLed,LOW);
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
 
 #pragma region //peticiones
 ///////////////imagenes html///////////////////////////
 
#pragma endregion
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


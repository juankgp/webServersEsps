#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <Updater.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include <FS.h>
#include <EEPROM.h>


File myFile;
const char* myFilePath = "/wifi.txt";
String estPuerta = "";
String estEspera = "";
String estSens = "";
const char* PARAM_INPUT_1 = "state";
const char* PARAM_INPUT_2 = "value";
int addr = 0;
int cambioluz;
AsyncWebServer server(80);
//DNSServer dns;
size_t content_len;
String wifiData="";
int topen=0;
int twait=0;
#define MYSSID "FLA GUTIERREZ"
#define PASSW "KYARALIBY"
char* ssid = "esp8285";
char* password = "esp8285pass";
IPAddress ip(192,168,43,155);  //Node static IP
IPAddress gateway(192,168,43,1);
IPAddress subnet(255,255,255,0);
#define rel 13
#define sens 5
#define lect 12
int value=0;
int valsens=0;
unsigned long TiempoAhora = 0;

void tiempoSenso(int sec){
  TiempoAhora = millis();
  Serial.println("Tiempo En Cierre");
    estEspera = "led-green-on";
    estPuerta = "led-red-on";
     while(millis() < TiempoAhora+sec*1000)
     {
        valsens = digitalRead(sens);
        if(!valsens)
          {
            estSens = "led-green-on";
            Serial.println("Detectado En Cierre");
           digitalWrite(rel,HIGH); 
           TiempoAhora = millis();
          }
          delay(100);
     }
     
     digitalWrite(rel,LOW);
    // espere [periodo] milisegundos
}

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
    int cmd = (filename.indexOf("spiffs") > -1) ? 0x6B000 : U_FLASH;//U_FS : U_FLASH;
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
  else if(var == "STATE"){
 
    String data1 = "Tiempo Abierto: " + String(topen) + " Tiempo Espera en Cierre: " + String(twait);
    return data1;
  }
  return String();
}


void setup() {
   Serial.begin(115200);
  pinMode(rel,OUTPUT);
  pinMode(sens,INPUT);
  pinMode(lect,INPUT);
  #pragma region //Configuracio
 
  EEPROM.begin(512);
  topen = EEPROM.read(addr);
  twait = EEPROM.read(addr+1);
  char host[16];
  Serial.println("Iniciando");

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
//     digitalWrite(redLed,LOW);
     return;
   }
   //digitalWrite(redLed,HIGH);
  MDNS.begin(host);
  if(webInit()) MDNS.addService("http", "tcp", 80);
 
   

  #pragma endregion
 ///////////////imagenes html///////////////////////////
  server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logo.png", "image/png");
  });
server.on("/logoVDT16", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/logoVDT16.png", "image/png");
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
  server.on("/onrly1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Prende RLY1");
    //digitalWrite(rly1,HIGH);
    delay(5000);
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offrly1", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("apaga RLY1");
    //digitalWrite(rly1,LOW);
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
   server.on("/onrly2", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("Prende RLY2");
    //digitalWrite(rly2,HIGH);
    //digitalWrite(redLed,HIGH);
     delay(5000);
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
  server.on("/offrly2", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("apaga RLY2");
    //digitalWrite(rly2,LOW);
     //digitalWrite(redLed,LOW);
    request->send(SPIFFS, "/config.html", String(), false, processor);
  });
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
  server.on("/timeuc", HTTP_GET, [](AsyncWebServerRequest *request){
    String cadena = "Hola Mundo JK!!!!!!!!!!!1";
  request->send_P(200,"text/plain",cadena.c_str());
  });
  server.on("/indicadorPuerta", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    cadena = estPuerta;
  request->send_P(200,"text/plain",cadena.c_str());
  });
  server.on("/indicadorEspera", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    cadena = estEspera;
  request->send_P(200,"text/plain",cadena.c_str());
  });
  server.on("/indicadorSensor", HTTP_GET, [](AsyncWebServerRequest *request){ 
    String cadena;
    cadena = estSens;
  request->send_P(200,"text/plain",cadena.c_str());
  });

////funcion para capturar las credenciales de wifi

server.on("/timedata", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
    String wifiData = String(inputMessage);
    Serial.println(wifiData);  
    int septopen = wifiData.indexOf(":");
    int septwait = wifiData.indexOf(":",septopen+1);
    String topentext = wifiData.substring(0,septopen);
    String twaittext = wifiData.substring(septopen+1,septwait);
    Serial.println("Datos Wifi" + topentext + "----" + twaittext);
    topen = topentext.toInt();
    twait = twaittext.toInt(); 
      EEPROM.write(addr, topen);
      EEPROM.write(addr+1, twait);
      EEPROM.commit();
    }
    else {
      inputMessage = "No message sent";
    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });


server.on("/ini", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
     // delay(1000);
     // Serial.println("Llego value");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
     
    }
    else {
      inputMessage = "No message sent";

    }
   
  
   request->send(SPIFFS, "/config.html", String(), false, processor);
  });

  //*****************************************************

  
}

void loop(){

 

value=digitalRead(lect);
 valsens = digitalRead(sens);
 estEspera = "led-red-on";
 estPuerta = "led-red-on";
  if(!valsens)
  {
    estSens = "led-green-on";
  }
  else
  {
    estSens = "led-red-on";
  }
  
  // Serial.println(value);
   if (!value)
  {
    Serial.println("Activadoo");
    estPuerta = "led-green-on";
    digitalWrite(rel,HIGH);
    delay(topen*1000);
    estPuerta = "led_red-on";
    valsens = digitalRead(sens);
    if(!valsens)
      {
        Serial.println("Sensor activado");
        estSens = "led-green-on";
        while (!valsens)
        {
          estSens = "led-green-on";
          //digitalWrite(rel,HIGH);
          valsens = digitalRead(sens);
          delay(100);
        }
        estSens = "led-red-on";
        delay(3000);        
        digitalWrite(rel,LOW);
        tiempoSenso(twait);
      }
      else
      {
        digitalWrite(rel,LOW);
        tiempoSenso(twait);
      }
      
  }

}


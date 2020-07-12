#include <Arduino.h>
//#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include <ESP8266mDNS.h>
#include <Updater.h>
#include <TimeLib.h>

//const char* ssid = "MiMix2s";
//const char* password = "wlanjuank1";
const char* ssid = "FLA GUTIERREZ";
const char* password = "KYARALIBY";
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
int totalMin = -1;
int totalSeg = -1;
bool firsttime=true;
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
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
 
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
String processorT(const String& var){
  //Serial.println(var);
  if(var == "BUTTONPLACEHOLDER"){
    String buttons = "";
    String outputStateValue = outputState();
    buttons+= "<p><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label></p>";
    return buttons;
  }
  else if(var == "TIMERVALUE"){
    return timerSliderValue;
  }
  
  return String();
}

String processor1(const String& var){
  //Serial.println(var);
  if(var == "TIEMPOVALUE"){
     Serial.println("En processor 1");
    // totalMin = timeValue.toInt() +minute();
    // totalSeg = second();
    // data = "Min Ingresado:" + timeValue + " Min actual: " + minute() +" Total: " + String(totalMin);
    // Serial.println(data);
    // return data;
   
      if(digitalRead(ledPin)){
        ledState = "ON";
      }
      else{
        ledState = "OFF";
      }
      Serial.print(ledState);
      return ledState;
  
    }
 
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
   char host[16];
  setTime(1,1,1,1,1,2020);
   
  // Connect to Wi-Fi
  pinMode(ledPin, OUTPUT);
  snprintf(host, 12, "ESP%08X", ESP.getChipId());
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  if(!SPIFFS.begin()){
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
  }

  // Print ESP32 Local IP Address
 MDNS.begin(host);
  if(webInit()) MDNS.addService("http", "tcp", 80);
  Serial.printf("Ready! Open http://%s.local in your browser\n", host);
  Serial.println(WiFi.localIP());

/////////////////////////
// Route for root / web page
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);    
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/ini", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.println("En ini");
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      
      timeValue = String(inputMessage);
      Serial.println("PARAM_INPUT2 "+ timeValue);
      digitalWrite(ledPin,HIGH);
      
    }
    else {
      inputMessage = "No message sent";
    }
   
    //request->send(200, "text/plain", "OK");
    Serial.println("MAndo pagina timer");
    //String sprog = processor1("TIEMPOVALUE");
    //Serial.println(sprog);
    request->send(SPIFFS, "/index.html", String(), false, processor1);
  });
////////////////////////
/////11111111111111111111
// Route for root / web page
  server.on("/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/timer.html", String(), false, processorT);
  });

  // Send a GET request to <ESP_IP>/update?state=<inputMessage>
  server.on("/update1", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/update?state=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      Serial.println("PARAM_INPUT1 "+ String(inputMessage));
      digitalWrite(ledPin, inputMessage.toInt());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  
  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      Serial.println("PARAM_INPUT2 "+ String(inputMessage));
      timerSliderValue = inputMessage;
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });

  /////////mio
  // Send a GET request to <ESP_IP>/tiempo?value=<inputMessage>
 /* server.on("/tiempo", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      Serial.println("PARAM_INPUT2 "+ String(inputMessage));
      timeValue = inputMessage;
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });*/
  ///////////
  
///////111111111111111111
  // Route for root / web page

}
 
void loop(){
  
  // if(minute() >= totalMin && second() >= totalSeg){
  //   digitalWrite(ledPin,LOW);
   
  // }
}
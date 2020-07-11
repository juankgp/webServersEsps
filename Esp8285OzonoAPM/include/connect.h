#include <Updater.h>
#include <ESP8266mDNS.h>
#include <Hash.h>
#include "requests.h"
char* ssid = "esp8285";
char* password = "esp8285pass";
String wifiData="";

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


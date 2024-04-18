/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

// Import required libraries
#ifdef ESP32
  #include <WiFi.h>
  #include <ESPAsyncWebServer.h>
  #include <SPIFFS.h>
#else
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
  #include <Hash.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <FS.h>
#endif
#include "DFRobot_SHT20.h"


DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

// Replace with your network credentials
const char* ssid = "AndroidAP394B";
const char* password = "mahdimahdi80";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readSHT20Temperature() {
  // Read temperature as Celsius (the default)
  float t = sht20.readTemperature();
  // Convert temperature to Fahrenheit
  if (isnan(t)) {    
    Serial.println("Failed to read from SHT20 sensor!");
    return "";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readSHT20Humidity() {
  float h = sht20.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from SHT20 sensor!");
    return "";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  
  sht20.initSHT20();

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readSHT20Temperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readSHT20Humidity().c_str());
    Serial.println(readSHT20Humidity().c_str());
  });


  // Start server
  server.begin();
}
 
void loop(){
  
}

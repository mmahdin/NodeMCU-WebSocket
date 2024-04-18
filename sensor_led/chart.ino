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
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <WiFi.h>
#else
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Hash.h>
#endif
#include <FS.h>

#include "DFRobot_SHT20.h"

DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

// Replace with your network credentials
const char *ssid = "AndroidAP394B";
const char *password = "mahdimahdi80";

// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readSHT20Temperature() {
  // Read temperature as Celsius (the default)
  float t = sht20.readTemperature();
  // Convert temperature to Fahrenheit
  if (isnan(t)) {
    Serial.println("Failed to read from SHT20 sensor!");
    return "";
  } else {
    Serial.println(t);
    return String(t);
  }
}

String readSHT20Humidity() {
  float h = sht20.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from SHT20 sensor!");
    return "";
  } else {
    Serial.println(h);
    return String(h);
  }
}

// Replaces placeholder with LED state value
String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE") {
    if (digitalRead(D4)) {
      ledState = "ON";
    } else {
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  } else if (var == "TEMPERATURE") {
    return readSHT20Temperature();
  } else if (var == "HUMIDITY") {
    return readSHT20Humidity();
  }
  return String();
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  sht20.initSHT20();

  // Initialize SPIFFS
  if (!SPIFFS.begin()) {
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
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(SPIFFS, "/index.html");
  // });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(D4, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    digitalWrite(D4, LOW);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readSHT20Temperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readSHT20Humidity().c_str());
  });

  // Start server
  server.begin();
}

void loop() {}

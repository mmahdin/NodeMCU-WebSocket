/*********
  Rui Santos
  Complete instructions at
https://RandomNerdTutorials.com/esp8266-nodemcu-websocket-server-sensor/

  Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files. The above copyright notice
and this permission notice shall be included in all copies or substantial
portions of the Software.
*********/

#include <Arduino.h>
#include <Arduino_JSON.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "DFRobot_SHT20.h"
#include "LittleFS.h"

// Replace with your network credentials
const char *ssid = "AndroidAP394B";
const char *password = "mahdimahdi80";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

bool ledState = 0;

// Create a sensor object
DFRobot_SHT20 sht20(&Wire, SHT20_I2C_ADDR);

// Get Sensor Readings and return JSON object
String getSensorReadings() {
  float t = sht20.readTemperature();
  float h = sht20.readHumidity();
  readings["temperature"] = String(t);
  readings["humidity"] = String(h);
  readings["state"] = String(ledState);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  } else {
    Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("\nConnecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String sensorReadings) { ws.textAll(sensorReadings); }

int findToggle(uint8_t *data, size_t dataSize, const char *toggleString) {
  size_t toggleLen = strlen(toggleString);

  for (size_t i = 0; i < dataSize - toggleLen + 1; i++) {
    int match = 1;
    for (size_t j = 0; j < toggleLen; j++) {
      if (data[i + j] != toggleString[j]) {
        match = 0;
        break;
      }
    }
    if (match) {
      return 1;  // Found "toggle" in data
    }
  }

  return 0;  // "toggle" not found in data
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  size_t dataSize = sizeof(data);
  const char *toggleString = "toggle";
  Serial.println(findToggle(data, dataSize, toggleString));
  if (info->final && info->index == 0 && info->len == len &&
      info->opcode == WS_TEXT) {
    // data[len] = 0;
    // String message = (char*)data;
    //  Check if the message is "getReadings"
    if (findToggle(data, dataSize, toggleString)) {
      ledState = !ledState;
      digitalWrite(D4, ledState);
      String sensorReadings = getSensorReadings();
      notifyClients(sensorReadings);
      Serial.println("toggle");
    }
    if (strcmp((char *)data, "getReadings") == 0) {
      String sensorReadings = getSensorReadings();
      notifyClients(sensorReadings);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
             AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(),
                    client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);
  sht20.initSHT20();
  pinMode(D4, OUTPUT);
  digitalWrite(D4, 0);
  initWiFi();
  initFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    String sensorReadings = getSensorReadings();
    // Serial.println(sensorReadings);
    notifyClients(sensorReadings);

    lastTime = millis();
  }
  digitalWrite(D4, ledState);
  ws.cleanupClients();
}

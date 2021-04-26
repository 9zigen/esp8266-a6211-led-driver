#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "app_schedule.h"
#include "settings.h"
#include "webui.h"
#include "led.h"
#include "Network.h"
#include "mqtt.h"
#include "status.h"

#ifndef OTA_ONLY
AsyncWebServer server(80);
#endif

void setup() {
  Serial.begin(115200);

  /* Load all CONFIG from eeprom */
  CONFIG.init();

  /* Setup and start WiFi */
  NETWORK.init();

  /* OTA Setup */
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();

  /* Led setup */
  LED.init();

#ifndef OTA_ONLY
  /* Scheduler setup */
  SCHEDULE.init();

  /* MQTT setup */
  initMqtt();

  /* Initiate WebUi and attach your Async webserver instance */
  WEBUI.init(server);
  server.begin();
#endif
}

void loop() {
  /* OTA Loop */
  ArduinoOTA.handle();

  /* check connection loop */
  NETWORK.loop();

#ifndef OTA_ONLY
  /* Schedule loop */
  SCHEDULE.loop();

  /* Device status */
  statusLoop();
#endif
}
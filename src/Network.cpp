/***
** Created by Aleksey Volkov on 2019-05-09.
***/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <NtpClientLib.h>
#include "settings.h"
#include "webui.h"
#include "led.h"
#include "Ticker.h"
#include "Network.h"
#include "mqtt.h"

#define NTP_TIMEOUT 1500

ESP8266WiFiMulti wifiMulti;
Ticker reconnect_timer;
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;
NTPSyncEvent_t ntpEvent;
bool ntpSyncEventTriggered = false;

/* NTP Sync Event */
void processSyncEvent (NTPSyncEvent_t ntpEvent) {
  switch (ntpEvent) {
    case noResponse:
      LOG_NETWORK("[NETWORK] NTP server not reachable \n");
      break;
    case timeSyncd:
      LOG_NETWORK("[NETWORK] Got NTP time: %s \n", NTP.getTimeDateString (NTP.getLastNTPSync ()).c_str());
      break;
    case invalidAddress:
      LOG_NETWORK("[NETWORK] Invalid NTP server address\n");
      break;
    case requestSent:
      break;
    case errorSending:
      LOG_NETWORK("[NETWORK] Error sending request\n");
      break;
    case responseError:
      LOG_NETWORK("[NETWORK] NTP response error\n");
      break;
  }
}

void onSTAGotIP(const WiFiEventStationModeGotIP& event) {
  LOG_NETWORK("[NETWORK] Wi-Fi Got IP: %s\n", WiFi.localIP().toString().c_str());
  LED.setMode(ONE_SHORT_BLINK);
  NETWORK.isConnected = true;
  NETWORK.stopAP();

  /* NTP Connect */
  NETWORK.startNtp = true;

  /* MQTT Connect */
  connectToMqtt();
}

void onSTADisconnect(const WiFiEventStationModeDisconnected& event) {
  LOG_NETWORK("[NETWORK] Wi-Fi Disconnected\n");
  LED.setMode(THREE_SHORT_BLINK);
  NETWORK.isConnected = false;

  /* NTP sync can be disabled to avoid sync errors */
  NTP.stop();

  /* Scan and connect to best network */
  NETWORK.connectSTA();
}

void NetworkClass::init() {
  LOG_NETWORK("[NETWORK] WiFi First Time Connecting to AP...\n");

  /* Start STA Mode with saved credentials */
  startSTA();

  /* Register events */
  gotIpEventHandler = WiFi.onStationModeGotIP(onSTAGotIP);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onSTADisconnect);

  NTP.onNTPSyncEvent ([](NTPSyncEvent_t event) {
    ntpEvent = event;
    ntpSyncEventTriggered = true;
  });

}

void NetworkClass::reloadSettings() {
  new_wifi_settings = true;
  new_mqtt_settings = true;
  check_wifi = true;
}

/* check WiFi connection in loop */
void NetworkClass::loop() {

  if (startNtp) {
    /* Setup NTP */
    int16_t tz_offset = CONFIG.getNtpOffset();
    auto tz           = (int8_t)(tz_offset / 60);
    auto tz_minutes   = (int8_t)(tz_offset % 60);
    NTP.setInterval (63);
    NTP.setNTPTimeout (NTP_TIMEOUT);
    NTP.begin (CONFIG.getNtpServerName(), tz, true, tz_minutes);

    startNtp = false;
  }

  if (ntpSyncEventTriggered) {
    processSyncEvent (ntpEvent);
    ntpSyncEventTriggered = false;
  }

  if (!check_wifi)
    return;

  /* update MQTT client config */
  if (new_mqtt_settings) {
    new_mqtt_settings = false;
    LOG_NETWORK("[NETWORK] MQTT New Settings Applied, connecting to Server...\n");
    initMqtt();
  }

  /* reconnect to AP requested */
  if (new_wifi_settings) {
    new_wifi_settings = false;
    LOG_NETWORK("[NETWORK] WiFi New Settings Applied, checking...\n");
    startSTA();
  }

  /* If STA still not connected */
  if (wifiMulti.run() != WL_CONNECTED) {
    isConnected = false;

    LOG_NETWORK("[NETWORK] WiFi Connect Timeout. WiFi Mode: %d \n", WiFi.getMode());

    if (WiFi.getMode() != WIFI_AP) {
      /* start AP */
      startAP();
    }
  } else {
    /* Wait 2 sec to repeat check state */
    connection_timer.once(2, std::bind(&NetworkClass::tik, this));
  }

  check_wifi = false;

}

void NetworkClass::tik() {
  check_wifi = true;
}

void NetworkClass::connectSTA() {
  /* Scan and connect to best WiFi network */
  if (wifiMulti.run() != WL_CONNECTED) {

    /* Wait 1000 msec to establish connection */
    connection_timer.once(10, std::bind(&NetworkClass::tik, this));
  }
}


void NetworkClass::startSTA() {
  /* Disable store WiFi config in SDK flash area */
  WiFi.persistent(false);

  /* Start WiFi in Station mode */
  WiFi.mode(WIFI_STA);

  for (uint8_t i = 0; i < MAX_NETWORKS; i++)
  {
    network_t * network = CONFIG.getNetwork(i);

    /* check if has wifi config */
    if (network->active) {
      wifiMulti.addAP((char *)&network->ssid, (char *)&network->password);
      LOG_NETWORK("[NETWORK] Add config to WiFi list: %s [******] \n", network->ssid);
    }
  }

  /* Scan and connect to best WiFi network */
  connectSTA();

}

void NetworkClass::startAP() {
  LOG_NETWORK("[NETWORK] WiFi Starting AP. \n");
  WiFi.disconnect(false);
  WiFi.mode(WIFI_AP);

  services_t * services = CONFIG.getService();
  auth_t * auth = CONFIG.getAuth();

  /* Start AP */
  WiFi.softAP(services->hostname, auth->password);
  LOG_NETWORK("[NETWORK] AP Started with name: %s and password: %s \n", services->hostname, auth->password);
  //LOG_NETWORK("[NETWORK] AP IP address: %s \n", WiFi.softAPIP().c_str());
}

void NetworkClass::stopAP() {

  /* Disable AP Mode */
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_STA);
  LOG_NETWORK("[NETWORK] Disabling AP. WiFi Mode: %d \n", WiFi.getMode());
}

NetworkClass NETWORK;
/***
** Created by Aleksey Volkov on 2019-05-09.
***/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include "settings.h"
#include "webui.h"
#include "led.h"
#include "Ticker.h"
#include "Network.h"
#include "mqtt.h"


ESP8266WiFiMulti wifiMulti;
WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

void onSTAGotIP(const WiFiEventStationModeGotIP& event) {
  Serial.printf("[NETWORK] Wi-Fi Got IP: %s\n", WiFi.localIP().toString().c_str());
  LED.setMode(ONE_SHORT_BLINK);
  NETWORK.isConnected = true;
  NETWORK.stopAP();

  /* MQTT Connect */
  connectToMqtt();
}

void onSTADisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.printf("[NETWORK] Wi-Fi Disconnected \n");
  LED.setMode(THREE_SHORT_BLINK);
  NETWORK.isConnected = false;
  NETWORK.startAP();
}

void NetworkClass::init() {
  Serial.println("[NETWORK] WiFi First Time Connecting to AP...\r\n");

  /* Start STA Mode with saved credentials */
  startSTA();

  /* Register events */
  gotIpEventHandler = WiFi.onStationModeGotIP(onSTAGotIP);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(onSTADisconnect);

}

void NetworkClass::reloadSettings() {
  new_wifi_settings = true;
  check_wifi = true;
}

/* check WiFi connection first 10 sec */
void NetworkClass::loop() {

  if (!check_wifi)
    return;

  /* reconnect to AP requested */
  if (new_wifi_settings) {
    new_wifi_settings = false;
    Serial.println("[NETWORK] WiFi New Settings Applied, connecting to AP...");
    startSTA();
    return;
  }

  /* If STA not connected in 10 sec */
  if (wifiMulti.run() != WL_CONNECTED) {
    isConnected = false;

#ifdef DEBUG_NETWORK
      Serial.printf("[NETWORK] WiFi Connect Timeout. WiFi Mode: %d \n", WiFi.getMode());
#endif

    if (WiFi.getMode() != WIFI_AP_STA) {
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

void NetworkClass::startSTA() {
  /* Disable store WiFi config in SDK flash area */
  WiFi.persistent(false);
  WiFi.disconnect(false);

  /* Start WiFi in Station mode */
  WiFi.mode(WIFI_STA);

  for (uint8_t i = 0; i < MAX_NETWORKS; i++)
  {
    network_t * network = CONFIG.getNetwork(i);

    /* check if has wifi config */
    if (network->active) {
      wifiMulti.addAP((char *)&network->ssid, (char *)&network->password);

#ifdef DEBUG_NETWORK
        Serial.printf("[NETWORK] Add config to WiFi list: %s [******] \n", network->ssid);
#endif
    }
  }

  /* Scan and connect to best WiFi network */
  if (wifiMulti.run() != WL_CONNECTED) {

    /* Wait 200 msec to establish connection */
    connection_timer.once_ms(200, std::bind(&NetworkClass::tik, this));
  }


}

void NetworkClass::startAP() {
#ifdef DEBUG_NETWORK
    Serial.println("[NETWORK] WiFi Starting AP.");
#endif

  WiFi.mode(WIFI_AP_STA);

  services_t * services = CONFIG.getService();
  auth_t * auth = CONFIG.getAuth();

  /* Start AP */
  WiFi.disconnect();
  WiFi.softAP(services->hostname, auth->password);

#ifdef DEBUG_NETWORK
    Serial.printf("[NETWORK] AP Started with name: %s and password: %s \r\n", services->hostname, auth->password);
#endif

  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("[NETWORK] AP IP address: %s \r\n", apIP.toString().c_str());
}

void NetworkClass::stopAP() {

  /* Disable AP Mode */
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_STA);

#ifdef DEBUG_NETWORK
    Serial.printf("[NETWORK] Disabling AP. WiFi Mode: %d \n", WiFi.getMode());
#endif
}

NetworkClass NETWORK;
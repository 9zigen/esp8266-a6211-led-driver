/***
** Created by Aleksey Volkov on 2019-05-16.
***/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "ArduinoJson.h"
#include <AsyncMqttClient.h>
#include "settings.h"
#include "schedule.h"
#include "mqtt.h"
#include "status.h"
#include "Network.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
bool mqtt_enabled = false;
bool isConnected = false;

void onMqttConnect(bool sessionPresent) {
  Serial.printf("[MQTT] Connected to server. \n");
  isConnected = true;

  char buf[128];

  /* Subscribe
   * [hostname]/channel/[channel_number]/set
   * */
  for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d/set", CONFIG.getHostname(), i);

    /* subscribe to topic QoS 0 */
    if (!mqttClient.subscribe(buf, 0))
      Serial.printf("[MQTT] ERROR Subscribe to topic: %s\n", buf);
  }

}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf("[MQTT] Disconnected from server. Reason: %d\n", (int)reason);
  isConnected = false;
  mqttReconnectTimer.once(2, connectToMqtt);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("[MQTT] Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("[MQTT] Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/* topic: LED_11324571/channel/0/set
 * payload: decimal 0-255
 * */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
#ifdef DEBUG_MQTT
  Serial.printf("[MQTT] Publish received. topic: %s, qos: %d, dup: %d, retain: %d, len: %d, index: %d, total: %d\n",
      topic, properties.qos, properties.dup, properties.retain, len, index, total);
#endif

  char buf[128];
  uint32_t led_id = 0;
  uint8_t duty = 0;
  sscanf(topic, "%s/channel/%u/set", buf, &led_id);

  duty = (uint8_t) atoi(payload);

#ifdef DEBUG_MQTT
  Serial.printf("[MQTT] LED#: %d Duty: %d\n", led_id, duty);
#endif

  SCHEDULE.setChannelDuty(duty, (uint8_t) led_id);

}

void onMqttPublish(uint16_t packetId) {
  Serial.println("[MQTT] Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void initMqtt() {
  services_t * services = CONFIG.getService();
  if (services->enable_mqtt_service) {
    mqtt_enabled = true;
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);

    /* Load config from eeprom */
    mqttClient.setServer(IPAddress(services->mqtt_ip_address), services->mqtt_port);
    mqttClient.setClientId(services->hostname);

    LOG_MQTT("[MQTT] Server: %s Port: %d Client ID: %s\n", IPAddress(services->mqtt_ip_address).toString().c_str(), services->mqtt_port, services->hostname);

    if ( (strlen(services->mqtt_user) > 0) && (strlen(services->mqtt_password) > 0) ) {
      LOG_MQTT("[MQTT] Server: %s Port: %d Client ID: %s\n", IPAddress(services->mqtt_ip_address).toString().c_str(), services->mqtt_port, services->hostname);
      mqttClient.setCredentials(services->mqtt_user, services->mqtt_password);
    }
  }
}

void connectToMqtt() {
  /* return if MQTT Disabled */
  if (!mqtt_enabled)
    return;

  /* return if not connected to AP */
  if (!NETWORK.isConnected)
    return;

  LOG_MQTT("[MQTT] Connecting...\n");
  mqttClient.connect();
}

/* Publish led channels current duty
 * [hostname]/channel/[channel_number]
 * */
void publishLedStatusToMqtt() {
  if (!mqtt_enabled || !isConnected)
    return;

  char buf[128];
  char message_buf[10];

  /* Publish */
  for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d", CONFIG.getHostname(), i);

    /* make message string */
    snprintf(message_buf, 128, "%d", SCHEDULE.getChannelDuty(i));

    LOG_MQTT("[MQTT] Publish led status.\n");

    /* publish led status to topic QoS 0, Retain */
    if (!mqttClient.publish(buf, 0, true, message_buf, strlen(message_buf)))
      LOG_MQTT("[MQTT] ERROR Publish to topic: %s\n", buf);
  }
}

/* Publish device status
 * [hostname]/status
 * */
void publishDeviceStatusToMqtt() {
  if (!mqtt_enabled || !isConnected)
    return;

  char buf[128];
  char message_buf[512];

  /* make topic string */
  snprintf(buf, 128, "%s/status", CONFIG.getHostname());

  /* make message json string */
  const size_t capacity = JSON_OBJECT_SIZE(7) + 256;
  DynamicJsonDocument doc(capacity);
  JsonObject root = doc.to<JsonObject>();

  /* get device info */
  status_t * device_info = getDeviceInfo();

  root["chip_id"]     = device_info->chip_id;
  root["free_heap"]   = device_info->free_heap;
  root["cpu_freq"]    = device_info->cpu_freq;
  root["vcc"]         = device_info->vcc;
  root["wifi_mode"]   = device_info->wifi_mode;
  root["ip_address"]  = device_info->ip_address;
  root["mac_address"] = device_info->mac_address;

  serializeJson(doc, message_buf, 512);
  LOG_MQTT("[MQTT] Publish device status.\n");

#ifdef DEBUG_MQTT_JSON
  serializeJson(doc, Serial);
  Serial.println();
#endif

  /* publish led status to topic QoS 0, Retain */
  if (!mqttClient.publish(buf, 0, true, message_buf, strlen(message_buf)))
    LOG_MQTT("[MQTT] ERROR Publish to topic: %s\n", buf);
}
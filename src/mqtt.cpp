/***
** Created by Aleksey Volkov on 2019-05-16.
***/

#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include "ArduinoJson.h"
#include <AsyncMqttClient.h>
#include <NtpClientLib.h>
#include "settings.h"
#include "schedule.h"
#include "mqtt.h"
#include "status.h"
#include "Network.h"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
bool mqtt_enabled = false;
bool isConnected = false;
uint8_t mqtt_qos = 0;

void onMqttConnect(bool sessionPresent) {
  Serial.printf("[MQTT] Connected to server. \n");
  isConnected = true;

  char buf[128];

  /* Subscribe to Set Duty topic
   * [hostname]/channel/[channel_number]/set
   * */
  for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d/set", CONFIG.getHostname(), i);
    LOG_MQTT("[MQTT] Subscribe to topic: %s\n", buf);

    /* subscribe to topic QoS */
    if (!mqttClient.subscribe(buf, mqtt_qos))
      Serial.printf("[MQTT] ERROR Subscribe to topic: %s\n", buf);
  }

  /* Subscribe to Switch topic
   * [hostname]/channel/[channel_number]/switch
   * */
  for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d/switch", CONFIG.getHostname(), i);
    LOG_MQTT("[MQTT] Subscribe to topic: %s\n", buf);

    /* subscribe to topic QoS */
    if (!mqttClient.subscribe(buf, mqtt_qos))
      Serial.printf("[MQTT] ERROR Subscribe to topic: %s\n", buf);
  }

  /* Subscribe to Brightness topic
   * [hostname]/brightness/set
   * */

  /* make topic string */
  snprintf(buf, 128, "%s/brightness/set", CONFIG.getHostname());
  LOG_MQTT("[MQTT] Subscribe to topic: %s\n", buf);

  /* subscribe to topic QoS */
  if (!mqttClient.subscribe(buf, mqtt_qos))
    Serial.printf("[MQTT] ERROR Subscribe to topic: %s\n", buf);

}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.printf("[MQTT] Disconnected from server. Reason: %d\n", (int)reason);
  isConnected = false;
  mqttReconnectTimer.once(2, connectToMqtt);
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  LOG_MQTT("[MQTT] Subscribe acknowledged:\n");
  LOG_MQTT("\tpacketID %d QoS: %d\n", packetId, qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  LOG_MQTT("[MQTT] Unsubscribe acknowledged:\n");
  LOG_MQTT("\tpacketID: %d\n", packetId);
}

/* Topic examples:
 * Brightness: LED_11324571/brightness        | payload: 0-255
 * Set Duty:   LED_11324571/channel/0/set     | payload: 0-255
 * Switch:     LED_11324571/channel/0/switch  | payload: 0-1
 * */
void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  LOG_MQTT("[MQTT] Publish received:\n");
  LOG_MQTT("\ttopic: %s, qos: %d, dup: %d, retain: %d, len: %d, index: %d, total: %d\n",
      topic, properties.qos, properties.dup, properties.retain, len, index, total);

  char buf[128];

  /* check brightness topic */
  snprintf(buf, 128, "%s/brightness/set", CONFIG.getHostname());

  /* Brightness received */
  if (strncmp(topic, buf, 128) == 0) {
    uint8_t brightness = atoi(payload) & 0xff;

    LOG_MQTT("[MQTT] Set Brightness: %d\n", brightness);
    SCHEDULE.setBrightness(brightness);
    return;
  }

  /* check switch or set command */
  uint32_t channel = 0;
  char command[7];
  char scan[128];

  /* "sscanf" template example: LED_11324571/channel/%u/%6s */
  snprintf(scan, 128, "%s/channel/%%u/%%6s", CONFIG.getHostname());
  sscanf(topic, scan, &channel, command);

  LOG_MQTT("[MQTT] Command: %s payload: %s\n", command, payload);

  if (strncmp(command, "set", 3) == 0) {
    /* Set command */
    uint8_t duty = atoi(payload) & 0xff;
    LOG_MQTT("[MQTT] Set Duty: %u Channel: %u\n", duty, channel);
    SCHEDULE.setChannelDuty(channel, duty);

  } else if (strncmp(command, "switch", 6) == 0) {
    /* switch command */
    uint8_t state = atoi(payload) & 0xff;
    LOG_MQTT("[MQTT] Set State Channel: %u to %u\n", channel, state);
    if (state) {
      /* Set max Duty */
      SCHEDULE.setChannelDuty(channel, MAX_DUTY);
    } else {
      /* Off channel */
      SCHEDULE.setChannelDuty(channel, 0);
    }
  }

}

void onMqttPublish(uint16_t packetId) {
  LOG_MQTT("[MQTT] Publish acknowledged:\n");
  LOG_MQTT("\tpacketID: %d\n", packetId);
}

void initMqtt() {
  services_t * services = CONFIG.getService();
  if (services->enable_mqtt) {
    mqtt_enabled = true;
    mqtt_qos = services->mqtt_qos;
    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);

    /* Load config from eeprom */
    mqttClient.setServer(IPAddress(services->mqtt_server), services->mqtt_port);
    mqttClient.setClientId(services->hostname);

    LOG_MQTT("[MQTT] Server: %s Port: %d Client ID: %s\n", IPAddress(services->mqtt_server).toString().c_str(), services->mqtt_port, services->hostname);

    if ( (strlen(services->mqtt_user) > 0) && (strlen(services->mqtt_password) > 0) ) {
      mqttClient.setCredentials(services->mqtt_user, services->mqtt_password);
      LOG_MQTT("[MQTT] User:%s Password:%s\n", services->mqtt_user, services->mqtt_password);
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

/* Publish all channels brightness */
void publishBrightness() {
  if (!mqtt_enabled || !isConnected)
    return;

  char buf[128];
  char message_buf[10];

  /* make topic string */
  snprintf(buf, 128, "%s/brightness", CONFIG.getHostname());

  /* make message string */
  snprintf(message_buf, 128, "%d", SCHEDULE.getBrightness());
  LOG_MQTT("[MQTT] Publish Brightness: %u\n", SCHEDULE.getBrightness());

  /* publish led status to topic QoS 0, Retain */
  if (!mqttClient.publish(buf, mqtt_qos, true, message_buf, strlen(message_buf)))
    LOG_MQTT("[MQTT] ERROR Publish to topic: %s\n", buf);
}

/* Publish led channels current duty
 * [hostname]/channel/[channel_number]
 * */
void publishChannelDuty() {
  if (!mqtt_enabled || !isConnected)
    return;

  char buf[128];
  char message_buf[10];

  /* Publish */
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d", CONFIG.getHostname(), i);

    /* make message string */
    snprintf(message_buf, 128, "%d", SCHEDULE.getChannelDuty(i));
    LOG_MQTT("[MQTT] Publish Channel: %u duty: %u.\n", i, SCHEDULE.getChannelDuty(i));

    /* publish led status to topic QoS 0, Retain */
    if (!mqttClient.publish(buf, mqtt_qos, true, message_buf, strlen(message_buf)))
      LOG_MQTT("[MQTT] ERROR Publish to topic: %s\n", buf);
  }
}

/* Publish led channels current state
 * [hostname]/channel/[channel_number]/state
 * */
void publishChannelState() {
  if (!mqtt_enabled || !isConnected)
    return;

  char buf[128];
  char message_buf[10];

  /* Publish */
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* make topic string */
    snprintf(buf, 128, "%s/channel/%d/state", CONFIG.getHostname(), i);

    /* make message string */
    uint8_t state = 0;
    if (SCHEDULE.getChannelDuty(i) > 0) {
      state = 1;
    }

    snprintf(message_buf, 128, "%d", state);
    LOG_MQTT("[MQTT] Publish Channel: %u state: %u.\n", i, state);

    /* publish led status to topic QoS 0, Retain */
    if (!mqttClient.publish(buf, mqtt_qos, true, message_buf, strlen(message_buf)))
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
  const size_t capacity = JSON_OBJECT_SIZE(8) + 256;
  DynamicJsonDocument doc(capacity);
  JsonObject root = doc.to<JsonObject>();

  /* get device info */
  status_t * device_info = getDeviceInfo();

  root["up_time"]     = NTP.getUptimeString();
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
  if (!mqttClient.publish(buf, mqtt_qos, true, message_buf, strlen(message_buf)))
    LOG_MQTT("[MQTT] ERROR Publish to topic: %s\n", buf);
}
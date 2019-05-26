/***
** Created by Aleksey Volkov on 2019-04-04.
***/

#include "AsyncWebSocket.h"
#include "Arduino.h"
#include <functional>
#include "ArduinoJson.h"
#include "settings.h"
#include "webui.h"
#include "schedule.h"
#include "webpage.h"
#include "Network.h"
#include "status.h"

AsyncWebSocket ws("/ws");

// Handle Websocket Requests
void WEBUIClass::onWsEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len) {
  if(type == WS_EVT_CONNECT){
#ifdef DEBUG_WEB
      Serial.println("[WEBSOCKET] Client connection received");
#endif
  } else if(type == WS_EVT_DISCONNECT){
#ifdef DEBUG_WEB
      Serial.println("[WEBSOCKET] Client disconnected");
#endif
  } else if(type == WS_EVT_DATA){
    auto *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len) {
      String message = "";
      if (info->opcode == WS_TEXT) {
        for (size_t i=0; i<info->len; i++) message += (char)data[i];
      } else {
        char buff[3];
        for (size_t i=0; i<info->len; i++) {
          sprintf(buff, "%02x ", (uint8_t) data[i]);
          message += buff ;
        }
      }

#ifdef DEBUG_WEB_JSON
        Serial.println("[WEBSOCKET] Message Received: "+message);
#endif

      StaticJsonDocument<2000> doc;
      DeserializationError err = deserializeJson(doc, message);
      if (err) {
#ifdef DEBUG_WEB
          Serial.println(F("deserializeJson() failed: "));
          Serial.println(err.c_str());
#endif
      } else {
        JsonObject object = doc.as<JsonObject>();
        const char * command      = object["command"];
        const char * successJson  = "{\"sucess\":true}";;
        const char * pongJson     = "{\"response\":\"pong\"}";;

        if(command != nullptr) {
#ifdef DEBUG_WEB
          Serial.printf("[WEBSOCKET] Got %s Command from Client %u \r\n", command, client->id());
#endif

          char response[1024];
          memset(response, 0, 1024);

          if (strncmp(command, "ping", 4) == 0) {                 /* Ping */
            ws.text(client->id(), pongJson);

          } else if (strncmp(command, "erase", 5) == 0) {          /* Erase EEPROM */
            CONFIG.erase();
            ws.text(client->id(), successJson);
            /* reboot */
            ESP.restart();

          } else if (strncmp(command, "getStatus", 9) == 0) {      /* Status */
            WEBUI.statusJson((char *) &response, sizeof(response));
            ws.text(client->id(), response);

          } else if (strncmp(command, "getSchedule", 11) == 0) {   /* Schedule */
            WEBUI.scheduleJson((char *) &response, sizeof(response));
            ws.text(client->id(), response);

          } else if (strncmp(command, "getNetworks", 11) == 0) {   /* Networks */
            WEBUI.networksJson((char *) &response, sizeof(response));
            ws.text(client->id(), response);

          } else if (strncmp(command, "getDuty", 7) == 0) {        /* Current led duty */
            WEBUI.dutyJson((char *) &response, sizeof(response));
            ws.text(client->id(), response);

          } else if (strncmp(command, "getSettings", 11) == 0) {   /* Settings (Led color + NTP, MQTT) */
            WEBUI.settingsJson((char *) &response, sizeof(response));
            ws.text(client->id(), response);

          } else if (strncmp(command, "setSettings", 11) == 0) {   /* New Services and Led Settings */

            /* Parse Services config */
            JsonObject services = object["services"];

            /* get point to current services config (eeprom cache) */
            services_t *service = CONFIG.getService();
            strlcpy(service->hostname, services["hostname"] | "", 20);
            strlcpy(service->ntp_server_name, services["ntp_server_name"] | "es.pool.ntp.org", 20);
            strlcpy(service->mqtt_user, services["mqtt_user"] | "", 16);
            strlcpy(service->mqtt_password, services["mqtt_password"] | "", 16);
            WEBUI.stringToIP(services["mqtt_ip_address"], service->mqtt_ip_address);
            service->mqtt_port           = services["mqtt_port"].as<int>();
            service->utc_offset_minutes  = services["utc_offset_minutes"].as<int>();
            service->ntp_dst             = services["ntp_dst"].as<boolean>();
            service->enable_ntp_service  = services["enable_ntp_service"].as<boolean>();
            service->enable_mqtt_service = services["enable_mqtt_service"].as<boolean>();

            /* Parse New LED config */
            JsonArray leds = object["leds"];

            for (uint8_t i = 0; i < leds.size(); ++i) {
              if (i < MAX_LED_CHANNELS) {

                /* get point to current led config (eeprom cache) */
                led_t *_led = CONFIG.getLED(i);

                /* update value in (eeprom cache) */
                strlcpy(_led->color, leds[i]["color"] | "#DDEFFF", 8);
                _led->default_duty = leds[i]["default_duty"].as<int>();

              }
            }

            /* Store EEPROM settings (sync cache and eeprom) */
            CONFIG.setSettings();

            ws.text(client->id(), successJson);

          } else if (strncmp(command, "setNetworks", 11) == 0) {    /* New WiFi Settings */
            /* Reset network cache */
            for (uint8_t i = 0; i < MAX_NETWORKS; i++) {
              network_t *network = CONFIG.getNetwork(i);
              network->active = false;
            }

            /* Parse New NETWORK config */
            JsonArray networks = object["networks"];
            for (uint8_t i = 0; i < networks.size(); i++) {
              if (i < MAX_NETWORKS) {
                /* get point to current network config (eeprom cache) */
                network_t *network = CONFIG.getNetwork(i);

                strlcpy(network->ssid, networks[i]["ssid"] | " ", 32);
                strlcpy(network->password, networks[i]["password"] | " ", 32);

                /* split ip address to 4x uint8_t */
                WEBUI.stringToIP(networks[i]["ip_address"], network->ip_address);
                WEBUI.stringToIP(networks[i]["mask"], network->mask);
                WEBUI.stringToIP(networks[i]["gateway"], network->gateway);
                WEBUI.stringToIP(networks[i]["dns"], network->dns);

                network->dhcp = networks[i]["dhcp"].as<bool>();
                network->active = true;

#ifdef DEBUG_WEB
                  Serial.printf("[WEBSOCKET] ssid: %s password: %s\r\n", network->ssid, network->password);
#endif
              }
            }

            /* Store EEPROM settings (sync cache and eeprom) */
            CONFIG.setSettings();

            /* Reload WiFi settings */
            NETWORK.reloadSettings();

            ws.text(client->id(), successJson);

          } else if (strncmp(command, "setDuty", 7) == 0) {         /* Led Duty */
            JsonArray duty = object["duty"];
            for (uint8_t i = 0; i < duty.size(); i++) {
              if (i < MAX_LED_CHANNELS) {
                uint8_t new_duty = duty[i];
                SCHEDULE.setChannelDuty(new_duty, i);

#ifdef DEBUG_WEB
                uint8_t old_duty = SCHEDULE.getChannelDuty(i);
                Serial.printf("[WEBSOCKET] Led Duty: new: %d, old: %d\r\n", new_duty, old_duty);
#endif
              }
            }

            ws.text(client->id(), successJson);

          } else if (strncmp(command, "setSchedule", 11) == 0) {    /* Schedule */
            /* Reset schedule cache */
            for (uint8_t i = 0; i < MAX_SCHEDULE; ++i) {
              schedule_t *schedule = CONFIG.getSchedule(i);
              schedule->active = false;
            }

            /* Parse New Schedule config */
            JsonArray schedules = object["schedule"];
            for (uint8_t i = 0; i < schedules.size(); ++i) {
              if (i < MAX_SCHEDULE) {
                /* get point to current schedule config (eeprom cache) */
                schedule_t *schedule = CONFIG.getSchedule(i);

                /* update value in (eeprom cache) */
                schedule->enabled     = schedules[i]["enabled"].as<boolean>();
                schedule->time_hour   = schedules[i]["time_hour"].as<int>();
                schedule->time_minute = schedules[i]["time_minute"].as<int>();
                schedule->active      = true;

#ifdef DEBUG_WEB
                Serial.printf("[WEBSOCKET] Schedule: %d:%d duty size %d\r\n", schedule->time_hour, schedule->time_minute, schedules[i]["duty"].size());
#endif

                for (uint8_t j = 0; j < schedules[i]["duty"].size(); ++j) {
                  if (j < MAX_LED_CHANNELS)
                  {
                    schedule->led_duty[j] = schedules[i]["duty"][j].as<int>();
                  }
                }
              }
            }

            /* Store EEPROM settings (sync cache and eeprom) */
            CONFIG.setSettings();

            ws.text(client->id(), successJson);

          } else if (strncmp(command, "reboot", 6) == 0) {
            ws.text(client->id(), successJson);
            ESP.restart();

          }
        } else {
#ifdef DEBUG_WEB
          Serial.println("[WEBSOCKET] Invalid Command");
#endif
        }
      }
    }
  }
}


/* Public */

void WEBUIClass::init(AsyncWebServer& server){

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding","gzip");
    request->send(response);
  });

  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request){
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding","gzip");
    request->send(response);
  });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request){
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding","gzip");
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding","gzip");
    request->send(response);
  });

  server.on("/about", HTTP_GET, [](AsyncWebServerRequest *request){
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding","gzip");
    request->send(response);
  });

  server.addHandler(&ws);

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
}

/* Private */

/* Return string from uint8_t[4] array. */
String WEBUIClass::ipAddressToString(uint8_t *ip) {
  char buf[15];
  sprintf(buf, "%03d.%03d.%03d.%03d", ip[0], ip[1], ip[2], ip[3]);
  return String(buf);
}

/* Split ip address to uint8_t[4] array. */
void WEBUIClass::stringToIP(const char *ip_string, uint8_t *octets) {
  char * octet;
  char ip_address[16];

  memset(ip_address, 0, 16);
  strcpy(ip_address, ip_string);

  octet = strtok(ip_address, ".");
  for (int j = 0; j < 4; ++j) {

    octets[j] = (uint8_t) atoi(octet);
    octet = strtok(NULL, ".");
  }
}

void WEBUIClass::settingsJson(char *result, size_t len) {
#ifdef DEBUG_WEB
    Serial.printf("[JSON] Free HEAP Before Serialization: %d \r\n", ESP.getFreeHeap());
#endif
  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(11)
      + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(MAX_LED_CHANNELS) + (JSON_OBJECT_SIZE(4) * MAX_LED_CHANNELS);

  DynamicJsonDocument doc(capacity + 30);
  JsonObject root = doc.to<JsonObject>();
  root["response"] = "getSettings";

  /* Services */
  JsonObject services = root.createNestedObject("services");

  /* Get service config */
  services_t *service = CONFIG.getService();

  services["hostname"]            = service->hostname;
  services["ntp_server_name"]     = service->ntp_server_name;
  services["utc_offset_minutes"]  = service->utc_offset_minutes;
  services["ntp_dst"]             = service->ntp_dst;
  services["mqtt_ip_address"]     = ipAddressToString(service->mqtt_ip_address);
  services["mqtt_port"]           = service->mqtt_port;
  services["mqtt_user"]           = service->mqtt_user;
  services["mqtt_password"]       = service->mqtt_password;
  services["enable_ntp_service"]  = service->enable_ntp_service;
  services["enable_mqtt_service"] = service->enable_mqtt_service;

  /* Led */
  JsonArray leds = root.createNestedArray("leds");
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; i++) {

    JsonObject ledChannel = leds.createNestedObject();
    /* Get single led channel config */
    led_t *_led = CONFIG.getLED(i);

    ledChannel["id"]            = i;
    ledChannel["color"]         = _led->color;
    ledChannel["default_duty"]  = _led->default_duty;

  }

  serializeJson(doc, result, len);

#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP After Serialization: %d Memory Usage: %d\r\n", ESP.getFreeHeap(), doc.memoryUsage());
#ifdef DEBUG_WEB_JSON
  serializeJson(doc, Serial);
#endif
#endif
}

void WEBUIClass::networksJson(char *result, size_t len) {
#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP Before Serialization: %d \r\n", ESP.getFreeHeap());
#endif
  const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(MAX_NETWORKS) + JSON_OBJECT_SIZE(8) * MAX_NETWORKS;

  DynamicJsonDocument doc(capacity + 30);
  JsonObject root = doc.to<JsonObject>();
  root["response"] = "getNetworks";
  root["capacity"] = MAX_NETWORKS;

  /* Networks */
  JsonArray networks = root.createNestedArray("networks");
  for (uint8_t i = 0; i < MAX_NETWORKS; i++) {
    /* Get single networks config */
    network_t *network = CONFIG.getNetwork(i);

    if (network->active) {
      JsonObject networkConfig    = networks.createNestedObject();
      networkConfig["id"]         = i;
      networkConfig["ssid"]       = network->ssid;
      networkConfig["password"]   = network->password;
      networkConfig["ip_address"] = ipAddressToString(network->ip_address);
      networkConfig["mask"]       = ipAddressToString(network->mask);
      networkConfig["gateway"]    = ipAddressToString(network->gateway);
      networkConfig["dns"]        = ipAddressToString(network->dns);
      networkConfig["dhcp"]       = network->dhcp;
    }
  }

  serializeJson(doc, result, len);

#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP After Serialization: %d Memory Usage: %d\r\n", ESP.getFreeHeap(), doc.memoryUsage());
#ifdef DEBUG_WEB_JSON
  serializeJson(doc, Serial);
#endif
#endif
}

void WEBUIClass::scheduleJson(char *result, size_t len) {
#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP Before Serialization: %d \r\n", ESP.getFreeHeap());
#endif

  const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(1)
      + MAX_SCHEDULE * (JSON_OBJECT_SIZE(4) + JSON_ARRAY_SIZE(MAX_LED_CHANNELS))
      + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(MAX_LED_CHANNELS) + (JSON_OBJECT_SIZE(3) * MAX_LED_CHANNELS);

  DynamicJsonDocument doc(capacity + 30);
  JsonObject root = doc.to<JsonObject>();
  root["response"] = "getSchedule";
  root["capacity"] = MAX_SCHEDULE;

  /* Schedule */
  JsonArray schedules = root.createNestedArray("schedule");
  for (uint8_t i = 0; i < MAX_SCHEDULE; i++) {

    /* Get One schedule */
    schedule_t *schedule = CONFIG.getSchedule(i);

    if (schedule->active) {
      JsonObject scheduleItem = schedules.createNestedObject();

      scheduleItem["time_hour"]   = schedule->time_hour;
      scheduleItem["time_minute"] = schedule->time_minute;
      scheduleItem["enabled"]     = schedule->enabled;

      JsonArray ledDuty = scheduleItem.createNestedArray("duty");

      for (unsigned char p : schedule->led_duty) {
        ledDuty.add(p); // led channel
      }
    }
  }

  /* Led */
  JsonArray leds = root.createNestedArray("leds");
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; i++) {

    JsonObject ledChannel = leds.createNestedObject();
    /* Get single led channel config */
    led_t *_led = CONFIG.getLED(i);

    ledChannel["id"]            = i;
    ledChannel["color"]         = _led->color;
    ledChannel["default_duty"]  = _led->default_duty;
  }

  serializeJson(doc, result, len);

#ifdef DEBUG_WEB
    Serial.printf("[JSON] Free HEAP After Serialization: %d Memory Usage: %d\r\n", ESP.getFreeHeap(), doc.memoryUsage());
#ifdef DEBUG_WEB_JSON
    serializeJson(doc, Serial);
#endif
#endif

}

void WEBUIClass::statusJson(char *result, size_t len) {
#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP Before Serialization: %d \r\n", ESP.getFreeHeap());
#endif

  const size_t capacity = JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(8)
      + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(MAX_LED_CHANNELS);
  DynamicJsonDocument doc(capacity + 30);
  JsonObject root = doc.to<JsonObject>();
  root["response"] = "getStatus";

  /* Status */
  JsonObject status = root.createNestedObject("status");

  /* Device specific */
  status["hardware"] = HARDWARE;

  /* Get service config */
  services_t *service = CONFIG.getService();
  status["mqttService"]       = ipAddressToString(service->mqtt_ip_address);
  status["ntpService"]        = service->ntp_server_name;

  /* Get device info, refresh periodical */
  status_t * device_info = getDeviceInfo();

  status["upTime"]            = SCHEDULE.getCurrentTimeString();
  status["chipId"]            = device_info->chip_id;
  status["cpuFreq"]           = device_info->cpu_freq;
  status["vcc"]               = device_info->vcc;
  status["freeHeap"]          = device_info->free_heap;
  status["wifiMode"]          = device_info->wifi_mode;
  status["ipAddress"]         = device_info->ip_address;
  status["macAddress"]        = device_info->mac_address;

  serializeJson(doc, result, len);

#ifdef DEBUG_WEB
    Serial.printf("[JSON] Free HEAP After Serialization: %d Memory Usage: %d\r\n", ESP.getFreeHeap(), doc.memoryUsage());
#ifdef DEBUG_WEB_JSON
    serializeJson(doc, Serial);
#endif
#endif

}

void WEBUIClass::dutyJson(char *result, size_t len) {
#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP Before Serialization: %d \r\n", ESP.getFreeHeap());
#endif

  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_ARRAY_SIZE(MAX_LED_CHANNELS);
  DynamicJsonDocument doc(capacity + 30);
  JsonObject root = doc.to<JsonObject>();
  root["response"] = "getDuty";

  /* Get current leds duty */
  JsonArray ledDuty = root.createNestedArray("duty");
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
    ledDuty.add(SCHEDULE.getChannelDuty(i));
  }

  serializeJson(doc, result, len);

#ifdef DEBUG_WEB
  Serial.printf("[JSON] Free HEAP After Serialization: %d Memory Usage: %d\r\n", ESP.getFreeHeap(), doc.memoryUsage());
#ifdef DEBUG_WEB_JSON
  serializeJson(doc, Serial);
#endif
#endif

}

WEBUIClass WEBUI;
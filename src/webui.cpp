/***
** Created by Aleksey Volkov on 2019-04-04.
***/

#include "AsyncWebSocket.h"
#include "Arduino.h"
#include <functional>
#include <NtpClientLib.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include "settings.h"
#include "webui.h"
#include "schedule.h"
#include "webpage.h"
#include "Network.h"
#include "status.h"

/* Public */

void WEBUIClass::init(AsyncWebServer& server) {
  /* Pages ----------------------------------- */
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  server.on("/about", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send File
    AsyncWebServerResponse *response = request->beginResponse_P(200, "text/html", HTML, HTML_SIZE);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });


  /* JSON ------------------------------------ */
  /* GET device status */
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant &root = response->getRoot();

    /* Device specific */
    root["hardware"] = HARDWARE;

    /* Get service config */
    services_t *service = CONFIG.getService();
    root["mqttService"] = IPAddress(service->mqtt_server).toString();
    root["ntpService"] = service->ntp_server;

    /* Get device info, refresh periodical */
    status_t *device_info = getDeviceInfo();

    root["upTime"] = NTP.getUptimeString();
    root["localTime"] = SCHEDULE.getCurrentTimeString();
    root["chipId"] = device_info->chip_id;
    root["cpuFreq"] = device_info->cpu_freq;
    root["vcc"] = device_info->vcc;
    root["freeHeap"] = device_info->free_heap;
    root["wifiMode"] = device_info->wifi_mode;
    root["ipAddress"] = device_info->ip_address;
    root["macAddress"] = device_info->mac_address;
    root["brightness"] = SCHEDULE.getBrightness();

    JsonArray leds = root.createNestedArray("channels");
    for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
      leds.add(SCHEDULE.getChannelDuty(i));
    }

    response->setLength();
    request->send(response);
  });

  /* GET led channels schedule config */
  server.on("/config/schedule", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    /* Schedule */
    JsonArray schedule = root.createNestedArray("schedule");
    for (uint8_t i = 0; i < MAX_SCHEDULE; i++)
    {
      /* Get One schedule */
      schedule_t *_schedule = CONFIG.getSchedule(i);
      if (_schedule->active) {
        JsonObject scheduleItem = schedule.createNestedObject();

        scheduleItem["time_hour"]       = _schedule->time_hour;
        scheduleItem["time_minute"]     = _schedule->time_minute;
        scheduleItem["brightness"]      = _schedule->brightness;

        JsonArray ledDuty = scheduleItem.createNestedArray("duty");

        for (unsigned char p : _schedule->channel) {
          ledDuty.add(p); // led channel
        }
      }
    }

    response->setLength();
    request->send(response);
  });

  /* SET led channels schedule config */
  AsyncCallbackJsonWebHandler* schedule_handler = new AsyncCallbackJsonWebHandler("/config/schedule", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();

    /* Reset schedule cache */
    for (uint8_t i = 0; i < MAX_SCHEDULE; ++i) {
      schedule_t *schedule = CONFIG.getSchedule(i);
      schedule->active = false;
    }

    /* Parse New Schedule config */
    JsonArray schedules = jsonObj["schedule"];
    for (uint8_t i = 0; i < schedules.size(); ++i) {
      if (i < MAX_SCHEDULE) {
        /* get point to current schedule config (eeprom cache) */
        schedule_t *schedule = CONFIG.getSchedule(i);

        /* update value in (eeprom cache) */
        schedule->time_hour       = schedules[i]["time_hour"].as<int>();
        schedule->time_minute     = schedules[i]["time_minute"].as<int>();
        schedule->active          = true;
        schedule->brightness  = schedules[i]["brightness"].as<int>();

        LOG_WEB("[WEBSERVER] Set Schedule: %d:%d\n", schedule->time_hour, schedule->time_minute);

        for (uint8_t j = 0; j < schedules[i]["duty"].size(); ++j) {
          if (j < MAX_LED_CHANNELS)
          {
            schedule->channel[j] = schedules[i]["duty"][j].as<int>();
          }
        }
      }
    }

    /* Store EEPROM settings (sync cache and eeprom) */
    CONFIG.setSettings();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    root["save"].set(true);

    response->setLength();
    request->send(response);
  });

  /* GET led channels config */
  server.on("/config/leds", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    /* Leds */
    JsonArray leds = root.createNestedArray("leds");
    for (uint8_t i = 0; i < MAX_LED_CHANNELS; i++) {
      JsonObject ledChannel = leds.createNestedObject();
      /* Get single led channel config */
      led_t *_led = CONFIG.getLED(i);

      ledChannel["id"]        = i;
      ledChannel["color"]     = _led->color;
      ledChannel["power"]     = _led->power;
      ledChannel["state"]     = _led->state;
      ledChannel["last_duty"] = _led->last_duty;
    }

    response->setLength();
    request->send(response);
  });

  /* SET led channels schedule config */
  AsyncCallbackJsonWebHandler* leds_handler = new AsyncCallbackJsonWebHandler("/config/leds", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();

    /* Parse New LED config */
    JsonArray leds = jsonObj["leds"];

    for (uint8_t i = 0; i < leds.size(); ++i) {
      if (i < MAX_LED_CHANNELS) {

        /* get point to current led config (eeprom cache) */
        led_t *_led = CONFIG.getLED(i);

        /* update value in (eeprom cache) */
        strlcpy(_led->color, leds[i]["color"] | "#DDEFFF", 8);
        _led->last_duty     = leds[i]["last_duty"].as<int>();
        _led->power         = leds[i]["power"].as<int>();
        _led->state         = leds[i]["state"].as<int>();
      }
    }

    /* Store EEPROM settings (sync cache and eeprom) */
    CONFIG.setSettings();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    root["save"].set(true);

    response->setLength();
    request->send(response);
  });

  /* GET services config */
  server.on("/config/services", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    /* Services */
    JsonObject services = root.createNestedObject("services");

    /* Get service config */
    services_t *service = CONFIG.getService();

    services["hostname"]            = service->hostname;
    services["ntp_server"]          = service->ntp_server;
    services["utc_offset"]          = service->utc_offset;
    services["ntp_dst"]             = service->ntp_dst;
    services["mqtt_server"]         = IPAddress(service->mqtt_server).toString();
    services["mqtt_port"]           = service->mqtt_port;
    services["mqtt_qos"]            = service->mqtt_qos;
    services["mqtt_user"]           = service->mqtt_user;
    services["mqtt_password"]       = service->mqtt_password;
    services["enable_ntp"]          = service->enable_ntp;
    services["enable_mqtt"]         = service->enable_mqtt;

    response->setLength();
    request->send(response);
  });

  /* SET services config */
  AsyncCallbackJsonWebHandler* services_handler = new AsyncCallbackJsonWebHandler("/config/services", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();

    /* Parse Services config */
    JsonObject services = jsonObj["services"];

    /* get point to current services config (eeprom cache) */
    services_t *service = CONFIG.getService();
    strlcpy(service->hostname, services["hostname"] | "", 20);
    strlcpy(service->ntp_server, services["ntp_server"] | "es.pool.ntp.org", 20);
    strlcpy(service->mqtt_user, services["mqtt_user"] | "", 16);
    strlcpy(service->mqtt_password, services["mqtt_password"] | "", 16);
    WEBUI.stringToIP(services["mqtt_server"], service->mqtt_server);
    service->mqtt_port           = services["mqtt_port"].as<int>();
    service->mqtt_qos            = services["mqtt_qos"].as<int>();
    service->utc_offset          = services["utc_offset"].as<int>();
    service->ntp_dst             = services["ntp_dst"].as<boolean>();
    service->enable_ntp          = services["enable_ntp"].as<boolean>();
    service->enable_mqtt         = services["enable_mqtt"].as<boolean>();

    /* Store EEPROM settings (sync cache and eeprom) */
    CONFIG.setSettings();

    /* Reconfigure network and services */
    NETWORK.reloadSettings();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();


    root["save"].set(true);

#ifdef DEBUG_WEB_JSON
    serializeJson(root, Serial);
    Serial.println();
#endif

    response->setLength();
    request->send(response);
  });

  /* GET Networks config */
  server.on("/config/networks", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

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
        networkConfig["ip_address"] = IPAddress(network->ip_address).toString();
        networkConfig["mask"]       = IPAddress(network->mask).toString();
        networkConfig["gateway"]    = IPAddress(network->gateway).toString();
        networkConfig["dns"]        = IPAddress(network->dns).toString();
        networkConfig["dhcp"]       = network->dhcp;
      }
    }

    response->setLength();
    request->send(response);
  });

  /* SET Networks config */
  AsyncCallbackJsonWebHandler* networks_handler = new AsyncCallbackJsonWebHandler("/config/networks", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();

    /* Reset network cache */
    for (uint8_t i = 0; i < MAX_NETWORKS; i++) {
      network_t *network = CONFIG.getNetwork(i);
      network->active = false;
    }

    /* Parse New NETWORK config */
    JsonArray networks = jsonObj["networks"];
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
        LOG_WEB("[WEBSOCKET] ssid: %s password: %s\n", network->ssid, network->password);

      }
    }

    /* Store EEPROM settings (sync cache and eeprom) */
    CONFIG.setSettings();

    /* Reload WiFi settings */
    NETWORK.reloadSettings();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    root["save"].set(true);

    response->setLength();
    request->send(response);
  });

  /* ----- Set: Duty at Home page ----- */
  AsyncCallbackJsonWebHandler* duty_handler = new AsyncCallbackJsonWebHandler("/set/duty", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    JsonArray duty = jsonObj["duty"];

    for (uint8_t i = 0; i < duty.size(); i++) {
      if (i < MAX_LED_CHANNELS) {
        uint8_t new_duty = duty[i];

        /* Apply new Channel Duty */
        SCHEDULE.setChannelDuty(i, new_duty);
        LOG_WEB("[WEBSERVER] Set Led Duty: new: %d, old: %d\n", new_duty, SCHEDULE.getChannelDuty(i));
      }
    }

    /* Send Response */
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();


    JsonArray leds = root.createNestedArray("duty");
    for (int i = 0; i < MAX_LED_CHANNELS; ++i) {
      leds.add(SCHEDULE.getTargetChannelDuty(i));
    }

    response->setLength();
    request->send(response);
  });

  /* ----- Set: Brightness at Home page ----- */
  AsyncCallbackJsonWebHandler* brightness_handler = new AsyncCallbackJsonWebHandler("/set/brightness", [](AsyncWebServerRequest *request, JsonVariant &json) {
    JsonObject jsonObj = json.as<JsonObject>();
    JsonVariant brightness = jsonObj["brightness"];

    /* Apply new Brightness */
    SCHEDULE.setBrightness(brightness.as<int>());

    LOG_WEB("[WEBSERVER] Set Brightness: %d\n", brightness.as<int>());

    /* Send Response */
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();
    root.set(SCHEDULE.getBrightness());

    response->setLength();
    request->send(response);

  });

  /* Reboot */
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    /* Reboot */
    ESP.restart();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    root["success"].set(true);

    response->setLength();
    request->send(response);
  });

  /* Factory reset */
  server.on("/factory", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    /* Erase EEPROM */
    CONFIG.erase();

    // Send Json Response
    AsyncJsonResponse *response = new AsyncJsonResponse();
    JsonVariant& root = response->getRoot();

    root["success"].set(true);

    response->setLength();
    request->send(response);
  });

  server.addHandler(schedule_handler);
  server.addHandler(leds_handler);
  server.addHandler(services_handler);
  server.addHandler(networks_handler);
  server.addHandler(duty_handler);
  server.addHandler(brightness_handler);

  /* CORS */
//  server.onNotFound([](AsyncWebServerRequest *request) {
//    if (request->method() == HTTP_OPTIONS) {
//      request->send(200);
//    } else {
//      request->send(404);
//    }
//  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
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
    octet = strtok(nullptr, ".");
  }
}

WEBUIClass WEBUI;
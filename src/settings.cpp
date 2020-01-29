#include <EEPROM_Rotate.h>
#include "Ticker.h"
#include "settings.h"

Ticker save_timer;
EEPROM_Rotate eepromRotate;

void Settings::init() {
  // EEPROM Initialization
  eepromRotate.size(2);
  eepromRotate.begin(EEPROM_SIZE);

#if DEBUG_EEPROM
  Serial.printf("[EEPROM] Sector pool size : %u\n", eepromRotate.size());
  Serial.printf("[EEPROM] Sectors in use   : ");
  for (uint32_t i = 0; i < eepromRotate.size(); i++) {
    if (i>0) Serial.print(", ");
    Serial.print(eepromRotate.base() - i);
  }
  Serial.println();
  Serial.printf("[EEPROM] Current sector   : %u\n", eepromRotate.current());

  Serial.println();
  Serial.printf("[EEPROM] Dumping data for sector #%u\n", eepromRotate.current());
  eepromRotate.dump(Serial);
#endif

  const char * empty_str = " ";

  /* init auth configuration */
  eepromRotate.get(AUTH_OFFSET, auth);
  if (auth.magic_number != 10) {
    auth.magic_number = 10;
    strlcpy(auth.login, AUTH_LOGIN, 32);
    strlcpy(auth.password, AUTH_PASSWORD, 32);

    /* set default values */
    eepromRotate.put(AUTH_OFFSET, auth);
    _update_requested = true;

  }

  /* init network configuration */
  for(size_t i = 0; i < MAX_NETWORKS; i++)
  {
    LOG_EEPROM("[EEPROM] NETWORK Config ID: %d. \n", i);
    eepromRotate.get(NETWORK_OFFSET + sizeof(network_t) * i, network[i]);
    if (network[i].magic_number != (20 + i)) {
      LOG_EEPROM("[EEPROM] NETWORK Config incorrect: magic %d, loading default. \n", network[i].magic_number);
      network[i].magic_number  = (uint8_t)(20 + i);
      strlcpy(network[i].ssid, empty_str, 32);
      strlcpy(network[i].password, empty_str, 32);

      network[i].ip_address[0] = 192;
      network[i].ip_address[0] = 168;
      network[i].ip_address[0] = 1;
      network[i].ip_address[0] = 100;

      network[i].mask[0]       = 255;
      network[i].mask[0]       = 255;
      network[i].mask[0]       = 255;
      network[i].mask[0]       = 0;

      network[i].gateway[0]    = 192;
      network[i].gateway[0]    = 168;
      network[i].gateway[0]    = 1;
      network[i].gateway[0]    = 1;

      network[i].dns[0]        = 192;
      network[i].dns[0]        = 168;
      network[i].dns[0]        = 1;
      network[i].dns[0]        = 1;

      network[i].dhcp          = true;
      network[i].active        = false; /* hide config in web ui */

      /* set default values */
      eepromRotate.put(NETWORK_OFFSET + sizeof(network_t) * i, network[i]);
      _update_requested = true;

    }
    LOG_EEPROM("[EEPROM] NETWORK Config ID: %d, Status: %d. \n", i, network[i].active);

  }

  /* init services configuration */
  /* Global */
  eepromRotate.get(SERVICES_OFFSET, service);
  if (service.magic_number != 30) {
    service.magic_number = 30;

    char hostname_buf[20];
    sprintf(hostname_buf, "LED_%d", ESP.getChipId());
    strlcpy(service.hostname, hostname_buf, 20);

    /* NTP */
    strcpy(service.ntp_server, "es.pool.ntp.org");
    service.utc_offset = 60;
    service.ntp_dst = true;
    service.enable_ntp = true;

    /*MQTT */
    strcpy(service.mqtt_user, empty_str);
    strcpy(service.mqtt_password, empty_str);
    service.mqtt_port = 1883;
    service.enable_mqtt = false;
    service.mqtt_qos = 0;

    /* set default values */
    eepromRotate.put(SERVICES_OFFSET, service);
    _update_requested = true;

  }

  /* init led configuration */
  for(size_t i = 0; i < MAX_LED_CHANNELS; i++)
  {
    eepromRotate.get(LED_OFFSET + sizeof(led_t) * i, led[i]);
    if (led[i].magic_number != (40 + i)) {
      led[i].magic_number = (uint8_t)(40 + i);

      strlcpy(led[i].color, "#DDEFFF", 8);  /* default color #DDEFFF -> 'Cold White' in UI */
      led[i].last_duty = 0;                 /* 0 - 100 -> channel boot duty */
      led[i].power = 0;                     /* 0 ->  0.0 in Watts x 10 */
      led[i].state = 1;                     /* 0 ->  OFF | 1 -> ON */

      /* set default values */
      eepromRotate.put(LED_OFFSET, led);
      _update_requested = true;

    }

  }

  /* init schedule configuration */
  for(size_t i = 0; i < MAX_SCHEDULE; i++)
  {
    eepromRotate.get(SCHEDULE_OFFSET + sizeof(schedule_t) * i, schedule[i]);
    if (schedule[i].magic_number != (50 + i)) {
      schedule[i].magic_number = (uint8_t)(50 + i);

      schedule[i].time_hour     = 0;
      schedule[i].time_minute   = 0;
      schedule[i].channel[0]    = 0; /* CH1 Default */
      schedule[i].channel[1]    = 0; /* CH2 Default */
      schedule[i].channel[2]    = 0; /* CH3 Default */
#if MAX_LED_CHANNELS == 5
      schedule[i].channel[3]    = 0; /* CH4 Default */
      schedule[i].channel[4]    = 0; /* CH5 Default */
#endif
      schedule[i].brightness    = 0; /* All Channels brightness */
      schedule[i].active        = false;

      /* set default values */
      eepromRotate.put(SCHEDULE_OFFSET + sizeof(schedule_t) * i, schedule[i]);
      _update_requested = true;

    }

  }

  /* save new config if, some values updated */
  if (_update_requested) {
    eepromRotate.commit();
  }

}

/* Save Param to EEPROM, delayed to avoid wear */
void Settings::setSettings() {

  /* Store Auth */
  eepromRotate.put(AUTH_OFFSET, auth);

  /* Store Network */
  for(size_t i = 0; i < MAX_NETWORKS; i++)
  {
    eepromRotate.put(NETWORK_OFFSET + sizeof(network_t) * i, network[i]);
  }

  /* Store Service */
  eepromRotate.put(SERVICES_OFFSET, service);

  /* Store Led */
  for(size_t i = 0; i < MAX_LED_CHANNELS; i++) {
    eepromRotate.put(LED_OFFSET + sizeof(led_t) * i, led[i]);
  }

  /* Store Schedule */
  for(size_t i = 0; i < MAX_SCHEDULE; i++)
  {
    eepromRotate.put(SCHEDULE_OFFSET + sizeof(schedule_t) * i, schedule[i]);
  }

  LOG_EEPROM("[EEPROM] Save timer started.\n");

  /* Delay to avoid memory wear */
  save_timer.once(10, std::bind(&Settings::save, this));

}

bool Settings::save() {
  LOG_EEPROM("[EEPROM] Saved.\n");
  return eepromRotate.commit();
}

String Settings::getNtpServerName() {
  String name(service.ntp_server);
  if (name.length() > 0)
    return name;
  else
    return String("pool.ntp.org");
}

int16_t Settings::getNtpOffset() {
  return service.utc_offset;
}

led_t * Settings::getLED(uint8_t id) {
  if (id > (MAX_LED_CHANNELS - 1)) id = MAX_LED_CHANNELS - 1;
  return &led[id];
}

schedule_t * Settings::getSchedule(uint8_t id) {
  if (id > MAX_SCHEDULE - 1) id = MAX_SCHEDULE - 1;
  return &schedule[id];
};

network_t * Settings::getNetwork(uint8_t id) {
  if (id > MAX_NETWORKS - 1) id = MAX_NETWORKS - 1;
  return &network[id];
};

services_t * Settings::getService() {
  return &service;
};

auth_t * Settings::getAuth() {
  return &auth;
}

char * Settings::getHostname() {
  return (char *)service.hostname;
}

/* Store new credentials in EEPROM */
void Settings::updateAuth() {
  eepromRotate.put(AUTH_OFFSET, auth);
  eepromRotate.commit();
}

/* Erase EEPROM */
void Settings::erase() {
  for (int i = 0; i < EEPROM_SIZE; ++i) {
    eepromRotate.put(i, 0xFF);
  }
  eepromRotate.commit();

  LOG_EEPROM("[EEPROM] Erased.\n");

}

Settings CONFIG;
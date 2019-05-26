/***
** Created by Aleksey Volkov on 2019-05-24.
***/

#include <user_interface.h>
#include <IPAddress.h>
#include "Ticker.h"
#include "status.h"
#include "mqtt.h"

Ticker status_refresh_timer;
status_t status = {0};
bool refresh = true;

void refresh_tik() {
  refresh = true;
};

status_t * getDeviceInfo() {
  return &status;
}

void statusLoop() {
  if (refresh) {
    refresh = false;

    /* get end store static values */
    status.chip_id = system_get_chip_id();
    status.free_heap = system_get_free_heap_size();
    status.cpu_freq = system_get_cpu_freq();
    status.vcc = analogRead(A0);
    status.wifi_mode = wifi_get_opmode();

    /* IP Address */
    String ip_address;
    struct ip_info ip;
    wifi_get_ip_info(STATION_IF, &ip);
    ip_address = IPAddress(ip.ip.addr).toString();
    strlcpy(status.ip_address, ip_address.c_str(), 16);

    /* WiFi MAC Address */
    uint8_t mac[6];
    char macStr[18] = { 0 };
    wifi_get_macaddr(STATION_IF, mac);
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    strlcpy(status.mac_address, macStr, 18);

    /* Publish to MQTT topic {hostname}/status */
    publishDeviceStatusToMqtt();

    /* Setup refresh timer */
    status_refresh_timer.once(30, refresh_tik);
  }
};
/***
** Created by Aleksey Volkov on 2019-05-09.
***/

#ifndef HV_CC_LED_DRIVER_NETWORK_H
#define HV_CC_LED_DRIVER_NETWORK_H

#include "Ticker.h"

class NetworkClass {

public:
  bool isConnected = false;

  void init();
  void reloadSettings();
  void connectSTA();
  void startSTA();
  void startAP();
  void stopAP();
  void loop();

private:

  bool new_wifi_settings = false;
  bool new_mqtt_settings = false;
  bool check_wifi = false;
  Ticker connection_timer;
  uint8_t connect_timeout = 10;
  void tik();
};

extern NetworkClass NETWORK;

#endif //HV_CC_LED_DRIVER_NETWORK_H

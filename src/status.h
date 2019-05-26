/***
** Created by Aleksey Volkov on 2019-05-24.
***/

#ifndef HV_CC_LED_DRIVER_STATUS_H
#define HV_CC_LED_DRIVER_STATUS_H

#include "Arduino.h"

typedef struct {
  uint32 chip_id;
  uint32 free_heap;
  uint8_t cpu_freq;
  uint16_t vcc;
  uint8 wifi_mode;
  char ip_address[16];
  char mac_address[18];
} status_t;

void refresh_tik();
void statusLoop();
status_t * getDeviceInfo();

#endif //HV_CC_LED_DRIVER_STATUS_H

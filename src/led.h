/***
** Created by Aleksey Volkov on 2019-05-08.
***/

#ifndef HV_CC_LED_DRIVER_LED_H
#define HV_CC_LED_DRIVER_LED_H

#include "Ticker.h"

typedef enum {
  OFF, ON, HALF_SEC, ONE_SHORT_BLINK, TWO_SHORT_BLINK, THREE_SHORT_BLINK, SHORT_BLINKS
} led_mode_t;

class LEDClass {
  public:
    void init();
    void loop();
    void setMode(led_mode_t mode);

  private:
    Ticker led_blink_timer;
    bool led_refresh   = false;
    void refresh();
};

extern LEDClass LED;

#endif //HV_CC_LED_DRIVER_LED_H

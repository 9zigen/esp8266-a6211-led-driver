/***
** Created by Aleksey Volkov on 2019-04-11.
***/

#ifndef HV_CC_LED_DRIVER_SCHEDULE_H
#define HV_CC_LED_DRIVER_SCHEDULE_H

#include "settings.h"
#include "Ticker.h"

#ifndef DEBUG_UI_PORT
#define DEBUG_UI_PORT Serial
#endif

#ifdef DEBUG_SCHEDULE
#define LOG_SCHEDULE(...) DEBUG_UI_PORT.printf( __VA_ARGS__ )
#else
#define LOG_SCHEDULE(...)
#endif

#define MAX_BRIGHTNESS 255
#define MAX_DUTY       255
#define RTC_LED_MAGIC 0xAECE00EF
#define RTC_LED_ADDR  65

typedef struct {
  uint8_t current_duty;
  uint8_t target_duty;
  uint8_t real_duty;
  uint32_t steps_left;
} led_schedule_t;

typedef struct {
  uint8_t target_duty[MAX_LED_CHANNELS];
  uint8_t led_brightness;
  uint32 magic_number;
} led_state_rtc_mem_t;



class ScheduleClass {

public:
  void init();
  void refresh();
  static char * getCurrentTimeString();
  void loop();
  uint8_t getChannelDuty(uint8_t channel);
  void setChannelDuty(uint8_t channel, uint8_t duty);
  uint8_t getTargetChannelDuty(uint8_t id);
  uint8_t getBrightness();
  void setBrightness(uint8_t newBrightness);
  uint8_t getChannelRealDuty(uint8_t channel);

private:
  bool process_schedule = true;
  uint8_t brightness = MAX_BRIGHTNESS;
  led_schedule_t channel[MAX_LED_CHANNELS] = {};

  void saveChannelState();
  void updateChannels();
  void transition();
  static uint32_t toPWM(uint8_t percent);
  static int minutesLeft(time_t t, unsigned char schedule_hour, unsigned char schedule_minute);
};

extern ScheduleClass SCHEDULE;

#endif //HV_CC_LED_DRIVER_SCHEDULE_H

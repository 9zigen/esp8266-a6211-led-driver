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

#define MAX_BRIGHTNESS 255.0f
#define RTC_LED_MAGIC 0xAECE00EF

typedef enum  {
  CHANNEL_DISABLED = 0,
  CHANNEL_ENABLED
} channel_state_t;

typedef struct {
  channel_state_t current_state;
  channel_state_t target_state;
  uint8_t current_duty;
  uint8_t target_duty;
  uint8_t real_duty;
  uint32_t steps_left;
} led_schedule_t;

typedef struct {
  channel_state_t target_state[MAX_LED_CHANNELS];
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
  channel_state_t getChannelState(uint8_t channel);
  void setChannelState(uint8_t channel, channel_state_t state);
  uint8_t getChannelDuty(uint8_t channel);
  void setChannelDuty(uint8_t channel, uint8_t duty);
  uint8_t getBrightness();
  void setBrightness(uint8_t newBrightness);
  uint8_t getChannelRealDuty(uint8_t channel);

private:
  bool process_schedule = true;
  uint8_t brightness = 255;
  led_schedule_t _leds[MAX_LED_CHANNELS] = {};

  void saveChannelState();
  void updateChannels();
  void transition();
  static uint32_t toPWM(uint8_t percent);
  static int minutesLeft(time_t t, unsigned char schedule_hour, unsigned char schedule_minute);
};

extern ScheduleClass SCHEDULE;

#endif //HV_CC_LED_DRIVER_SCHEDULE_H

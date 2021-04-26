/***
** Created by Aleksey Volkov on 2019-04-11.
***/
#ifndef OTA_ONLY

#include "Arduino.h"
#include "Ticker.h"
#include <TimeLib.h>
#include <NtpClientLib.h>
#include "settings.h"
#include "mqtt.h"
#include "app_schedule.h"

#define PWM_CHANNEL_NUM_MAX MAX_LED_CHANNELS
extern "C" {
#include "pwm.h"
}

Ticker shedule_refresh_timer;
Ticker transition_timer;

/* Current local time
 * Format: HH:MM
 * Update in loop, NOT callback */
char current_time[6] = {0};

#if MAX_LED_CHANNELS == 3
  uint8_t ledPin[MAX_LED_CHANNELS] = {LED_CH0_PIN, LED_CH1_PIN, LED_CH2_PIN};
#elif MAX_LED_CHANNELS == 5
  uint8_t ledPin[MAX_LED_CHANNELS] = {LED_CH0_PIN, LED_CH1_PIN, LED_CH2_PIN, LED_CH3_PIN, LED_CH4_PIN};
#endif

void ScheduleClass::init() {
  /* Setup PWM */
  uint32 pwm_duty_init[PWM_CHANNEL_NUM_MAX];

#if MAX_LED_CHANNELS == 3
  uint32 io_info[PWM_CHANNEL_NUM_MAX][3] = {
    //  MUX, FUNC, PIN
    //	{PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5,   5}, // D1
    //	{PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,   4}, // D2
    //	{PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0,   0}, // D3
    //	{PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2,   2}, // D4
    {PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14}, // D5
    {PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12}, // D6
    {PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13}, // D7
    //	{PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15 ,15}, // D8
    //  D0 - not have PWM :-(
  };
#elif MAX_LED_CHANNELS == 5
  uint32 io_info[PWM_CHANNEL_NUM_MAX][3] = {
    //  MUX, FUNC, PIN
    {PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4,   4}, // D2
    {PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5,   5}, // D1
    {PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13}, // D7
    {PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12}, // D6
    {PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14}, // D5
    //	{PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0,   0}, // D3
    //	{PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2,   2}, // D4
    //	{PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15 ,15}, // D8
    //  D0 - not have PWM :-(
  };
#endif
  for (unsigned int i=0; i < MAX_LED_CHANNELS; i++) {
    pwm_duty_init[i] = 0;
    pinMode(ledPin[i], OUTPUT);
  }

  pwm_init(LIGHT_MAX_PWM, pwm_duty_init, PWM_CHANNEL_NUM_MAX, io_info);
  pwm_start();



  /* Read RTC MEM packet */
  led_state_rtc_mem_t rtc_mem;
  system_rtc_mem_read(RTC_LED_ADDR, &rtc_mem, sizeof(rtc_mem));

  /* Check Magic if OK, apply RTC backup */
  if (rtc_mem.magic_number == RTC_LED_MAGIC) {
    LOG_SCHEDULE("[SCHEDULE] RTC Magic is OK: %04x\n", rtc_mem.magic_number);

    /* Restore brightness */
    brightness = rtc_mem.led_brightness;

    for (unsigned int i=0; i < MAX_LED_CHANNELS; i++) {
      /* Restore channel state */
      channel[i].target_duty = rtc_mem.target_duty[i];
      LOG_SCHEDULE("[SCHEDULE] GET RTC MEM LED %d Brightness: %d Target: %d\n", i, brightness, channel[i].target_duty);
    }
  } else {
    /* Apply initial led channel output state */
    LOG_SCHEDULE("[SCHEDULE] LOAD LED Defaults from flash\n");

    /* Restore brightness
     * ToDo: store brightness in flash */
    brightness = MAX_BRIGHTNESS;

    for (uint8_t i=0; i < MAX_LED_CHANNELS; i++) {
      led_t * led = CONFIG.getLED(i);
      channel[i].target_duty    = led->last_duty;
    }
  }

  /* Setup refresh timer */
  shedule_refresh_timer.attach_ms(1000, std::bind(&ScheduleClass::refresh, this));
}

void ScheduleClass::refresh() {
  process_schedule = true;
}

int ScheduleClass::minutesLeft(time_t t, uint8_t schedule_hour, uint8_t schedule_minute) {
  auto now_hour   = (uint8_t)hour(t);
  auto now_minute = (uint8_t)minute(t);
  return (schedule_hour - now_hour) * 60 + schedule_minute - now_minute;
}

/* Return String in HH:MM format */
char * ScheduleClass::getCurrentTimeString() {
  return current_time;
}

/* Light loop */
void ScheduleClass::loop() {
  /* Every second action, check Schedule */
  if (process_schedule) {
    /* wait next timer event */
    process_schedule = false;

    /* get local time and store for later use */
    time_t local_time = now();
    auto now_hour   = (uint8_t)hour(local_time);
    auto now_minute = (uint8_t)minute(local_time);
    snprintf(current_time, 6, "%02u:%02u", now_hour, now_minute);

    for (uint8_t j = 0; j < MAX_SCHEDULE; ++j) {
      schedule_t * _schedule = CONFIG.getSchedule(j);

      if (_schedule->active)
      {
        /* once, every min check */
        if ((second(local_time) == 0)) {

          /* Count Minutes left */
          auto left = (int) (minutesLeft(local_time, _schedule->time_hour, _schedule->time_minute));

          LOG_SCHEDULE("[SCHEDULE] Now: %s Next Point: %d:%d Minutes left: %d\n", current_time,
                       _schedule->time_hour,
                       _schedule->time_minute, left);

          /* Set target duty from schedule */
          if (left == 0) {
            for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
              brightness = _schedule->brightness;
              channel[i].target_duty = _schedule->channel[i];

              LOG_SCHEDULE("[SCHEDULE] LED %d current: %d target: %d real: %d brightness: %d left: %d\n",
                           i, channel[i].current_duty, channel[i].target_duty, channel[i].real_duty, _schedule->brightness, left);

            }
          }
        }
      }
    }

    /* check for pending transition */
    updateChannels();
  }
}

void ScheduleClass::updateChannels() {
  bool transition = false;

  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {

    /* Calc Real Duty */
    uint8_t real_duty = channel[i].current_duty * brightness / MAX_BRIGHTNESS;

    if (channel[i].current_duty != channel[i].target_duty || channel[i].real_duty != real_duty) {

      /* Setup transition */
      channel[i].steps_left = 50;
      transition = true;
    }
  }

  /* Start transition */
  if (transition) {
    transition_timer.attach_ms(10, std::bind(&ScheduleClass::transition, this));
  }

}

/* Save channel state to RTC Memory */
void ScheduleClass::saveChannelState() {
  /* Prepare RTC MEM */
  led_state_rtc_mem_t rtc_mem = {};
  rtc_mem.magic_number = RTC_LED_MAGIC;
  rtc_mem.led_brightness = brightness;

  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
    /* Save target duty and channel state */
    rtc_mem.target_duty[i] = channel[i].target_duty;

    /* Duty or Brightness changed */
    LOG_SCHEDULE("[SCHEDULE] RTC MEM SET LED %u Brightness: %u Target: %u\n", i, rtc_mem.led_brightness, rtc_mem.target_duty[i]);

  }

  /* Save channels to RTC mem */
  system_rtc_mem_write(RTC_LED_ADDR, &rtc_mem, sizeof(rtc_mem));
}

/* LED channels update status every second, the transition should be faster than this period */
void ScheduleClass::transition() {

  uint32_t steps_left = 0;
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {

    if (channel[i].steps_left > 0) {
      channel[i].steps_left--;

      if (channel[i].steps_left == 0) {
        channel[i].current_duty = channel[i].target_duty;
      } else {
        double difference = (0.0f + channel[i].target_duty - channel[i].current_duty) / (channel[i].steps_left + 1);
        channel[i].current_duty = (uint32_t) fabs(0.0f + channel[i].current_duty + difference);
      }

      /* Calc Real Duty */
      uint8_t real_duty = channel[i].current_duty * brightness / MAX_BRIGHTNESS;

      /* Brightness changed */
      if (channel[i].real_duty != real_duty) {
        double difference = (0.0f + real_duty - channel[i].real_duty) / (channel[i].steps_left + 1);
        channel[i].real_duty = (uint32_t) fabs(0.0f + channel[i].real_duty + difference);
      }

      /* Apply New Duty */
      pwm_set_duty(toPWM(channel[i].real_duty), i);

      /* count steps left */
      steps_left += channel[i].steps_left;
    }
  }

  /* END Transition if steps_left CH0 + CH1 + etc = 0 */
  if (steps_left == 0 ) {
    /* Stop transition */
    transition_timer.detach();

    /* Publish channel duty */
    publishChannelDuty();

    /* Publish brightness */
    publishBrightness();

    /* Publish channel state */
    publishChannelState();

    /* Save channel state to RTC Memory */
    saveChannelState();
  }

  /* Apply new pwm */
  pwm_start();
}

/* Return 0 -> 100 Brightness Value */
uint8_t ScheduleClass::getBrightness() {
  return brightness;
}

/* Set New Brightness Value */
void ScheduleClass::setBrightness(uint8_t newBrightness) {
  if (newBrightness != brightness) {
    brightness = newBrightness;
  } else {
    return;
  }
}

/* Return 0 -> 100 Duty Value */
uint8_t ScheduleClass::getChannelDuty(uint8_t id) {
  return channel[id].current_duty;
}

uint8_t ScheduleClass::getTargetChannelDuty(uint8_t id) {
  return channel[id].target_duty;
}

/* Set New Channel Duty */
void ScheduleClass::setChannelDuty(uint8_t id, uint8_t duty) {
  channel[id].target_duty  = duty;
}

/* Convert from 0 - MAX_DUTY to 0 - 5000 or 2500 PWM Duty */
uint32_t ScheduleClass::toPWM(uint8_t value) {
  return value * LIGHT_MAX_PWM / MAX_DUTY;
}

ScheduleClass SCHEDULE;

#endif
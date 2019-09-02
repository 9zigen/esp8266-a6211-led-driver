/***
** Created by Aleksey Volkov on 2019-04-11.
***/

#include "Arduino.h"
#include "Ticker.h"
#include <TimeLib.h>
#include <NtpClientLib.h>
#include "settings.h"
#include "mqtt.h"
#include "schedule.h"

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
//    io_info[i][0] = getIOMux(ledPin[i]);
//    io_info[i][1] = getIOFunc(ledPin[i]);
//    io_info[i][2] = ledPin[i];
    pinMode(ledPin[i], OUTPUT);
  }

  pwm_init(LIGHT_MAX_PWM, pwm_duty_init, PWM_CHANNEL_NUM_MAX, io_info);
  pwm_start();

  /* Apply initial led channel output state */
  for (unsigned int i=0; i < MAX_LED_CHANNELS; i++) {
    led_t * led = CONFIG.getLED(i);
    _leds[i].current_duty = 0;
    _leds[i].target_duty  = led->default_duty;
    _leds[i].steps_left   = 50;
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

///* Return Nearest schedule point index */
//uint8_t ScheduleClass::getNearestPoint() {
//  uint8_t minimal = 0;
//  bool first_minimal_found = false;
//
//  /* Current time */
//  time_t local_time = now();
//
//  for (int i = 0; i < MAX_SCHEDULE; ++i) {
//
//    schedule_t *_schedule_a = CONFIG.getSchedule(i);
//    /* Select only Enabled Points*/
//    if (_schedule_a->active && _schedule_a->enabled) {
//
//      /* current point */
//      auto left_a = (int)(minutesLeft(local_time, _schedule_a->time_hour, _schedule_a->time_minute));
//
//      if (left_a >= 0) {
//        /* store first active index */
//        if (!first_minimal_found) {
//          minimal = i;
//          first_minimal_found = true;
//        }
//
//        /* last minimal point */
//        schedule_t *_schedule_b = CONFIG.getSchedule(minimal);
//        auto left_b = (int)(minutesLeft(local_time, _schedule_b->time_hour, _schedule_b->time_minute));
//
//        /* compare points */
//        if (left_a < left_b) {
//          /* New minimal found */
//          minimal = i;
//        }
//      }
//    }
//  }
//
//  return minimal;
//}

/* Return String in HH:MM format */
char * ScheduleClass::getCurrentTimeString() {
  return current_time;
}

void ScheduleClass::loop() {

  /* Every second action, check Schedule */
  if (process_schedule) {
    /* wait next timer event */
    process_schedule = false;

    for (int j = 0; j < MAX_SCHEDULE; ++j) {
      schedule_t * _schedule = CONFIG.getSchedule(j);

      if (_schedule->active && _schedule->enabled)
      {
        /* get local time and store for later use */
        time_t local_time = now();
        auto now_hour   = (uint8_t)hour(local_time);
        auto now_minute = (uint8_t)minute(local_time);
        snprintf(current_time, 6, "%02u:%02u", now_hour, now_minute);


        NTP.getTimeDateString ();

        /* every min check */
        if ( (second(local_time) == 0) ) {

          /* Count Minutes left */
          auto left = (int) (minutesLeft(local_time, _schedule->time_hour, _schedule->time_minute));

          LOG_SCHEDULE("[SCHEDULE] Now: %s Next Point: %d:%d Minutes left: %d\n", current_time,
                       _schedule->time_hour,
                       _schedule->time_minute, left);

          /* Set target duty from schedule */
          if (left == 0) {
            for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
              _leds[i].target_duty = _schedule->led_duty[i];
              _leds[i].steps_left  = 50;
              LOG_SCHEDULE("[SCHEDULE] LED%d current: %d target: %d left: %d\n", i, _leds[i].current_duty, _leds[i].target_duty, left);

            }
          }
        }
      }
    }

    /* check for pending transition */
    updateLed();

  }
}

//void ScheduleClass::loop() {
//
//  /* Every second action, check Schedule */
//  if (process_schedule) {
//    /* wait next timer event */
//    process_schedule = false;
//
//    uint8_t near_point = getNearestPoint();
//    schedule_t * _schedule = CONFIG.getSchedule(near_point);
//
//    if (_schedule->active && _schedule->enabled)
//    {
//      /* get local time */
//      time_t local_time = now();
//
//      /* every min check */
//      if ( (second(local_time) == 0) ) {
//        /* Count Minutes left */
//        auto left = (int) (minutesLeft(local_time, _schedule->time_hour, _schedule->time_minute));
//
//        /* Next day */
//        if (left < 0 && near_point == 0) {
//          left = abs(left) + 24 * 60;
//        }
//
//#ifdef DEBUG_SCHEDULE
//        char current_time_string[6];
//        getCurrentTimeString((char *) &current_time_string);
//
//        Serial.printf("[SCHEDULE] Now: %s Next: %d:%d Minutes left: %d\n", current_time_string,
//                      _schedule->time_hour,
//                      _schedule->time_minute, left);
//#endif
//
//
//
//
//        /* calc channel_transition: variant every min change duty */
//        for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {
//          uint32_t current_duty = pwm_get_duty(i);
//          uint32_t target_duty  = toPWM(_schedule->led_duty[i]);
//          _leds[i].current_duty = current_duty;
//
//#ifdef DEBUG_SCHEDULE
//          Serial.printf("[SCHEDULE] LED%d current: %d target: %d left: %d\n", i, current_duty, target_duty, left);
//#endif
//
//          if (left == 0) {
//            _leds[i].target_duty = target_duty;
//            _leds[i].steps_left = 1;
//          } else {
//            /* calculate PWM change in next min */
//            double difference = 0.0f + target_duty - current_duty;
//            double target = fabs(0.0f + _leds[i].current_duty + difference) / (left +1);
//            _leds[i].target_duty = (uint32_t) target;
//            _leds[i].steps_left = 50;
//
//          }
//        }
//      }
//    }
//
//    /* check for pending transition */
//    updateLed();
//  }
//}

void ScheduleClass::updateLed() {
  bool transition = false;

  for (auto & _led : _leds) {
    if (_led.current_duty != _led.target_duty) {
      transition = true;
    }
  }

  if (transition)
    transition_timer.attach_ms(10, std::bind(&ScheduleClass::transition, this));
}

void ScheduleClass::transition() {

  uint32_t steps_left = 0;
  for (uint8_t i = 0; i < MAX_LED_CHANNELS; ++i) {

    if (_leds[i].steps_left > 0) {
      _leds[i].steps_left--;

      if (_leds[i].steps_left == 0) {
        _leds[i].current_duty = _leds[i].target_duty;
      } else {
        double difference = (0.0f + _leds[i].target_duty - _leds[i].current_duty) / (_leds[i].steps_left + 1);
        _leds[i].current_duty = (uint32_t) fabs(0.0f + _leds[i].current_duty + difference);
      }

      steps_left = _leds[i].steps_left;
      pwm_set_duty(toPWM(_leds[i].current_duty), i);
    }
  }

  /* END Transition */
  if (steps_left == 0 ) {
    transition_timer.detach();
    publishLedStatusToMqtt();
  }

  /* Apply new pwm */
  pwm_start();

}

/* Return 0 -> 255 Duty Value*/
uint8_t ScheduleClass::getChannelDuty(uint8_t channel) {
  return _leds[channel].target_duty;
}

void ScheduleClass::setChannelDuty(uint8_t duty, uint8_t channel) {
  _leds[channel].target_duty  = duty;
  _leds[channel].steps_left   = 1;

  if (_leds[channel].target_duty != _leds[channel].current_duty) {
    _leds[channel].steps_left = 50;
  }

}

/* Convert from 0 - 255 to 0 - 5000 or 2500 PWM Duty */
uint32_t ScheduleClass::toPWM(uint8_t value) {
  return value * LIGHT_MAX_PWM / 255;
}

/* Convert from 0 - 5000 to 0 - 255 Value Duty */
uint8_t ScheduleClass::fromPWM(uint32_t pwm) {
  return pwm * 255 / LIGHT_MAX_PWM;
}

//void ScheduleClass::setScheduleDuty(schedule_t *schedule) {
//  uint32_t target       = duty * LIGHT_MAX_PWM/100;
//  uint32_t current_duty = pwm_get_duty(channell);
//
//  pwm_set_duty(target, channell);
//  pwm_start();
//}

ScheduleClass SCHEDULE;
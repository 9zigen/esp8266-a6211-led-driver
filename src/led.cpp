/***
** Created by Aleksey Volkov on 2019-05-08.
***/

#include "Arduino.h"
#include "Ticker.h"
#include "settings.h"
#include "led.h"


uint8_t modes[] = {
    0B00000000, /* Off */
    0B11111111, /* On */
    0B00001111, /* Half second blinking */
    0B00000001, /* Short flash once per second */
    0B00000101, /* Two short flashes once a second */
    0B00010101, /* Three short flashes once a second */
    0B01010101  /* Frequent short flashes (4 times per second) */
};
uint8_t blink_loop = 0;
uint8_t blink_mode = 0;

void LEDClass::init() {
  /* Setup Led Info */
  pinMode(INFO_LED_PIN, OUTPUT);

  /* Setup blink timer 125 msec period */
  led_blink_timer.once_ms(125, std::bind(&LEDClass::refresh, this));

  /* Defaul mode */
  blink_mode = modes[TWO_SHORT_BLINK];
}

void LEDClass::refresh() {
  /* iterate on every bit "blink_mode", 1 = LED ON, 0 = LED OFF */
  if( blink_mode & 1<<(blink_loop & 0x07) ) {
    digitalWrite(INFO_LED_PIN, LOW);
  } else {
    digitalWrite(INFO_LED_PIN, HIGH);
  }

  if ((blink_loop & 0x07) == 7) {
    /* pause */
    led_blink_timer.once_ms(5000, std::bind(&LEDClass::refresh, this));
  } else {
    led_blink_timer.once_ms(125, std::bind(&LEDClass::refresh, this));
  }

  blink_loop++;
}

void LEDClass::setMode(led_mode_t mode) {
  blink_mode = modes[mode];
}

LEDClass LED;
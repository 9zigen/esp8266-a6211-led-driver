/*
 * Copyright (C) 2016 Stefan Brüns <stefan.bruens@rwth-aachen.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef HV_CC_LED_DRIVER_PWM_H
#define HV_CC_LED_DRIVER_PWM_H

//#define LIGHT_MAX_PWM 5000 // * 200ns ^= 1 kHz
#define LIGHT_MAX_PWM 2500 // * 200ns ^= 1 kHz

/*SUPPORT UP TO 8 PWM CHANNEL*/
#ifndef PWM_CHANNEL_NUM_MAX
#define PWM_CHANNEL_NUM_MAX 8
#endif

struct pwm_param {
  uint32 period;
  uint32 freq;
  uint32  duty[PWM_CHANNEL_NUM_MAX];  //PWM_CHANNEL<=8
};

/* pwm_init should be called only once, for now  */
void pwm_init(uint32 period, uint32 *duty,uint32 pwm_channel_num,uint32 (*pin_info_list)[3]);
void pwm_start(void);

void pwm_set_duty(uint32 duty, uint8 channel);
uint32 pwm_get_duty(uint8 channel);
void pwm_set_period(uint32 period);
uint32 pwm_get_period(void);

uint32 get_pwm_version(void);
void set_pwm_debug_en(uint8 print_en);

#endif //HV_CC_LED_DRIVER_PWM_H


//#define PWM_CHANNELS MAX_LED_CHANNELS
//const uint32_t period = 5000; // * 200ns ^= 1 kHz
//
//// PWM setup
//uint32 io_info[PWM_CHANNELS][3] = {
//    // MUX, FUNC, PIN
//    {PERIPHS_IO_MUX_MTDI_U,  FUNC_GPIO12, 12},
//    {PERIPHS_IO_MUX_MTDO_U,  FUNC_GPIO15, 15},
//    {PERIPHS_IO_MUX_MTCK_U,  FUNC_GPIO13, 13},
//    {PERIPHS_IO_MUX_MTMS_U,  FUNC_GPIO14, 14},
//    {PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5 ,  5},
//};
//
//// initial duty: all off
//uint32 pwm_duty_init[PWM_CHANNELS] = {0, 0, 0, 0, 0};
//
//pwm_init(period, pwm_duty_init, PWM_CHANNELS, io_info);
//pwm_start();
//
//// do something like this whenever you want to change duty
//pwm_set_duty(500, 1);  // GPIO15: 10%
//pwm_set_duty(5000, 1); // GPIO15: 100%
//pwm_start();           // commit
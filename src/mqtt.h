/***
** Created by Aleksey Volkov on 2019-05-16.
***/

#ifndef HV_CC_LED_DRIVER_MQTT_H
#define HV_CC_LED_DRIVER_MQTT_H

#ifdef DEBUG_MQTT
#define LOG_MQTT(...) DEBUG_UI_PORT.printf( __VA_ARGS__ )
#else
#define LOG_MQTT(...)
#endif

void initMqtt();
void connectToMqtt();
void publishLedStatusToMqtt();
void publishDeviceStatusToMqtt();

#endif //HV_CC_LED_DRIVER_MQTT_H

/***
** Created by Aleksey Volkov on 2019-05-16.
***/

#ifndef HV_CC_LED_DRIVER_MQTT_H
#define HV_CC_LED_DRIVER_MQTT_H

void initMqtt();
void connectToMqtt();
void publishLedStatusToMqtt();
void publishDeviceStatusToMqtt();

#endif //HV_CC_LED_DRIVER_MQTT_H

# ESP8266 a6211 led-driver
esp8266 + A6211 up to 48v multichannel led driver

This is a PlatformIO project using the Arduino core on the ESP8266 as an MCU and the A6211 as a dedicated LED driver.

Hardware designed to control light in Reef Aqarium and Grow box.

Project [log](https://hackaday.io/project/165103-wifi-constant-current-led-driver)

## Features

- 6-48 volt input, can drive up to 13 serial connected led from 48v input!
- 2 kHz PWM Dimming
- Web Setup: 
  - LED
  - MQTT
  - Schedule


  ### OTA Update

  ### 2 stage for 1mb flash (esp07)
  - 1st stage
  python espota.py -d -i 192.168.2.46 -f .pio/build/5ch-esp07-1st-stage-ota/firmware.bin

  - 2nd stage 
  python espota.py -d -i 192.168.2.46 -f .pio/build/5ch-esp07-2nd-stage-ota/firmware.bin
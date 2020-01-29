#ifndef HV_CC_LED_DRIVER_SETTINGS_H
#define HV_CC_LED_DRIVER_SETTINGS_H

#if defined(ESP8266) || defined(ESP32)
#include "Arduino.h"
#include "stdlib_noniso.h"
#endif

#if defined(ESP8266)
  #define HARDWARE "ESP8266"
#elif defined(ESP32)
  #define HARDWARE "ESP32"
#endif

#ifdef DEBUG_EEPROM
#define LOG_EEPROM(...) DEBUG_UI_PORT.printf( __VA_ARGS__ )
#else
#define LOG_EEPROM(...)
#endif

#define INFO_LED_PIN 2
#define EEPROM_SIZE 1024

/* EEPROM MAP */
/* |--  Version  --|--  Auth   --|--  Network   --|--  Services   --|--  LED        --|--  Schedule   --|
 * |--  1x8b     --|--  64x8b  --|--  98x8b     --|--  63x8       --|--  5x13x8b    --|--  12x10x8b   --|
 * |--  offset 4 --|--  offset --|--  offset 97 --|--  offset 256 --|--  offset 512 --|--  offset 640 --|
 * */

/* Auth ----------------------
 * 32 bytes of eeprom */
#define AUTH_OFFSET         32
#define AUTH_LOGIN          "admin"
#define AUTH_PASSWORD       "12345678" // Web and WIFI AP Password


typedef struct {
  uint8_t magic_number;
  char login[32];             // Web Interface and OTA Update Login
  char password[32];          // Web Interface and OTA Update Password
} auth_t;

/* Network ------------------
 * 49 * 2
 * 98 bytes of eeprom */
#define MAX_NETWORKS        2
#define NETWORK_OFFSET      90

typedef struct {
  uint8_t magic_number;
  char ssid[32];                // Wifi SSID Name
  char password[32];            // Wifi Password
  uint8_t ip_address[4];        // IP Address
  uint8_t mask[4];              // Mask
  uint8_t gateway[4];           // Gateway
  uint8_t dns[4];               // DNS
  bool dhcp;                    // Enable DHCP Client
  bool active;                  // Need send by WS to GUI
} network_t;

/* Services ------------------
 * 63 bytes of eeprom */
#define SERVICES_OFFSET     256

typedef struct {
  uint8_t magic_number;
  char hostname[20];            // Device Name
  char ntp_server[20];          // Wifi SSID Name
  int16_t utc_offset;           // UTC offset in minutes
  bool ntp_dst;                 // Daylight save
  uint8_t mqtt_server[4];       // IP v4 Address Array
  uint16_t mqtt_port;           // MQTT Server port 1883 default
  char mqtt_user[16];           // 16 Char MAX
  char mqtt_password[16];       // 16 Char MAX
  uint8_t mqtt_qos;
  bool enable_ntp;              // Enable NTP Service
  bool enable_mqtt;             // Enable MQTT Service
} services_t;

/* Led -----------------------
 * 5 * 3
 * 15 bytes of eeprom
 * */
#define LED_OFFSET          512 // Led struct eeprom address
#ifndef MAX_LED_CHANNELS
  #define MAX_LED_CHANNELS   3   // 8 - MAX
#endif
#if MAX_LED_CHANNELS == 3
  #define LED_CH0_PIN         14  // LED 1 PWM pin
  #define LED_CH1_PIN         12  // LED 2 PWM pin
  #define LED_CH2_PIN         13  // LED 3 PWM pin
#elif MAX_LED_CHANNELS == 5
  #define LED_CH0_PIN         5   // LED 1 PWM pin
  #define LED_CH1_PIN         4   // LED 2 PWM pin
  #define LED_CH2_PIN         13  // LED 3 PWM pin
  #define LED_CH3_PIN         12  // LED 4 PWM pin
  #define LED_CH4_PIN         14  // LED 5 PWM pin
#endif

typedef struct {
  uint8_t magic_number;
  char color[8];                // RGB CSS Hex value FFFFFF -> white
  uint8_t last_duty;            // On Boot led duty in percentage (0-100%)
  uint16_t power;               // Real Channel Power in Watts x 10 (0-65535) 100 = 10.0 in Web UI
  uint8_t state;                // Enable/Disable channel
} led_t;

/* Schedule ------------------
 * 9 * 10
 * 120 bytes of eeprom */
#define MAX_SCHEDULE        10
#define SCHEDULE_OFFSET     640

typedef struct {
  uint8_t  magic_number;
  uint8_t  time_hour;                   // Schedule fire hour
  uint8_t  time_minute;                 // Schedule fire minutes
  uint8_t  channel[MAX_LED_CHANNELS];   // Channel Duty in percentage (0-100%)
  uint8_t  brightness;                  // All channels brightness in percentage (0-100%)
  bool     active;                      // Need send by WS to GUI
} schedule_t;

class Settings {

  public:
    void init();
    void setSettings();
    void updateAuth();
    static void erase();

    /* NTP */
    String getNtpServerName();
    int16_t getNtpOffset();

    /* LED */
    led_t * getLED(uint8_t id);

    /* Schedule */
    schedule_t * getSchedule(uint8_t id);

    /* Network */
    network_t * getNetwork(uint8_t id);

    /* Service */
    services_t * getService();

    /* Auth */
    auth_t * getAuth();

    /* Hostname */
    char * getHostname();

  private:
      auth_t auth = {};;                          // Auth initialization
      network_t network[MAX_NETWORKS] = {};       // Network initialization
      services_t service = {};                    // Services initialization
      schedule_t schedule[MAX_SCHEDULE] = {};     // Schedule initialization
      led_t led[MAX_LED_CHANNELS] = {};           // Led initialization
      bool _update_requested = false;

      /* save to internal flash */
      bool save();
};

extern Settings CONFIG;

#endif //HV_CC_LED_DRIVER_SETTINGS_H
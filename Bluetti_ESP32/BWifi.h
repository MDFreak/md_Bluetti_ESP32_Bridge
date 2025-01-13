/* -- BWifi.h -- MD0.0.1----------------------------------------------------------------------*/
#ifndef BWIFI_H
  #define BWIFI_H
  #include "Arduino.h"
  #include "config.h"

  typedef struct{
    int  salt = EEPROM_SALT;
    //char mqtt_server[40] = "127.0.0.1";
    char mqtt_server[40] = "10.0.0.111";
    char mqtt_port[6]  = "1883";
    char mqtt_username[40] = "MQTT";
    char mqtt_password[40] = "MQTT-Tiny";
    char bluetti_device_id[40] = "AC300-1";
    char ota_username[40] = "";
    char ota_password[40] = "";
  } ESPBluettiSettings;

  extern ESPBluettiSettings get_esp32_bluetti_settings();
  extern void initBWifi(bool resetWifi);
  extern void handleWebserver();
  String processorWebsiteUpdates(const String& var);
  extern void AddtoMsgView(String data);
#endif
/* MD0.0.1 - 2025-01-11 - md - initial version
 * - new define USE_DISPLAY (-> platform.ini)
 *   ndef USE_DISPLAY = no display implemented
 * - change code format to MD format for better readability
 *///------------------------------------------------------------------------------------

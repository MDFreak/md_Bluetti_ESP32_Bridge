#include "BWifi.h"
#include "BTooth.h"
#include "MQTT.h"
#include "config.h"
#ifdef USE_DISPLAY
    #include "display.h"
  #endif
unsigned long lastTime1 = 0;
unsigned long timerDelay1 = 3000;

void setup()
  {
    Serial.begin(115200);
    #ifdef RELAISMODE
        pinMode(RELAIS_PIN, OUTPUT);
        #ifdef DEBUG
            Serial.println(F("deactivate relais contact"));
          #endif
        digitalWrite(RELAIS_PIN, RELAIS_LOW);
      #endif
    #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
        esp_sleep_enable_timer_wakeup(SLEEP_TIME_ON_BT_NOT_AVAIL * 60 * 1000000ULL);
      #endif
    #ifdef USE_DISPLAY
        #ifdef DISPLAYSSD1306
            initDisplay();
          #endif
      #endif
    #ifdef DEBUG
        Serial.println(F("vor initBWifi"));
      #endif
    initBWifi(false);
    #ifdef DEBUG
        Serial.println(F("nach initBWifi"));
      #endif
    initBluetooth();
    initMQTT();
    #ifdef USE_DISPLAY
        #ifdef DISPLAYSSD1306
            wrDisp_Status("Running!");
          #endif
      #endif
  }
void loop()
  {
    #ifdef USE_DISPLAY
        #ifdef DISPLAYSSD1306
            handleDisplay();
          #endif
      #endif
    handleBluetooth();
    handleMQTT();
    handleWebserver();
  }
/* - changelog --------------------------------------------------------------------------
 * MD0.0.1 - 2025-01-11 - md - initial version
 *
 * - new define USE_DISPLAY (-> platform.ini)
 *   ndef USE_DISPLAY = no display implemented
 * - change code format to MD format for better readability
 * ------------------------------------------------------------------------------------- */

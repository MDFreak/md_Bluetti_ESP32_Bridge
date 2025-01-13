/* -- BTooth.cpp -- MD0.0.2----------------------------------------------------------------------*/
#include "BluettiConfig.h"
#include "BTooth.h"
#include "utils.h"
#include "PayloadParser.h"
#include "BWifi.h"
#include "MQTT.h"
#ifdef USE_DISPLAY
    #include "display.h"
  #endif
int pollTick = 0;
#ifdef SIM_BLUETTI
    uint8_t simTick = 0;
  #endif
struct command_handle
  {
    uint8_t page;
    uint8_t offset;
    int length;
  };

QueueHandle_t commandHandleQueue;
QueueHandle_t sendQueue;

unsigned long lastBTMessage = 0;

class MyClientCallback : public BLEClientCallbacks
  {
    void onConnect(BLEClient* pclient)
      {
        Serial.println(F("BLE - onConnect"));
        #ifdef DISPLAYSSD1306
            disp_setBlueTooth(true);
          #endif
      }
    void onDisconnect(BLEClient* pclient)
      {
        connected = false;
        Serial.println(F("BLE - onDisconnect"));
        #ifdef DISPLAYSSD1306
            disp_setBlueTooth(false);
          #endif
        #ifdef RELAISMODE
            #ifdef DEBUG
                Serial.println(F("deactivate relais contact"));
              #endif
            digitalWrite(RELAIS_PIN, RELAIS_LOW);
          #endif
      }
  };
/* Scan for BLE servers and find the first one that advertises the service we are looking for */
class BluettiAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks
  {
    /* Called for each advertising BLE server. */
    void onResult(BLEAdvertisedDevice *advertisedDevice)
      {
        Serial.print(F("[BLE] Advertised Device found: "));
        Serial.println(advertisedDevice->toString().c_str());

        ESPBluettiSettings settings = get_esp32_bluetti_settings();
        // We have found a device, let us now see if it contains the service we are looking for.
        if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(serviceUUID) && (strcmp(advertisedDevice->getName().c_str(),settings.bluetti_device_id)==0) )
          {
            BLEDevice::getScan()->stop();
            bluettiDevice = advertisedDevice;
            doConnect = true;
            doScan = true;
          }
      }
  };
void initBluetooth()
  {
    BLEDevice::init("");
    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new BluettiAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);

    commandHandleQueue = xQueueCreate( 5, sizeof(bt_command_t ) );
    sendQueue = xQueueCreate( 5, sizeof(bt_command_t) );
  }
static void notifyCallback(
    BLERemoteCharacteristic* pBLERemoteCharacteristic,
    uint8_t* pData,
    size_t length,
    bool isNotify)
  {
    #ifdef DEBUG
        Serial.println("[BT] F01 - Write Response");
        /* pData Debug... */
        for (int i=1; i<=length; i++)
          {
            Serial.printf("%02x", pData[i-1]);
            if(i % 2 == 0)
              {
                Serial.print(" ");
              }
            if(i % 16 == 0)
              {
                Serial.println();
              }
          }
        Serial.println();
      #endif
    bt_command_t command_handle;
    if(xQueueReceive(commandHandleQueue, &command_handle, 500))
      {
        parse_bluetooth_data(command_handle.page, command_handle.offset, pData, length);
      }
  }

bool connectToServer()
  {
    #ifndef SIM_BLUETTI
        Serial.print(F("[BT] Forming a connection to "));
        Serial.println(bluettiDevice->getAddress().toString().c_str());
        BLEDevice::setMTU(517); // set client to request maximum MTU from server (default is 23 otherwise)
        BLEClient*  pClient  = BLEDevice::createClient();
        Serial.println(F("[BT] - Created client"));
        pClient->setClientCallbacks(new MyClientCallback());
        // Connect to the remove BLE Server.
        pClient->connect(bluettiDevice);  // if you pass BLEAdvertisedDevice instead of address, it will be recognized type of peer device address (public or private)
        Serial.println(F("[BT] - Connected to server"));
        // pClient->setMTU(517); //set client to request maximum MTU from server (default is 23 otherwise)
        // Obtain a reference to the service we are after in the remote BLE server.
        BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
        if (pRemoteService == nullptr)
          {
            Serial.print(F("[BT] Failed to find our service UUID: "));
            Serial.println(serviceUUID.toString().c_str());
            pClient->disconnect();
            return false;
          }
        Serial.println(F("[BT] - Found our service"));
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteWriteCharacteristic = pRemoteService->getCharacteristic(WRITE_UUID);
        if (pRemoteWriteCharacteristic == nullptr)
          {
            Serial.print(F("[BT] Failed to find our characteristic UUID: "));
            Serial.println(WRITE_UUID.toString().c_str());
            pClient->disconnect();
            return false;
          }
        Serial.println(F("[BT] - Found our Write characteristic"));
        // Obtain a reference to the characteristic in the service of the remote BLE server.
        pRemoteNotifyCharacteristic = pRemoteService->getCharacteristic(NOTIFY_UUID);
        if (pRemoteNotifyCharacteristic == nullptr)
          {
            Serial.print(F("[BT] Failed to find our characteristic UUID: "));
            Serial.println(NOTIFY_UUID.toString().c_str());
            pClient->disconnect();
            return false;
          }
        Serial.println(F("[BT] - Found our Write characteristic"));
        // Read the value of the characteristic.
        if(pRemoteWriteCharacteristic->canRead())
          {
            std::string value = pRemoteWriteCharacteristic->readValue();
            Serial.print(F("[BT] The characteristic value was: "));
            Serial.println(value.c_str());
          }

        if(pRemoteNotifyCharacteristic->canNotify())
          {
            pRemoteNotifyCharacteristic->registerForNotify(notifyCallback);
          }
      #else
        Serial.println(F("[BT] SIM_BLUETTI - connected to server"));
      #endif
    connected = true;
    #ifdef RELAISMODE
        #ifdef DEBUG
            Serial.println(F("[BT] activate relais contact"));
          #endif
        digitalWrite(RELAIS_PIN, RELAIS_HIGH);
      #endif

    return true;
  }
void handleBTCommandQueue()
  {
    bt_command_t command;
    if(xQueueReceive(sendQueue, &command, 0))
      {
        #ifdef DEBUG
            Serial.print("[BT] Write Request FF02 - Value: ");
            for(int i=0; i<8; i++)
              {
                 if ( i % 2 == 0){ Serial.print(" "); };
                 Serial.printf("%02x", ((uint8_t*)&command)[i]);
              }
            Serial.println("");
          #endif
        pRemoteWriteCharacteristic->writeValue((uint8_t*)&command, sizeof(command),true);
      };
  }
void sendBTCommand(bt_command_t command)
  {
      bt_command_t cmd = command;
      xQueueSend(sendQueue, &cmd, 0);
  }
#ifdef SIM_BLUETTI
    void sendSIM_data()
      {
        uint8_t val[20];
        for (int i = 0; i < 8 ; i++ ) //sizeof(bluetti_device_state) / sizeof(device_field_data_t); i++)
          {
            switch(i)
              {
                case 0: // DEVICE_TYPE - STRINGFIELD
                    sprintf((char*) val, "AC300");
                    publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    Serial.print("[BT] SIM - publish DEVICE_TYPE: ");
                    Serial.println(parse_string_field((uint8_t*) val)); Serial.println();
                  break;
                case 1: // SERIAL_NUMBER - SN_FIELD
                    val[0]=0x01; val[1]=0x02; val[2]=0x03; val[3]=0x04;
                    val[4]=0x05; val[5]=0x06; val[6]=0x07; val[7]=0x08;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_serial_field((uint8_t*) val)));
                    Serial.print("[BT] SIM - publish SERIAL_NUMBER: ");
                    Serial.println(String(parse_serial_field((uint8_t*) val),8)); Serial.println();
                  break;
                case 2: // ARM_VERSION - VERSION_FIELD
                    val[0]=0x03; val[1]=0xC7; val[2]=0x00; val[3]=simTick;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(val),2));
                    Serial.print("[BT] SIM - publish ARM_VERSION: ");
                    Serial.println(String(parse_version_field(&val[0]),2)); Serial.println();
                  break;
                case 3: // DSP_VERSION - VERSION_FIELD
                    val[0]=0x04; val[1]=0xC7; val[2]=0x00; val[3]=simTick;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(val),2));
                    Serial.print("[BT] SIM - publish DSP_VERSION: "); Serial.println();
                    Serial.println(String(parse_version_field(&val[0]),2));
                  break;
                case 4: // DC_INPUT_POWER - UINT_FIELD
                    val[0]=0; val[1]=12+simTick;   // -> 2816 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - DC_INPUT_POWER: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case 5: // AC_INPUT_POWER - UINT_FIELD
                    val[0]=0; val[1]=11+simTick;   // -> 11 (000B)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - AC_INPUT_POWER: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case 6: // AC_OUTPUT_POWER - UINT_FIELD
                    val[0]=22; val[1]=30+simTick;   // -> 5632 (1600)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - AC_OUTPUT_POWER: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case 7: // DC_OUTPUT_POWER - UINT_FIELD
                    val[0]=0; val[1]=13+simTick;   // -> 22 (0016)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - DC_OUTPUT_POWER: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case 8: // POWER_GENERATION - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 9: // TOTAL_BATTERY_PERCENT - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 10: // AC_OUTPUT_ON - BOOL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 11: // DC_OUTPUT_ON - BOOL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 12: // AC_OUTPUT_MODE - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 13: // INTERNAL_AC_VOLTAGE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 14: // INTERNAL_CURRENT_ONE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 15: // INTERNAL_POWER_ONE - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 16: // INTERNAL_AC_FREQUENCY - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 17: // INTERNAL_CURRENT_TWO - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 18: // INTERNAL_POWER_TWO - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 19: // AC_INPUT_VOLTAGE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 20: // INTERNAL_CURRENT_THREE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 21: // INTERNAL_POWER_THREE - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                case 22: // AC_INPUT_FREQUENCY - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 23: // INTERNAL_DC_INPUT_VOLTAGE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 24: // INTERNAL_DC_INPUT_POWER - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 25: // INTERNAL_DC_INPUT_CURRENT - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 26: // PACK_NUM_MAX - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 27: // INTERNAL_PACK_VOLTAGE - DECIMAL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 28: // PACK_NUM - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 29: // PACK_BATTERY_PERCENT - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 30: // UPS_MODE - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 31: // GRID_CHARGE_ON - BOOL_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;
                case 32: // AUTO_SLEEP_MODE - UINT_FIELD
                    //sprintf(val, "AC300");
                    //publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                    //Serial.print(millis()); Serial.print("[BT] SIM - DEVICE_TYPE: "); Serial.println(val);
                  break;

              }
          }
        simTick++;
        if (simTick >= 8) { simTick = 0; }
      }
  #endif
void handleBluetooth()
  {
    #ifdef SIM_BLUETTI
        doConnect = false;
        connected = true;
      #endif
    if (doConnect == true)
      {
        if (connectToServer())
          {
            Serial.println(F("We are now connected to the Bluetti BLE Server."));
          }
          else
          {
            Serial.println(F("We have failed to connect to the server; there is nothing more we will do."));
          }
        doConnect = false;
      }
    #ifndef SIM_BLUETTI
        if ((millis() - lastBTMessage) > (MAX_DISCONNECTED_TIME_UNTIL_REBOOT * 60000))
          {
            Serial.println(F("[BT] disconnected over allowed limit, reboot device"));
            #ifdef SLEEP_TIME_ON_BT_NOT_AVAIL
                esp_deep_sleep_start();
              #else
                ESP.restart();
              #endif
          }
      #endif
    if (connected)
      {
        // poll for device state
        if ( millis() - lastBTMessage > BLUETOOTH_QUERY_MESSAGE_DELAY)
          {
            #ifndef SIM_BLUETTI
                bt_command_t command;
                command.prefix = 0x01;
                command.field_update_cmd = 0x03;
                command.page = bluetti_polling_command[pollTick].f_page;
                command.offset = bluetti_polling_command[pollTick].f_offset;
                command.len = (uint16_t) bluetti_polling_command[pollTick].f_size << 8;
                command.check_sum = modbus_crc((uint8_t*)&command,6);

                xQueueSend(commandHandleQueue, &command, portMAX_DELAY);
                xQueueSend(sendQueue, &command, portMAX_DELAY);
              #else
                Serial.print(millis()); Serial.print(" -> sendSIM_data "); Serial.println(pollTick);
                sendSIM_data();
              #endif
            if (pollTick == sizeof(bluetti_polling_command)/sizeof(device_field_data_t)-1 )
              { pollTick = 0; }
              else
              { pollTick++; }
            #ifndef SIM_BLUETTI
                handleBTCommandQueue();
              #endif
            lastBTMessage = millis();
          }
      }
      else if(doScan)
      {
        BLEDevice::getScan()->start(0);
      }
  }
void btResetStack()
  {
    connected=false;
  }
bool isBTconnected()
  {
    return connected;
  }
unsigned long getLastBTMessageTime()
  {
      return lastBTMessage;
  }
// - changelog --------------------------------------------------------------------------
/* MD0.0.2 - 2025-01-13 - simuting Bluetti data for MQTT
 * - introduce simulation for BT to implement MQTT without Bluetti
 *   - new define SIM_BLUETTI (-> platform.ini)
 *     used to block unused BT functions
 *     and activate simulation function
 *   - simulation starts in function 'handleBluetooth()' and uses
 *     new function 'sendSIM_data()' to publish data
 *     uses standard decoding methods
 *   - works with 8 items
 *   - add '#include "MQTT.h"' to Bluetooth.cpp
 * - set default data for connections
 * - introduce simulation for BT to implement MQTT without Bluetti
 */// -----------------------------------------------------------------------------------
/* MD0.0.1 - 2025-01-11 - md - initial version
 * - new define USE_DISPLAY (-> platform.ini)
 *   ndef USE_DISPLAY = no display implemented
 * - change code format to MD format for better readability
 *///------------------------------------------------------------------------------------






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
    void sendSIM_data(int poll_idx)
      {
        uint8_t val[20];
        //for (int i = 0; i < sizeof(bluetti_device_state) / sizeof(device_field_data_t); i++)
        for (int i = bluetti_poll_idx[poll_idx].f_start; i <= bluetti_poll_idx[poll_idx].f_end; i++)
          {
            switch(bluetti_device_state[i].f_name)
              {
                case DEVICE_TYPE:               // STRINGFILD AC300                  //sprintf((char*) val, "AC300");
                    val[0]=0x41; val[1]=0x43; val[2]=0x33; val[3]=0x30; val[4]=0x30; val[5]=0x00;  // -> AC300
                    val[6]=0x31; val[7]=0x32; val[8]=0x33; val[9]=0x34; val[10]=0x35; val[11]=0x36;
                    publishTopic(bluetti_device_state[i].f_name, parse_string_field((uint8_t*) val));
                      //Serial.print("[BT] SIM -  publish DEVICE_TYPE: ");
                      //Serial.println(parse_string_field((uint8_t*) val)); Serial.println();
                  break;
                case ADR_0x0010_UINT:           // UINT_FIELD AC300
                    val[0]=0x03; val[1]=0xFA;    // -> 1018 (03FA)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0010_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case SERIAL_NUMBER:             // SN_FIELD   AC300
                    char sn[16];                // 2235000574654
                    val[0]=0x52; val[1]=0xBE; val[2]=0x60; val[3]=0x6A;
                    val[4]=0x02; val[5]=0x08; val[6]=0x00; val[7]=0x00;
                    sprintf(sn, "%lld", parse_serial_field(val));
                    publishTopic(bluetti_device_state[i].f_name, String(sn));
                      //Serial.print("[BT] SIM - publish SERIAL_NUMBER: ");
                      //Serial.println(String(sn)); Serial.println();
                  break;
                case ADR_0x0012_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x12+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0012_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0013_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x04;   // -> 4 (0004)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0013_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ARM_VERSION:               // VERSION_FIELD AC300
                    val[0]=0x28; val[1]=0xFB; val[2]=0x00; val[3]=0x06; // 4037.07
                    publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(val),2));
                      //Serial.print("[BT] SIM - publish ARM_VERSION: ");
                      //Serial.println(String(parse_version_field(&val[0]),2)); Serial.println();
                  break;
                case DSP_VERSION:               // VERSION_FIELD AC300
                    val[0]=0x28; val[1]=0x92; val[2]=0x00; val[3]=0x06; //4036.02
                    publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(val),2));
                      //Serial.print("[BT] SIM - publish DSP_VERSION: "); Serial.println();
                      //Serial.println(String(parse_version_field(&val[0]),2));
                  break;
                case ADR_0x001B_UINT:           // UINT_FIELD AC300
                    val[0]=0x8E; val[1]=0xDA;   // -> 36570 (8EDA)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x001B_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x001C_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x001C_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x001D_UINT:           // UINT_FIELD AC300
                    val[0]=0xc1; val[1]=0x24;   // -> 49444 (C124)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x001D_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x001E_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x0D;   // -> 13 (000D)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x001E_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x001F_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x1F + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x001F_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0020_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x20 + simTick;   // -> 32
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0020_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0021_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x21  + simTick;   // -> 33
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0021_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0022_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x22 + simTick;   // -> 34
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0022_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0023_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x23 + simTick;   // -> 35
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0023_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case DC_INPUT_POWER:            // UINT_FIELD
                    val[0]=0x04; val[1]=0x00 + simTick;   // ->1024
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - DC_INPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_INPUT_POWER:            // UINT_FIELD
                    val[0]=0; val[1]=00 + simTick;   // -> 0
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - AC_INPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_OUTPUT_POWER:           // UINT_FIELD
                    val[0]=06; val[1]=0x00 + simTick;   // -> 1536
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - AC_OUTPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case DC_OUTPUT_POWER:           // UINT_FIELD
                    val[0]=0; val[1]=0xB0+simTick;   // -> 176
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - DC_OUTPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0028_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x28 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0028_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case POWER_GENERATION:          // DECIMAL_FIELD AC300
                    val[0]=0xAF; val[1]=0x5A;   // -> 448,90 (AF5A)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale)));
                      //Serial.print("[BT] SIM - DC_OUTPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x002A_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x2A + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x002A_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case TOTAL_BATTERY_PERCENT:     // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x58;   // -> 88 (0058)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - TOTAL_BATTERY_PERCENT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x002C_UINT:           // UINT_FIELD ac300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x002C_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x002D_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x2D + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x002D_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x002E_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x002E_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x002F_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x2F + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x002F_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_OUTPUT_ON:              // BOOL_FIELD
                    val[0]=0x00; val[1]=0x00 + (simTick > 0);   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - AC_OUTPUT_ON: ");
                      //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case DC_OUTPUT_ON:              // BOOL_FIELD
                    val[0]=0x00; val[1]=0x00 + (simTick > 0);   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      // Serial.print("[BT] SIM - DC_OUTPUT_ON: ");
                      // Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case AC_OUTPUT_MODE:            // ENUM_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick/2;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_enum_field(val,bluetti_device_state[i].f_type)));
                      //Serial.print("[BT] SIM - AC_OUTPUT_MODE: ");
                      //Serial.println(String(parse_enum_field(val, bluetti_device_state[i].f_type))); Serial.println();
                  break;
                case INTERNAL_AC_VOLTAGE:       // DECIMAL_FIELD AC300
                    val[0]=0x09; val[1]=0x02;   // -> 230,6 (0902)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale )));
                      //Serial.print("[BT] SIM - INTERNAL_AC_VOLTAGE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_CURRENT_ONE:      // DECIMAL_FIELD AC300
                    val[0]=0x01; val[1]=0x93;   // -> 4,30 (0193)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - INTERNAL_CURRENT_ONE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_POWER_ONE:        // UINT_FIELD AC300
                    val[0]=0x03; val[1]=0x65 + simTick;   // -> 869 (0365)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - INTERNAL_POWER_ONE: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case INTERNAL_CURRENT_TWO:      // DECIMAL_FIELD AC300
                    val[0]=0x01; val[1]=0xC2;   // -> 4,50 (01C2)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - INTERNAL_CURRENT_TWO: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_POWER_TWO:        // UINT_FIELD AC300
                    val[0]=0x1F; val[1]=0x50 + simTick;   // -> 1066 (1F50)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - INTERNAL_POWER_TWO: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_INPUT_VOLTAGE:          // DECIMAL_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - AC_INPUT_VOLTAGE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_CURRENT_THREE:    // DECIMAL_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - INTERNAL_CURRENT_THREE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_POWER_THREE:      // UINT_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - INTERNAL_POWER_THREE: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_INPUT_FREQUENCY:        // DECIMAL_FIELD
                    val[0]=0x13; val[1]=0x88 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - AC_INPUT_FREQUENCY: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case ADR_0x0051_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x51 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0051_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0052_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x52 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0052_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0053_UINT:           // UINT_FIELD  AC300
                    val[0]=0x00; val[1]=0x83;   // -> 131 (0083)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0053_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0054_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x11;   // -> 11 (0011)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0054_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0055_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x0E;   // -> 14 (000E)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0055_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case INTERNAL_DC_INPUT_VOLTAGE: // DECIMAL_FIELD AC300
                    val[0]=0x28; val[1]=0x04 + simTick;   // -> 101,40 (2804)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - INTERNAL_DC_INPUT_VOLTAGE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case INTERNAL_DC_INPUT_POWER:   // UINT_FIELD AC300
                    val[0]=0x08; val[1]=0x00 + simTick;   // -> 69 (0800)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - INTERNAL_DC_INPUT_POWER: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case INTERNAL_DC_INPUT_CURRENT: // DECIMAL_FIELD
                    val[0]=0x00; val[1]=0x3C;   // -> 0,60 (003C)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) );
                      //Serial.print("[BT] SIM - INTERNAL_AC_INTERNAL_DC_INPUT_CURRENTVOLTAGE: ");
                      //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case ADR_0x0059_UINT:           // UINT_FIELD AC300
                    val[0]=0x11; val[1]=0x89 + simTick;   // -> 4489 (1189)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0059_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x005A_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x5A + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x005A_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case PACK_NUM_MAX:              // UINT_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - PACK_NUM_MAX: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case TOTAL_BATTERY_VOLTAGE:     // DECIMAL_FIELD
                    val[0]=0x00; val[1]=0x80+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale )));
                    //Serial.print("[BT] SIM - TOTAL_BATTERY_VOLTAGE: ");
                    //Serial.println(String(parse_decimal_field(val, bluetti_device_state[i].f_scale ), 2) ); Serial.println();
                  break;
                case TOTAL_BATTERY_CURRENT:     // UINT_FIELD
                    val[0]=0x04; val[1]=0xA5+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - TOTAL_BATTERY_CURRENT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x005E_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x5E + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x005E_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x005F_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x5F + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x005F_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case PACK_NUM:                  // UINT_FIELD
                    val[0]=0x00; val[1]=0x00+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - PACK_NUM: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case PACK_STATUS:               // ENUM_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick/2;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_enum_field(val, bluetti_device_state[i].f_type)));
                      //Serial.print("[BT] SIM - PACK_STATUS: ");
                      //Serial.println(String(parse_enum_field(val, bluetti_device_state[i].f_type))); Serial.println();
                  break;
                case PACK_VOLTAGE:              // DECIMAL_FIELD AC300
                    val[0]=0x14; val[1]=0xBE;   // -> 5310 (14BE)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(val, bluetti_device_state[i].f_scale)));
                      //Serial.print("[BT] SIM - PACK_VOLTAGE: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case PACK_BATTERY_PERCENT:      // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x51;   // -> 81 (0051)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - PACK_BATTERY_PERCENT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0064_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x00;   // -> 0 (0000)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0064_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0065_UINT:           // UINT_FIELD AC300
                    val[0]=0x00; val[1]=0x68;   // -> 104 (0068)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0065_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0066_UINT:           // UINT_FIELD AC300
                    val[0]=0x04; val[1]=0xB0;   // -> 1200 (04B0)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0066_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0067_UINT:           // UINT_FIELD AC300
                    val[0]=0x16; val[1]=0x80;   // -> 5760 (1680)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0067_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0068_UINT:           // UINT_FIELD AC300
                    val[0]=0x12; val[1]=0xC0;   // -> 4800 (12C0)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - ADR_0x0068_UINT: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case CELL_VOLTAGES:             // DECIMAL_ARRAY
                    val[0]=01; val[1]=0x4A; val[2]=01; val[3]=0x4A;   // -> 3,30/3,31/../3,44/3,45 (014A/014B/../0154/0155)
                    val[4]=01; val[5]=0x4B; val[6]=01; val[7]=0x4B;
                    val[8]=01; val[9]=0x4C; val[10]=01; val[11]=0x4C;
                    val[12]=01; val[13]=0x4D; val[14]=01; val[15]=0x4D;
                    publishTopic(bluetti_device_state[i].f_name,
                                 parse_decimal_array(val, bluetti_device_state[i].f_size,
                                                          bluetti_device_state[i].f_scale,
                                                          bluetti_device_state[i].f_enum  ));
                    //Serial.print("[BT] SIM - CELL_VOLTAGES: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0089_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x89 + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0089_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008A_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x8A + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008A_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008B_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x8B + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008B_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008C_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x8C + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008C_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008D_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x8D + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008D_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008E_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0xBE + simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008E_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x008F_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0x8F+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x008F_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                // page 0x0B -> 2816+
                case UPS_MODE:                  // ENUM_FIELD AC300
                    val[0]=0x00; val[1]=0x04;   // ->4 (0004)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_enum_field(val, bluetti_device_state[i].f_type)));
                      //Serial.print("[BT] SIM - UPS_MODE: ");
                      //Serial.println(String(parse_enum_field(val, bluetti_device_state[i].f_type))); Serial.println();
                  break;
                case ADR_0x0BBA_UINT:           // UINT_FIELD
                    val[0]=0x0B; val[1]=0xBA+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BBA_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0BBB_UINT:           // UINT_FIELD
                    val[0]=0x0B; val[1]=0xBB+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BBB_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case SPLIT_PHASE_ON:            // BOOL_FIELD AC300
                    val[0]=0x00; val[1]=0x00;   // -> 0 (0000)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - SPLIT_PHASE_ON: ");
                      //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case SPLIT_PHASE_MACHINE_MODE:  // UINT_FIELD
                    val[0]=0x00; val[1]=0x00+simTick/4;   // ->  (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - SPLIT_PHASE_MACHINE_MODE: ");
                      //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case PACK_NUM_SET:              // UINT_FIELD
                    val[0]=0x00; val[1]=0x00+simTick/2;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - PACK_NUM_SET: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case AC_OUTPUT_CTRL:            // BOOL_FIELD
                    val[0]=0x00; val[1]=0x00+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - AC_OUTPUT_CTRL: ");
                    //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case DC_OUTPUT_CTRL:            // BOOL_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - DC_OUTPUT_CTRL: ");
                      //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case ADR_0x0BC1_UINT:           // UINT_FIELD
                    val[0]=0x0B; val[1]=0xC1+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - ADR_0x0BC1_UINT: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case ADR_0x0BC2_UINT:           // UINT_FIELD
                    val[0]=0x0b; val[1]=0xC2+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - ADR_0x0BC2_UINT: ");
                    Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case GRID_CHARGE_ON:            // BOOL_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    Serial.print("[BT] SIM - GRID_CHARCHE_ON: ");
                    Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case ADR_0x0BC4_UINT:           // UINT_FIELD
                    val[0]=0x00; val[1]=0xC4+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BC4_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case TIME_CONTROL_ON:           // BOOL_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - TIME_CONTROL_ON: ");
                      //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case ADR_0x0BC6_UINT:           // UINT_FIELD
                    val[0]=0x0B; val[1]=0xC6+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BC6_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case BATTERY_RANGE_START:       // UINT_FIELD
                    val[0]=0x5A; val[1]=0xA5+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - BATTERY_RANGE_START: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case BATTERY_RANGE_END:         // UINT_FIELD
                    val[0]=0x5A; val[1]=0xA5+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - BATTERY_RANGE_END: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case  ADR_0x0BDA_UINT:          // UINT_FIELD
                    val[0]=0x0B; val[1]=0xDA+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BDA_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case  ADR_0x0BDB_UINT:          // UINT_FIELD
                    val[0]=0x0B; val[1]=0xDB+simTick;   // -> 1823 (0B00)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                    //Serial.print("[BT] SIM - ADR_0x0BDB_UINT: ");
                    //Serial.println(String(parse_uint_field(val))); Serial.println();
                  break;
                case  BLUETOOTH_CONNECTED:      // BOOL_FIELD AC300
                    val[0]=0x00; val[1]=0x01;   // -> 1 (0001)
                    publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(val)));
                      //Serial.print("[BT] SIM - BLUETOOTH_CONNECTED: ");
                      //Serial.println(String(parse_bool_field(val))); Serial.println();
                  break;
                case  AUTO_SLEEP_MODE:          // ENUM_FIELD
                    val[0]=0x00; val[1]=0x00 + simTick/2;
                    publishTopic(bluetti_device_state[i].f_name, String(parse_enum_field(val, bluetti_device_state[i].f_type)));
                    //Serial.print("[BT] SIM - AUTO_SLEEP_MODE: ");
                    //Serial.println(String(parse_enum_field(val, bluetti_device_state[i].f_type))); Serial.println();
                  break;
              }
            usleep(200000);
          }
        simTick++;
        if (simTick >= 6) { simTick = 0; }
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
                sendSIM_data( pollTick);
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






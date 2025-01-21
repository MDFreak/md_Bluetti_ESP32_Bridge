/* -- Payloadparser.cpp -- MD0.0.1----------------------------------------------------------------------*/
#include "BluettiConfig.h"
#include "MQTT.h"
#include "PayloadParser.h"
#include "BWifi.h"

uint16_t switch_bytes(uint8_t data[])
  {
    return ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  }
uint16_t parse_uint_field(uint8_t data[])
  {
    //return ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    return switch_bytes(data);
  }
bool     parse_bool_field(uint8_t data[])
  {
    return (data[1]) > 0;
  }
float    parse_decimal_field(uint8_t data[], uint8_t scale)
  {
    //uint16_t raw_value = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    return (switch_bytes(data)  / pow(10, scale));
  }
String   parse_decimal_array(uint8_t data[], uint8_t size, uint8_t scale, uint8_t type)
  {
    String tmp = "{ ";
    switch (type)
      {
        case CELL_VOLTAGES: tmp.concat(" { 'CellVolt': ["); break;
        default:            break;
      }
    for (uint8_t i = 0; i < (size * 2); i + 2)
      {
        if (i > 0) { tmp.concat(", "); }
        tmp.concat("Decimal('");
        tmp.concat(parse_decimal_field(&data[i], scale));
        tmp.concat("')");
      }
    tmp.concat("]}");
    return tmp;
        //'cell_voltage': [Decimal('3.35'), Decimal('3.36'), Decimal('3.35'), Decimal('3.35'), Decimal('3.36'), Decimal('3.35'), Decimal('3.35'), Decimal('3.35'), Decimal('3.35'), Decimal('3.35'), Decimal('3.35'), Decimal('3.36'), Decimal('3.36'), Decimal('3.35'), Decimal('3.35'),
        //                 Decimal('3.35')]}
  }

float    parse_version_field(uint8_t data[])
  { // (high16(big end) + low16(big end) -> (high16(little end) + low16(little end)) / 100
    //uint16_t low  = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    //uint16_t high = ((uint16_t)data[2] << 8) | (uint16_t)data[3];
    uint16_t high = switch_bytes(&data[0]);
    uint16_t low  = switch_bytes(&data[2]);
    long val = (low << 16) | (high);
    return (float)val / 100;
  }
uint64_t parse_serial_field(uint8_t data[])
  { // val64(big end) -> val64(little end)
    uint16_t val1 = switch_bytes(&data[0]); // ((uint16_t)data[0] << 8) | (uint16_t)data[1];
    uint16_t val2 = switch_bytes(&data[2]); // ((uint16_t)data[2] << 8) | (uint16_t)data[3];
    uint16_t val3 = switch_bytes(&data[4]); // ((uint16_t)data[4] << 8) | (uint16_t)data[5];
    uint16_t val4 = switch_bytes(&data[6]); // ((uint16_t)data[6] << 8) | (uint16_t)data[7];
    uint64_t sn = ((((uint64_t)val1) | ((uint64_t)val2 << 16)) | ((uint64_t)val3 << 32)) | ((uint64_t)val4 << 48);
    return sn;
  }
String   parse_string_field(uint8_t data[])
  {
    return String((char*)data);
  }
// not implemented yet, leads to nothing
String   parse_enum_field(uint8_t data[], uint8_t type)
  {
    if     (type == OUT_MODE_t)
      {
        uint8_t anz = ANZ_OutputMode;
        String tmp[anz] = {"STOP", "INVERTER_OUTPUT", "BYPASS_OUTPUT_C", "BYPASS_OUTPUT_D", "LOAD_MATCHING"};
        Serial.println();
        Serial.print("OUT_MODE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == UPS_MODE_t)
      {
        uint8_t anz = ANZ_UPS_Mode;
        String tmp[anz] = {"CUSTOMIZED", "PV_PRIORITY", "STANDARD", "TIME_CONTROl"};
        Serial.println();
        Serial.print("UPS_MODE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == BATT_STATE_t)
      {
        uint8_t anz = ANZ_BatteryState;
        String tmp[anz] = {"STANDBY", "CHARGE", "DISCHARGE"};
        Serial.println();
        Serial.print("BATT_STATE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == SLAVE_TYPE_t)
      {
        uint8_t anz = ANZ_MasterType;
        String tmp[anz] = {"SLAVE", "MASTER"};
        Serial.println();
        Serial.print("SLAVE_TYPE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == AUTO_SLEEP_t)
      {
        uint8_t anz = ANZ_AutoSleepMode;
        String tmp[anz] = {"THIRTY_SECONDS", "ONE_MINNUTE", "FIVE_MINUTES", "NEVER"};
        Serial.println();
        Serial.print("AUTO_SLEEP_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == LED_MODE_t)
      {
        uint8_t anz = ANZ_LedMode;
        String tmp[anz] = {"LED_LOW", "LED_HIGH", "LED_SOS", "LED_OFF"};
        Serial.println();
        Serial.print("LED_MODE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == SHUTDOWN_t)
      {
        uint8_t anz = ANZ_EcoShutdown;
        String tmp[anz] = {"ONE_HOUR", "TWO_HOURS", "THREE_HOURS", "FOUR_HOURS"};
        Serial.println();
        Serial.print("SHUTDOWN_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    else if(type == CHARG_MODE_t)
      {
        uint8_t anz = ANZ_ChargingMode;
        String tmp[anz] = {"STANDARD", "SILENT", "TURBO"};
        Serial.println();
        Serial.print("CHARG_MODE_t: "); Serial.println(tmp[data[1]]);
        Serial.println();
        if (data[1] < anz) { return tmp[data[1]]; };
      }
    return "";
  }
void     parse_bluetooth_data(uint8_t page, uint8_t offset, uint8_t* pData, size_t length)
  {
    char Byte_In_Hex_offset[3];
    char Byte_In_Hex_page[3];
    sprintf(Byte_In_Hex_offset, "%x", offset);
    sprintf(Byte_In_Hex_page, "%x", page);

    switch(pData[1])  // ignore the first byte, it's fix 0x01
      {
        // range request
        case 0x03:
           for (int i = 0; i < sizeof(bluetti_device_state) / sizeof(device_field_data_t); i++)
            {
              // filter fields not in range, reworked by https://github.com/AlexBurghardt
              // the original code didn't work completely and skipped some fields to be published
              if  (    bluetti_device_state[i].f_page == page     // it's the correct page
                    && bluetti_device_state[i].f_offset >= offset // data offset greater than or equal to page offset
                                                                  // local offset does not exceed the page length,
                                                                  // likely not needed because of the last condition check
                    && ((2* ((int)bluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE) <= length
                                                                  // local offset + data size do not exceed the page length
                    && ((2* ((int)bluetti_device_state[i].f_offset - (int)offset + bluetti_device_state[i].f_size)) + HEADER_SIZE) <= length
                  )
                {
                  uint8_t data_start = (2* ((int)bluetti_device_state[i].f_offset - (int)offset)) + HEADER_SIZE;
                  uint8_t data_end = (data_start + 2 * bluetti_device_state[i].f_size);
                  uint8_t data_payload_field[data_end - data_start] ;
                  int p_index = 0;
                  for (int i=data_start; i<= data_end; i++)
                    {
                      data_payload_field[p_index] = pData[i-1];
                      p_index++;
                    }
                  switch (bluetti_device_state[i].f_type)
                    {
                      case UINT_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, String(parse_uint_field(data_payload_field)));
                        break;
                      case BOOL_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, String((int)parse_bool_field(data_payload_field)));
                        break;
                      case DECIMAL_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, String(parse_decimal_field(data_payload_field, bluetti_device_state[i].f_scale ), 2) );
                        break;
                      case SN_FIELD:
                        char sn[16];
                        sprintf(sn, "%lld", parse_serial_field(data_payload_field));
                        publishTopic(bluetti_device_state[i].f_name, String(sn));
                        break;
                      case VERSION_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, String(parse_version_field(data_payload_field),2) );
                        break;
                      case STRING_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, parse_string_field(data_payload_field));
                        break;
                      // doesn't work yet, not implemented further
                      case DECIMAL_ARRAY:
                        break;
                      case ENUM_FIELD:
                        publishTopic(bluetti_device_state[i].f_name, parse_enum_field(data_payload_field,bluetti_device_state[i].f_type));
                        break;
                      default:
                        break;
                    }
                }
                else
                {
                  /* causes way too many messages, for debugging only
                  //AddtoMsgView(String(millis()) + ": skip filtered field: "+ Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
                  */
                }
            }
            break;
        case 0x06:
          AddtoMsgView(String(millis()) + ":skip 0x06 request! page: " + Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
          break;
        default:
          AddtoMsgView(String(millis()) + ":skip unknow request! page: " + Byte_In_Hex_page + " offset: " + Byte_In_Hex_offset);
          break;
      }
  }
/* MD0.0.1 - 2025-01-11 - md - initial version
 * - new define USE_DISPLAY (-> platform.ini)
 *   ndef USE_DISPLAY = no display implemented
 * - change code format to MD format for better readability
 *///------------------------------------------------------------------------------------

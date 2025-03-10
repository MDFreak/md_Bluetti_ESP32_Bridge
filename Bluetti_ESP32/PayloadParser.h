#ifndef PAYLOAD_PARSER_H
  #define PAYLOAD_PARSER_H
  #include "Arduino.h"
  #include "DeviceType.h"

  #define HEADER_SIZE 4
  #define CHECKSUM_SIZE 2

  uint16_t parse_uint_field(uint8_t data[]);
  bool     parse_bool_field(uint8_t data[]);
  float    parse_decimal_field(uint8_t data[], uint8_t scale);
  String   parse_decimal_array(uint8_t data[], uint8_t size, uint8_t scale, uint8_t type);
  uint64_t parse_serial_field(uint8_t data[]);
  float    parse_version_field(uint8_t data[]);
  String   parse_string_field(uint8_t data[]);
  String   parse_enum_field(uint8_t data[],uint8_t type);

  extern void parse_bluetooth_data(uint8_t page, uint8_t offset, uint8_t* pData, size_t length);
#endif
/* - changelog --------------------------------------------------------------------------
 * MD0.0.1 - 2025-01-11 - md - initial version
 *
 * - change code format to MD format for better readability
 * ------------------------------------------------------------------------------------- */

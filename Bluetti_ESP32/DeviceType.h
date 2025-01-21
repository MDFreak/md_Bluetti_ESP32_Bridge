/* -- DeviceType.h -- MD0.0.1 ---------------------------------------------------------*/
#ifndef __DEVICE_TYPE_H__
  #define __DEVICE_TYPE_H__
  #include "Arduino.h"

  enum field_types {
      UINT_FIELD,
      BOOL_FIELD,
      ENUM_FIELD,
      STRING_FIELD,
      DECIMAL_ARRAY,
      DECIMAL_FIELD,
      VERSION_FIELD,
      SN_FIELD,
      TYPE_UNDEFINED
   };
  enum field_names {
      DEVICE_TYPE,
      ADR_0x0010_UINT,
      SERIAL_NUMBER,
      ADR_0x0012_UINT,
      ADR_0x0013_UINT,
      ARM_VERSION,
      DSP_VERSION,
      ADR_0x001B_UINT,
      ADR_0x001C_UINT,
      ADR_0x001D_UINT,
      ADR_0x001E_UINT,
      ADR_0x001F_UINT,
      ADR_0x0020_UINT,
      ADR_0x0021_UINT,
      ADR_0x0022_UINT,
      ADR_0x0023_UINT,
      DC_INPUT_POWER,
      AC_INPUT_POWER,
      AC_OUTPUT_POWER,
      DC_OUTPUT_POWER,
      ADR_0x0028_UINT,
      POWER_GENERATION,
      ADR_0x002A_UINT,
      TOTAL_BATTERY_PERCENT,
      ADR_0x002C_UINT,
      ADR_0x002D_UINT,
      ADR_0x002E_UINT,
      ADR_0x002F_UINT,
      DC_OUTPUT_ON,
      AC_OUTPUT_ON,

      AC_OUTPUT_MODE,
      INTERNAL_AC_VOLTAGE,
      INTERNAL_CURRENT_ONE,
      INTERNAL_POWER_ONE,
      INTERNAL_AC_FREQUENCY,
      INTERNAL_CURRENT_TWO,
      INTERNAL_POWER_TWO,
      AC_INPUT_VOLTAGE,
      INTERNAL_CURRENT_THREE,
      INTERNAL_POWER_THREE,
      AC_INPUT_FREQUENCY,
      ADR_0x0051_UINT,
      ADR_0x0052_UINT,
      ADR_0x0053_UINT,
      ADR_0x0054_UINT,
      ADR_0x0055_UINT,
      INTERNAL_DC_INPUT_VOLTAGE,
      INTERNAL_DC_INPUT_POWER,
      INTERNAL_DC_INPUT_CURRENT,
      ADR_0x0059_UINT,
      ADR_0x005A_UINT,
      PACK_NUM_MAX,
      TOTAL_BATTERY_VOLTAGE,
      TOTAL_BATTERY_CURRENT,
      ADR_0x005E_UINT,
      ADR_0x005F_UINT,
      PACK_NUM,
      PACK_STATUS,
      PACK_VOLTAGE,
      PACK_BATTERY_PERCENT,
      ADR_0x0064_UINT,
      ADR_0x0065_UINT,
      ADR_0x0066_UINT,
      ADR_0x0067_UINT,
      ADR_0x0068_UINT,
      CELL_VOLTAGES,
      ADR_0x0089_UINT,
      ADR_0x008A_UINT,
      ADR_0x008B_UINT,
      ADR_0x008C_UINT,
      ADR_0x008D_UINT,
      ADR_0x008E_UINT,
      ADR_0x008F_UINT,

      UPS_MODE,
      ADR_0x0BBA_UINT,
      ADR_0x0BBB_UINT,
      SPLIT_PHASE_ON,
      SPLIT_PHASE_MACHINE_MODE,
      PACK_NUM_SET,
      AC_OUTPUT_CTRL,
      DC_OUTPUT_CTRL,
      ADR_0x0BC1_UINT,
      ADR_0x0BC2_UINT,
      GRID_CHARGE_ON,
      ADR_0x0BC4_UINT,
      TIME_CONTROL_ON,
      ADR_0x0BC6_UINT,
      BATTERY_RANGE_START,
      BATTERY_RANGE_END,

      ADR_0x0BDA_UINT,
      ADR_0x0BDB_UINT,
      BLUETOOTH_CONNECTED,
      AUTO_SLEEP_MODE,

      INTERNAL_CELL01_VOLTAGE,
      INTERNAL_CELL02_VOLTAGE,
      INTERNAL_CELL03_VOLTAGE,
      INTERNAL_CELL04_VOLTAGE,
      INTERNAL_CELL05_VOLTAGE,
      INTERNAL_CELL06_VOLTAGE,
      INTERNAL_CELL07_VOLTAGE,
      INTERNAL_CELL08_VOLTAGE,
      INTERNAL_CELL09_VOLTAGE,
      INTERNAL_CELL10_VOLTAGE,
      INTERNAL_CELL11_VOLTAGE,
      INTERNAL_CELL12_VOLTAGE,
      INTERNAL_CELL13_VOLTAGE,
      INTERNAL_CELL14_VOLTAGE,
      INTERNAL_CELL15_VOLTAGE,
      INTERNAL_CELL16_VOLTAGE,
      LED_MODE,
      POWER_OFF,
      ECO_ON,
      ECO_SHUTDOWN,
      CHARGING_MODE,
      POWER_LIFTING_ON,
      AC_INPUT_POWER_MAX,
      AC_INPUT_CURRENT_MAX,
      AC_OUTPUT_POWER_MAX,
      AC_OUTPUT_CURRENT_MAX,
      BATTERY_MIN_PERCENTAGE,   // Discharge lower limit
      AC_CHARGE_MAX_PERCENTAGE,  // Percentage to which point battery will be charged with AC
      AUTOSLEEP,
      ADR_0x0BF6_UINT,
      ADR_0x0BF7_UINT,
      ADR_0x0BF8_UINT,
      ADR_0x0BF9_UINT,
      ADR_0x0BFA_UINT,
      ADR_0x0BFB_UINT,
      FIELD_UNDEFINED,
      FIELD_MAX
  };
  typedef struct device_field_data {
      enum field_names f_name;
      uint8_t f_page;
      uint8_t f_offset;
      int8_t f_size;
      int8_t f_scale;
      int8_t f_enum;
      enum field_types f_type;
  } device_field_data_t;
  #ifdef SIM_BLUETTI
      typedef struct device_field_idx {
          uint8_t f_start;
          uint8_t f_end;
      } device_field_idx_t;
    #endif
#endif

/* MD0.0.1 - 2025-01-11 - md - initial version
 * - change code format to MD format for better readability
 *///------------------------------------------------------------------------------------

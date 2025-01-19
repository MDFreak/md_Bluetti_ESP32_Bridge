/* -- Device_AC300.h -- MD0.0.1 --------------------------------------------------------------------*/
#ifdef AC300_ACTIVE
  #ifndef DEVICE_AC300_H
    #define DEVICE_AC300_H
    #include "Arduino.h"

    // Not implemented yet -------------------------------------
    enum enumTypes
      {
        DO_NOTHING_t = 0,
        OUT_MODE_t   = 1,
        UPS_MODE_t   = 2,
        BATT_STATE_t = 3,
        SLAVE_TYPE_t = 4,
        AUTO_SLEEP_t = 5,
        LED_MODE_t   = 6,
        SHUTDOWN_t   = 7,
        CHARG_MODE_t = 8,
        ANZ_enumTypes
      };
    enum OutputMode
      {
        OM_STOP = 0,
        OM_INVERTER_OUTPUT = 1,
        OM_BYPASS_OUTPUT_C = 2,
        OM_BYPASS_OUTPUT_D = 3,
        OM_LOAD_MATCHING   = 4,
        ANZ_OutputMode
      };
    enum UPS_Mode
      {
        UPS_CUSTOMIZED   = 1,
        UPS_PV_PRIORITY  = 2,
        UPS_STANDARD     = 3,
        UPS_TIME_CONTROl = 4,
        ANZ_UPS_Mode
      };
    enum BatteryState
      {
        BS_STANDBY      = 0,
        BS_CHARGE       = 1,
        BS_DISCHARGE    = 2,
        ANZ_BatteryState
      };
    enum MasterType
      {
        MA_SLAVE      = 0,
        MA_MASTER     = 1,
        ANZ_MasterType
      };
    // END Not implemented yet -------------------------------------
    enum AutoSleepMode
      {
        THIRTY_SECONDS = 2,
        ONE_MINNUTE    = 3,
        FIVE_MINUTES   = 4,
        NEVER          = 5,
        ANZ_AutoSleepMode
      };
    enum LedMode
      {
        LED_LOW  = 1,
        LED_HIGH = 2,
        LED_SOS  = 3,
        LED_OFF  = 4,
        ANZ_LedMode
      };
    enum EcoShutdown
      {
        ONE_HOUR    = 1,
        TWO_HOURS   = 2,
        THREE_HOURS = 3,
        FOUR_HOURS  = 4,
        ANZ_EcoShutdown
      };
    enum ChargingMode
      {
        STANDARD = 0,
        SILENT   = 1,
        TURBO    = 2,
        ANZ_ChargingMode
      };
    // { FIELD_NAME,                PAGE, OFFSET,
    //                                          SIZE,
    //                                              SCALE (dec val, def = 0),
    //                                                 EnumType (defaults to 0), FIELD_TYPE }
    static device_field_data_t bluetti_device_state[] =
      {
        /*Page 0x00 Core */                                              // poll 0
                                                                         // adr  len  bytes last offs
        {DEVICE_TYPE,               0x00, 0x0A,  6, 0, 0,             STRING_FIELD},  //  10   12   12    0x0F
        { ADR_0x0010_UINT,          0x00, 0x10,  1, 0, 0,             UINT_FIELD},    //  16    2   14    0x10
        {SERIAL_NUMBER,             0x00, 0x11,  4, 0 ,0,             SN_FIELD},      //  17    8   22    0x14
        { ADR_0x0012_UINT,          0x00, 0x15,  1, 0, 0,             UINT_FIELD},    //  21    2   24    0x15
        { ADR_0x0013_UINT,          0x00, 0x16,  1, 0, 0,             UINT_FIELD},    //  22    2   26    0x16
        {ARM_VERSION,               0x00, 0x17,  2, 0, 0,             VERSION_FIELD}, //  23    4   30    0x18
        {DSP_VERSION,               0x00, 0x19,  2, 0, 0,             VERSION_FIELD}, //  25    4   34    0x1A
        { ADR_0x001B_UINT,          0x00, 0x1B,  1, 0, 0,             UINT_FIELD},    //  27    2   36    0x1B
        { ADR_0x001C_UINT,          0x00, 0x1C,  1, 0, 0,             UINT_FIELD},    //  28    2   38    0x1C
        { ADR_0x001D_UINT,          0x00, 0x1D,  1, 0, 0,             UINT_FIELD},    //  29    2   40    0x1D
        { ADR_0x001E_UINT,          0x00, 0x1E,  1, 0, 0,             UINT_FIELD},    //  30    2   42    0x1E
        { ADR_0x001F_UINT,          0x00, 0x1F,  1, 0, 0,             UINT_FIELD},    //  31    2   44    0x1F
        { ADR_0x0020_UINT,          0x00, 0x20,  1, 0, 0,             UINT_FIELD},    //  32    2   46    0x20
        { ADR_0x0021_UINT,          0x00, 0x21,  1, 0, 0,             UINT_FIELD},    //  33    2   48    0x21
        { ADR_0x0022_UINT,          0x00, 0x22,  1, 0, 0,             UINT_FIELD},    //  34    2   50    0x22
        { ADR_0x0023_UINT,          0x00, 0x23,  1, 0, 0,             UINT_FIELD},    //  35    2   52    0x23
        {DC_INPUT_POWER,            0x00, 0x24,  1, 0, 0,             UINT_FIELD},    //  36    2   54    0x24
        {AC_INPUT_POWER,            0x00, 0x25,  1, 0, 0,             UINT_FIELD},    //  37    2   56    0x25
        {AC_OUTPUT_POWER,           0x00, 0x26,  1, 0, 0,             UINT_FIELD},    //  38    2   58    0x26
        {DC_OUTPUT_POWER,           0x00, 0x27,  1, 0, 0,             UINT_FIELD},    //  39    2   60    0x27
        { ADR_0x0028_UINT,          0x00, 0x28,  1, 0, 0,             UINT_FIELD},    //  40    2   62    0x28
        {POWER_GENERATION,          0x00, 0x29,  1, 1, 0,             DECIMAL_FIELD}, //  41    2   64    0x29
        { ADR_0x002A_UINT,          0x00, 0x2A,  1, 0, 0,             UINT_FIELD},    //  42    2   66    0x2A
        {TOTAL_BATTERY_PERCENT,     0x00, 0x2B,  1, 0, 0,             UINT_FIELD},    //  43    2   68    0x2B
        { ADR_0x002C_UINT,          0x00, 0x2C,  1, 0, 0,             UINT_FIELD},    //  44    2   70    0x2C
        { ADR_0x002D_UINT,          0x00, 0x2D,  1, 0, 0,             UINT_FIELD},    //  45    2   72    0x2D
        { ADR_0x002E_UINT,          0x00, 0x2E,  1, 0, 0,             UINT_FIELD},    //  46    2   74    0x2E
        { ADR_0x002F_UINT,          0x00, 0x2F,  1, 0, 0,             UINT_FIELD},    //  47    2   76    0x2F
        {AC_OUTPUT_ON,              0x00, 0x30,  1, 0, 0,             BOOL_FIELD},    //  48    2   78    0x30
        {DC_OUTPUT_ON,              0x00, 0x31,  1, 0, 0,             BOOL_FIELD},    //  49    2   80    0x31
                                                                          // poll 0 end
                                                                          // poll 1
        {AC_OUTPUT_MODE,            0x00, 0x46,  1, 0, OUT_MODE_t,    ENUM_FIELD},    //  70    2    2  0x46
        {INTERNAL_AC_VOLTAGE,       0x00, 0x47,  1, 1, 0,             DECIMAL_FIELD}, //  71    2    4  0x47
        //INTERNAL_POWER_ONE AC Output usage
        {INTERNAL_CURRENT_ONE,      0x00, 0x48,  1, 1, 0,             DECIMAL_FIELD}, //  72    2    6  0x48
        {INTERNAL_POWER_ONE,        0x00, 0x49,  1, 0, 0,             UINT_FIELD},    //  73    2    8  0x49      {INTERNAL_AC_FREQUENCY,     0x00, 0x4A, 1, 2, 0, DECIMAL_FIELD}, // 74    2   10  0x4E
        {INTERNAL_AC_FREQUENCY,     0x00, 0x4A,  1, 2, 0,             DECIMAL_FIELD}, //  74    2    10  0x4A
        //INTERNAL_POWER_TWO AC Internal usage?
        {INTERNAL_CURRENT_TWO,      0x00, 0x4B,  1, 1, 0,             DECIMAL_FIELD}, //  75    2   12  0x4B
        {INTERNAL_POWER_TWO,        0x00, 0x4C,  1, 0, 0,             UINT_FIELD},    //  76    2   14  0x4C
        {AC_INPUT_VOLTAGE,          0x00, 0x4D,  1, 1, 0,             DECIMAL_FIELD}, //  77    2   16  0x4D
        //INTERNAL_POWER_THREE AC Load from grid
        {INTERNAL_CURRENT_THREE,    0x00, 0x4E,  1, 1, 0,             DECIMAL_FIELD}, //  78    2   18  0x4E
        {INTERNAL_POWER_THREE,      0x00, 0x4F,  1, 0, 0,             UINT_FIELD},    //  89    2   20  0x4F
        {AC_INPUT_FREQUENCY,        0x00, 0x50,  1, 2, 0,             DECIMAL_FIELD}, //  80    2   22  0x50
        { ADR_0x0051_UINT,          0x00, 0x51,  1, 0, 0,             UINT_FIELD},    //  81    2    24  0x51
        { ADR_0x0052_UINT,          0x00, 0x52,  1, 0, 0,             UINT_FIELD},    //  82    2    26  0x52
        { ADR_0x0053_UINT,          0x00, 0x53,  1, 0, 0,             UINT_FIELD},    //  83    2    28  0x53
        { ADR_0x0054_UINT,          0x00, 0x54,  1, 0, 0,             UINT_FIELD},    //  84    2    30  0x54
        { ADR_0x0055_UINT,          0x00, 0x55,  1, 0, 0,             UINT_FIELD},    //  85    2    32  0x55
        {INTERNAL_DC_INPUT_VOLTAGE, 0x00, 0x56,  1, 1, 0,             DECIMAL_FIELD}, //  86    2   34  0x56
        {INTERNAL_DC_INPUT_POWER,   0x00, 0x57,  1, 0, 0,             UINT_FIELD},    //  87    2   36  0x57
        {INTERNAL_DC_INPUT_CURRENT, 0x00, 0x58,  1, 1, 0,             DECIMAL_FIELD}, //  88    2   38  0x58
        { ADR_0x0059_UINT,          0x00, 0x59,  1, 0, 0,             UINT_FIELD},    //  89    2    40  0x59
        { ADR_0x005A_UINT,          0x00, 0x5A,  1, 0, 0,             UINT_FIELD},    //  90    2    42  0x5A
                                                                                      // poll 1 end
                                                                                      // poll 6
        {PACK_NUM_MAX,              0x00, 0x5B,  1, 0, 0,             UINT_FIELD },   //  91    2    2  0x5B
        {TOTAL_BATTERY_VOLTAGE,     0x00, 0x5C,  1, 1 ,0,             DECIMAL_FIELD}, //  92    2    4  0x5C
        {TOTAL_BATTERY_CURRENT,     0x00, 0x5D,  1, 0, 0,             UINT_FIELD},    //  93    2    6  0x5D
        { ADR_0x005E_UINT,          0x00, 0x5E,  1, 0, 0,             UINT_FIELD},    //  94    2    8  0x5E
        { ADR_0x005F_UINT,          0x00, 0x5F,  1, 0, 0,             UINT_FIELD},    //  95    2   10  0x5F
        {PACK_NUM,                  0x00, 0x60,  1, 0, 0,             UINT_FIELD},    //  96    2   12  0x60
        {PACK_STATUS,               0x00, 0x61,  1, 0, BATT_STATE_t,  ENUM_FIELD},    //  97    2   14  0x61
        {PACK_VOLTAGE,              0x00, 0x62,  1, 2, 0,          DECIMAL_FIELD}, //  98    2   16  0x62
        {PACK_BATTERY_PERCENT,      0x00, 0x63,  1, 0, 0,             UINT_FIELD},    //  99    2   18  0x63
        { ADR_0x0064_UINT,          0x00, 0x64,  1, 0, 0,             UINT_FIELD},    // 100    2   20  0x64
        { ADR_0x0065_UINT,          0x00, 0x65,  1, 0, 0,             UINT_FIELD},    // 101    2   22  0x65
        { ADR_0x0066_UINT,          0x00, 0x66,  1, 0, 0,             UINT_FIELD},    // 102    2   24  0x66
        { ADR_0x0067_UINT,          0x00, 0x67,  1, 0, 0,             UINT_FIELD},    // 103    2   26  0x67
        { ADR_0x0068_UINT,          0x00, 0x68,  1, 0, 0,             UINT_FIELD},    // 104    2   28  0x68
        {CELL_VOLTAGES,             0x00, 0x69, 16, 2, 0,        DEC_ARRAY_FIELD},  // 105   32   60  0x88
        { ADR_0x0089_UINT,          0x00, 0x8A,  1, 0, 0,             UINT_FIELD},    // 106    2   62  0x89
        { ADR_0x008A_UINT,          0x00, 0x8B,  1, 0, 0,             UINT_FIELD},    // 107    2   64  0x8A
        { ADR_0x008B_UINT,          0x00, 0x8C,  1, 0, 0,             UINT_FIELD},    // 108    2   66  0x8B
        { ADR_0x008C_UINT,          0x00, 0x8D,  1, 0, 0,             UINT_FIELD},    // 109    2   68  0x8C
        { ADR_0x008D_UINT,          0x00, 0x8E,  1, 0, 0,             UINT_FIELD},    // 110    2   70  0x8D
        { ADR_0x008E_UINT,          0x00, 0x8F,  1, 0, 0,             UINT_FIELD},    // 111    2   72  0x8E
        { ADR_0x008F_UINT,          0x00, 0x80,  1, 0, 0,             UINT_FIELD},    // 112    2   74  0x8F
                                                                          // poll 6 end
                                                                          // poll 2
        {UPS_MODE,                  0x0B, 0xB9,  1, 0, UPS_MODE_t,    ENUM_FIELD},     // 3001    2    2  0xB9
        { ADR_0x0BBA_UINT,          0x0B, 0xBA,  1, 0, 0,             UINT_FIELD},     // 3002    2    4  0xBA
        { ADR_0x0BBB_UINT,          0x0B, 0xBB,  1, 0, 0,             UINT_FIELD},     // 3003    2    6  0xBB
        {SPLIT_PHASE_ON,            0x0B, 0xBC,  1, 0, 0,             BOOL_FIELD},     // 3004    2    8  0xBC
        {SPLIT_PHASE_MACHINE_MODE,  0x0B, 0xBD,  1, 0, DO_NOTHING_t,  ENUM_FIELD},     // 3005    2   10  0xBD
        {PACK_NUM_SET,              0x0B, 0xBE,  1, 0, 0,             UINT_FIELD},     // 3006    2   12  0xBE
        {AC_OUTPUT_CTRL,            0x0B, 0xBF,  1, 0, 0,             ENUM_FIELD},     // 3007    2   14  0xBF
        {DC_OUTPUT_CTRL,            0x0B, 0xC0,  1, 0, 0,             BOOL_FIELD},     // 3008    2   16  0xC0
        { ADR_0x0BC1_UINT,          0x0B, 0xC1,  1, 0, 0,             UINT_FIELD},     // 3003    2   18  0xC1
        { ADR_0x0BC2_UINT,          0x0B, 0xC2,  1, 0, 0,             UINT_FIELD},     // 3003    2   20  0xC2
        {GRID_CHARGE_ON,            0x0B, 0xC3,  1, 0, 0,             BOOL_FIELD},     // 3011    2   22  0xC3
        { ADR_0x0BC4_UINT,          0x0B, 0xC4,  1, 0, 0,             UINT_FIELD},     // 3003    2   24  0xC4
        {TIME_CONTROL_ON,           0x0B, 0xC5,  1, 0, 0,             BOOL_FIELD},     // 3013    2   26  0xC5
        { ADR_0x0BC6_UINT,          0x0B, 0xC6,  1, 0, 0,             UINT_FIELD},     // 3003    2   28  0xC6
        {BATTERY_RANGE_START,       0x0B, 0xC7,  1, 0, 0,             BOOL_FIELD},     // 3015    2   30  0xC7
        {BATTERY_RANGE_END,         0x0B, 0xC8,  1, 0, 0,             BOOL_FIELD},     // 3016    2   32  0xC8
                                                                         // poll 2 end
        // -----------------------
        {BLUETOOTH_CONNECTED,       0x0B, 0xDC,  1, 0, 0,             BOOL_FIELD},     // 3036    2       0xDC
        // -----------------------
        {AUTO_SLEEP_MODE,           0x0B, 0xF5,  1, 0, 0,             BOOL_FIELD}      // 3061    2       0xF5
      };
    static device_field_data_t bluetti_device_command[] =
      {
        /*Page 0x00 Core */
        {UPS_MODE,                  0x0B, 0xB9, 1, 0, UPS_MODE_t,     ENUM_FIELD},     // 3001    2    2  0xB9
        { ADR_0x0BBA_UINT,          0x0B, 0xBA, 1, 0, 0,              UINT_FIELD},     // 3002    2    4  0xBA
        { ADR_0x0BBB_UINT,          0x0B, 0xBB, 1, 0, 0,              UINT_FIELD},     // 3003    2    6  0xBB
        {SPLIT_PHASE_ON,            0x0B, 0xBC, 1, 0, 0,              BOOL_FIELD},     // 3004    2    8  0xBC
        {SPLIT_PHASE_MACHINE_MODE,  0x0B, 0xBD, 1, 0, DO_NOTHING_t,   ENUM_FIELD},     // 3005    2   10  0xBD
        {PACK_NUM,                  0x0B, 0xBE, 1, 0, 0,              BOOL_FIELD},     // 3006    2   12  0xBE
        {AC_OUTPUT_CTRL,            0x0B, 0xBF, 1, 0, OUT_MODE_t,     ENUM_FIELD},     // 3007    2   14  0xBF
        {DC_OUTPUT_CTRL,            0x0B, 0xC0, 1, 0, 0,              BOOL_FIELD},     // 3008    2   16  0xC0
        { ADR_0x0BC1_UINT,          0x0B, 0xC1, 1, 0, 0,              UINT_FIELD},     // 3003    2   18  0xC1
        { ADR_0x0BC2_UINT,          0x0B, 0xC2, 1, 0, 0,              UINT_FIELD},     // 3003    2   20  0xC2
        {GRID_CHARGE_ON,            0x0B, 0xC3, 1, 0, 0,              BOOL_FIELD},     // 3011    2   22  0xC3
        { ADR_0x0BC4_UINT,          0x0B, 0xC4, 1, 0, 0,              UINT_FIELD},     // 3003    2   24  0xC4
        {TIME_CONTROL_ON,           0x0B, 0xC5, 1, 0, 0,              BOOL_FIELD},     // 3013    2   26  0xC5
        { ADR_0x0BC6_UINT,          0x0B, 0xC6, 1, 0, 0,              UINT_FIELD},     // 3003    2   28  0xC6
        {BATTERY_RANGE_START,       0x0B, 0xC7, 1, 0, 0,              BOOL_FIELD},     // 3015    2   30  0xC7
        {BATTERY_RANGE_END,         0x0B, 0xC8, 1, 0, 0,              BOOL_FIELD},     // 3016    2   32  0xC8
        // -----------------------
        { ADR_0x0BDA_UINT,          0x0B, 0xDA, 1, 0, 0,              UINT_FIELD},     // 3034    2   28  0xC6
        { ADR_0x0BDB_UINT,          0x0B, 0xDB, 1, 0, 0,              UINT_FIELD},     // 3035    2   28  0xC6
        {BLUETOOTH_CONNECTED,       0x0B, 0xDC, 1, 0, 0,              BOOL_FIELD},     // 3036    2       0xDC
        // -----------------------
        {AUTO_SLEEP_MODE,           0x0B, 0xF5, 1, 0, AUTO_SLEEP_t,   ENUM_FIELD},      // 3061    2       0xF5
        { ADR_0x0BF6_UINT,          0x0B, 0xF6, 1, 0, 0,              UINT_FIELD},     // 3062    2   28  0xC6
        { ADR_0x0BF7_UINT,          0x0B, 0xF7, 1, 0, 0,              UINT_FIELD},     // 3062    2   28  0xC6
        { ADR_0x0BF8_UINT,          0x0B, 0xF8, 1, 0, 0,              UINT_FIELD},     // 3062    2   28  0xC6
        { ADR_0x0BF9_UINT,          0x0B, 0xF9, 1, 0, 0,              UINT_FIELD},     // 3062    2   28  0xC6
        { ADR_0x0BFA_UINT,          0x0B, 0xFA, 1, 0, 0,              UINT_FIELD},     // 3062    2   28  0xC6
        { ADR_0x0BFB_UINT,          0x0B, 0xFB, 1, 0, 0,              UINT_FIELD}     // 3062    2   28  0xC6
      };
    static device_field_data_t bluetti_polling_command[] =
      {
        {FIELD_UNDEFINED, 0x00, 0x0A, 0x28 ,0 , 0, TYPE_UNDEFINED},
        {FIELD_UNDEFINED, 0x00, 0x46, 0x15 ,0 , 0, TYPE_UNDEFINED},
        {FIELD_UNDEFINED, 0x0B, 0xB9, 0x10 ,0 , 0, TYPE_UNDEFINED},
        {FIELD_UNDEFINED, 0x0B, 0xDA, 0x03 ,0 , 0, TYPE_UNDEFINED},
        {FIELD_UNDEFINED, 0x0B, 0xF5, 0x07 ,0 , 0, TYPE_UNDEFINED},
        //Pack Polling
        {FIELD_UNDEFINED, 0x00, 0x5B, 0x25 ,0 , 0, TYPE_UNDEFINED}
      };
  #endif
#endif
/* MD0.1.0 - 2025-01-18 - md - extend functionality for AC300
 * - add and synchronize enums and fields in Device_AC300.h and DeviceType.h
 * - extend simulation in BTooth.cpp
 * - update and add evaluation of 'ENUM_FIELD' and 'DEC_ARRAY_FIELD'
 *///------------------------------------------------------------------------------------
/* MD0.0.1 - 2025-01-11 - md - initial version
 * - change code format to MD format for better readability
 *///------------------------------------------------------------------------------------

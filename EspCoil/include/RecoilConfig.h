#ifndef RECOIL_CONFIG_H
#define RECOIL_CONFIG_H
#include <stdint.h>

/** @brief structure to hold long-term settings of a weapon type
 *
 *  @details Fields of this structure are set via the BLE protocol Config
 *  characteristic only, and are used by the main logic.
 */
typedef struct
{
    uint8_t TriggerMode;       ///< 0x00 = plasma mode 0x01-0xFD = burst of up to N shots 0xFE = full auto 0xFF = Reserved; Default 0x01
    uint8_t RoF;               ///< Rate of fire for Burst and Plasma mode, in units of telemetry periods (50 ms at the moment)
    uint8_t PowerIR1;          ///< Power for IR LED 1 (long range). 0-255, default 255.
    uint8_t PowerIR2;          ///< Power for IR LED 2 (short range). 0-255, default 255.
    uint8_t PowerLED1;         ///< Power for the green power LED. 0-255, default 255.
    uint8_t PowerLED2;         ///< Power for the white muzzle LED. 0-255, default 255.
    uint8_t PowerMotor;        ///< Power for the recoil motor. Valid values 0-255. Each unit is worth 5ms of motor on (i.e. max recoil duration per shot is 1275ms)
    uint8_t FlashModeLED1 : 4; ///< Flash mode for LED1 when shooting the weapon.
    uint8_t FlashModeLED2 : 4; ///< Flash mode for LED2 when shooting the weapon.
    uint8_t FlashParam1 : 4;   ///< for square wave, number of periods to flash
    uint8_t FlashParam2 : 4;   ///< for square wave, length of a period in 100ms steps
} WeaponDefinition_t;

/** @brief structure to hold long-term settings for the local shooting logic
 *
 *  @details Fields of this structure are set via the BLE protocol Config
 *  characteristic only, and are used by the main logic.
 */
typedef struct
{
    uint8_t AutoFeedback : 4; ///< Bitmask, 0x1 = auto recoil ON, 0x02 = auto flash on shooting ON, 0x04 = auto flash on being shot ON, default 0x3
    uint8_t Unused : 4;       ///< -
    uint8_t TriggerModeOvrd;  ///< Overrides the weapon-defined trigger mode. 0x00 = plasma mode, 0x01-0xFD = burst of up to N shots, 0xFE = full auto 0xFF = Use weapon-defined trigger mode. Default 0xFF
} ShotConfig_t;

/** @brief structure to hold long-term settings for IR behaviour
 *
 *  @details Fields of this structure are set via the BLE protocol Config
 *  characteristic only, and are used by the main logic.
 */
typedef struct
{
    uint8_t TXFlags : 4;   ///< Bitmask, 0x1 = Randomise TX times. Default 0x1
    uint8_t TXRepeats : 4; ///< Number of times each “bullet” packet is repeated over IR. defaults to 2
    uint8_t RXEnable;      ///< Mask to enable or disable each individual IR sensor. Default 0xF
    uint8_t CheckInterval; ///< Period between tests of the presence the Clip-on sensor, in 500ms units.
} IRConfig_t;

/// @brief Value type for the TLV format of the Config characteristic
typedef union {
    WeaponDefinition_t weapon_def; ///< value format if the Tag is a weapon definition
    ShotConfig_t shot_conf;        ///< value format is the Tag is a shot config
    IRConfig_t ir_conf;            ///< value format is the Tag is a shot config
} config_row_value_t;

/// @brief TLV format for the Config characteristic
typedef struct
{
    uint16_t tag;             ///< tag of the configuratoin table row. Defines the format for the value field
    uint8_t len;              ///< length of the value field. Only used for consistency checking.
    config_row_value_t value; ///< actual value of the table row.
} conf_row_t;

#endif
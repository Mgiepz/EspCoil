#ifndef RECOIL_BLE_H
#define RECOIL_BLE_H
#include <stdbool.h>
#include <stdint.h>

static uint8_t COMMAND_ID_INCREMENT = 0x10;

// Struct to represent a RecoilGun IR event inside a RecoilGun Telemetry packet
typedef struct IrEvent_s
{
  uint16_t payload;    // Payload
  uint8_t counter : 4; // Counter
  uint8_t sensor : 4;  // Counter
} __attribute__((packed)) IrEvent;

typedef struct BleChrTelemetry_s
{
  uint8_t PktCounter : 4;
  uint8_t CmdCounter : 4;
  uint8_t GunID;      ///< The identifier for this gun. 0x01…0x10 is valid, 0 is invalid. Onyly used for TX via BLE, the authoritative source is gGunSettings
  uint8_t Buttons;    ///< Digital buttons from controller; one bit per button.
  uint8_t Pressed[3]; ///< Number of times that various buttons have been pressed (mod 16):
                      // Trigger, reload, walkie talkie, reset, power, recoil cnt
  int16_t Voltage;    ///< Battery voltage
  IrEvent ir[2];      ///< up to 2 events from the IR receivers
  uint8_t WeaponAmmo; ///< WeaponAmmo the gun currently think it has
  uint8_t GunFlags;   ///< 0x01 = Reload mode is on, 0x02 clip-on sensor disconnected
  uint8_t WeaponType; ///< report back Weapon type
} __attribute__((packed)) BleChrTelemetry;

// Enum to represent buttons inside a RecoilGun Telemetry packet
typedef enum
{
  BTN_TRIGGER = 0x01,       // trigger
  BTN_RELOAD = 0x02,        // reload
  BTN_WALKIE_TALKIE = 0x04, // walkie talkie
  BTN_RESET = 0x08,         // reset
  BTN_POWER = 0x10,         // power
  BTN_RECOIL = 0x20,        // recoil cnt
} BTN_PRESS;

// Struct to represent a RecoilGun command
typedef struct BleChrCmd_s
{
  uint8_t CmdId;          ///< The counter of this packet.
                          ///< Increments each time we want to trigger a command.
  uint8_t IR_ack;         ///< Sequence of infrared events that have been received.
  uint16_t Command;       ///< Control commands to exert (bit mask)
  uint8_t GunID;          ///< Which users gun are we? (1..16 is valid)
  uint8_t WeaponType;     ///< Weapon id to use for firing
  uint8_t WeaponAmmo;     ///< Amount of ammo remaining for firing independently (not used).
  uint8_t pad[13];        ///< Padding
} __attribute__((packed)) BleChrCmd;

// Enum to represent an action inside a RecoilGun command
typedef enum
{
  ACT_NOTHING = 0x0000,   // Do nothing.
  ACT_SHOOT = 0x0001,     // Shoot gun using LED 1
  ACT_RELOADON = 0x0002,  // Set reolad mode (no shooting!)
  ACT_RELOADOFF = 0x0004, // Unset reload mode (take new ammo count value)
  ACT_RECOIL = 0x0008,    // Trigger recoil
  ACT_MUZZLE = 0x0010,    // Muzzle flash
  ACT_POWER_OFF = 0x0020, // Turn power off in one second
  ACT_STATS = 0x0040,     // Output stats on UART
  ACT_SYNC = 0x0080,      // Sync (forget outstanding commands and ir events)
  ACT_REBOOT_DFU = 0x0100 // Reboot to DFU
} BleCmdAction;

// Enum to represent the connected weapon type
typedef enum
{
  CW_NONE,
  CW_Rifle,
  CW_Pistol
} ConnectedWeapon;

#endif
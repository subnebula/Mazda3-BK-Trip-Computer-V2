/* Nathaniel Roach */

#ifndef _TYPES_H_
#define _TYPES_H_

#include <SPI.h>
#include <mcp_can.h>

enum KeyBarrel {off, acc, on, start};

typedef struct {
  uint16_t ID;
  uint8_t length;
  uint8_t data[8];
} BusMessage;

typedef struct {
  uint16_t bodySpeed; // 100 = 1km/h; can be negative but not known if signed
  uint16_t engineRPM;
  int8_t engineCoolTemp; // Coolant temp, 50 = 0degc
  uint8_t fuelUseCounter; // Increments and resets
  uint64_t fuelUsed;
  uint8_t guess3;  // unk, 
  uint8_t throttlePosition; // Actual position of butterfly valve - changes with cruise on
  uint16_t tripSpeedAvg;
  uint16_t tripUsageCur; // 22 = 2.2l/100km
  uint16_t tripUsageAvg;
  uint16_t tripDistRemain;
  uint8_t doorState;
  boolean gearReverse;
  boolean handbrake;
  KeyBarrel keyState;
} VehicleData;

typedef struct {
  boolean analysisEnabled;
  VehicleData *carState;
  MCP_CAN *canBus;
} DeviceState;

#endif

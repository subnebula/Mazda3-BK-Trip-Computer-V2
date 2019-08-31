/* Nathaniel Roach */

#ifndef _MAZDA3BK_H_
#define _MAZDA3BK_H_

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios
#include "types.h"

#define _INCLUDE_CANFUNC
#include "canscan.h"
#include "canfunctions.h"

#define RHEOSTAT_INPUT A1
#define RHEOSTAT_STEPS 6
#define RHEOSTAT_RES_MIN 520 // Top of rheostat
#define RHEOSTAT_RES_MAX 25 // Bottom of rheostat
#define IN_BUTTON 7
#define OUT_WASHER 4

#define ENGINE_RPM_REDLINE 6500
#define ENGINE_RPM_SHIFT 3500

#define SHIFT_LIGHT_DELAY 10000 // in ms

// y = mx + c
#define SHIFT_LIGHT_M 100
#define SHIFT_LIGHT_C 1000

void getData(DeviceState *settings);
uint32_t fuelVolumeInc(uint8_t counter, uint32_t total);
uint8_t getDesiredPage(uint8_t analogVal);
void formatScreen(DeviceState *settings);
void mazda3BKLCDPrint(DeviceState *settings, char inStr[], uint8_t extras, uint8_t formatting);
char guessGear(VehicleData carState);

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif
#endif

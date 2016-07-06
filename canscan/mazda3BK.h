/* Nathaniel Roach */

#ifndef _MAZDA3BK_H_
#define _MAZDA3BK_H_

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios
#include "types.h"

#define _INCLUDE_CANFUNC
#include "canscan.h"
#include "canfunctions.h"

#define RHEOSTAT_INPUT A0
#define RHEOSTAT_STEPS 6
#define RHEOSTAT_RES_MIN 520 // Top of rheostat
#define RHEOSTAT_RES_MAX 25 // Bottom of rheostat

void getData(DeviceState *settings);
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

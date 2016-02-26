/* Nathaniel Roach */

#ifndef _MAZDA3BK_H_
#define _MAZDA3BK_H_

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios
#include "types.h"

#define _INCLUDE_CANFUNC

#include "canscan.h"
#include "canfunctions.h"

void mazdaBKLCDPrint(MCP_CAN subjCAN, char inStr[], uint8_t formatting,
    boolean *analyseActive);
char guessGear(VehicleData carState);

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif
#endif

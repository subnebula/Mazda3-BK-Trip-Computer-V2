/* Nathaniel Roach */

#ifndef _CANSCAN_H_
#define _CANSCAN_H_

//##############################################


const uint8_t displayPage = 5;
const int SPI_CAN1CS_PIN = 9;
const int SPI_CAN2CS_PIN = 8;
#define _INCLUDE_CANFUNC


//##############################################


#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios

#include <MsTimer2.h>

#ifdef _INCLUDE_CANFUNC
extern "C"{
  #include <naz-linkList.h>
}
#include "canfunctions.h"
LinkedList* msgIndex;

#endif
#include "mazda3BK.h"

extern "C"{
  typedef struct {
    uint16_t ID;
    uint8_t length;
    uint8_t data[8];
  } BusMessage;
  
  typedef struct {
    uint16_t bodySpeed; // 100 = 1km/h; can be negative but not known if signed
    uint16_t engineRPM;
    uint8_t throttlePedal;
    uint16_t tripSpeedAvg;
    uint16_t tripUsageCur; // 22 = 2.2l/100km
    uint16_t tripUsageAvg;
    uint16_t tripDistRemain;
    uint8_t doorState;
    boolean gearReverse;
    boolean handbrake;
  } VehicleData;
}


#endif

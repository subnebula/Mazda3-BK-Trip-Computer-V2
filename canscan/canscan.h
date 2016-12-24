/* Nathaniel Roach */

#ifndef _CANSCAN_H_
#define _CANSCAN_H_

#include <stdint.h>

//##############################################

//const uint8_t displayPage = 1;
#define SPI_CAN1CS_PIN 9
#define MCP2515INT 2
#define _WITHANALYSE
#define _MAZDA3BK

//##############################################


#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios
#include <MsTimer2.h>

#ifdef _WITHANALYSE
#define _INCLUDE_CANFUNC

#endif

#include "naz-binaryTree.h"

#ifdef _MAZDA3BK
#include "mazda3BK.h"
#endif

#endif


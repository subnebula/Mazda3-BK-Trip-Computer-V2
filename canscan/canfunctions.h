/* Nathaniel Roach */

#ifndef _CANFUNCTIONS_H_
#define _CANFUNCTIONS_H_

#include "naz-binaryTree.h"

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios
#include "types.h"

BusMessage getMessage(MCP_CAN subjCAN);
void CAN2UART(char *prefix, uint16_t msgID, uint8_t msgLen, uint8_t *msg);
void trigger(MCP_CAN subjCAN, uint16_t triggerOnID, uint16_t triggerOnIndex,
    uint16_t triggerOnContents);
void analyse(MCP_CAN subjCAN, BinaryTree *subjTree);
void analyseMessage(BusMessage inMsg, BinaryTree *subjTree);

#endif

/* Nathaniel Roach */

#ifndef _CANFUNCTIONS_H_
#define _CANFUNCTIONS_H_

extern "C"{
  #include <naz-linkList.h>
}

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios

void CAN2UART(char *prefix, uint16_t msgID, uint8_t msgLen, uint8_t *msg);
void trigger(MCP_CAN subjCAN, uint16_t triggerOnID, uint16_t triggerOnIndex, 
    uint16_t triggerOnContents);
LinkedListNode *linkedListFind(LinkedList *subjList, uint16_t msgID);
void analyse(MCP_CAN subjCAN, LinkedList *subjList);

#endif

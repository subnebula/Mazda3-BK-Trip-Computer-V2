/* Nathaniel Roach */

#ifndef _MAZDA3BK_H_
#define _MAZDA3BK_H_

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios

#include "canscan.h"

uint8_t BUSMsg28FClockButton[8] = {128, 0, 0, 0, 48, 0, 0, 0};
uint8_t BUSMsg28FSetButton[8] =   {128, 0, 0, 0, 40, 0, 0, 0};
uint8_t BUSMsg28FIdle[8] =        {128, 0, 0, 0, 32, 0, 0, 0};
uint8_t BUSMsg28FDISOn[8] =       {128, 0, 0, 0,160, 0, 0, 0}; /*
uint8_t BUSMsg401ACK[1] = {12};
uint8_t LCDText1[8] = {192, ' ', ' ', ' ', 'M', 'a', 'd', 'M'};
uint8_t LCDText2[8] = {135, 'a', 'z', ' ', ' ', ' ', 32, 32};
uint8_t LCDText1Alt[8] = {192, 'H', 'i', ' ', 'K', 'a', 'l', 'd'};
uint8_t LCDText2Alt[8] = {135, 'e', 'n', '!', ' ', ' ', ' ', ' '};
uint8_t LCDClear1[8] = {192, 32, 32, 32, 32, 32, 32, 32};
uint8_t LCDClear2[8] = {135, 32, 32, 32, 32, 32, 32, 32};
uint8_t LCDText1DISOn[8] = {192, 32, 68, 73, 83, 32, 79, 78}; */

void mazdaBKLCDPrint(MCP_CAN subjCAN, char inStr[], uint8_t formatting);
char guessGear(uint16_t inRPM, int8_t inSpeed);

#ifdef __cplusplus
extern "C"{
#endif

#ifdef __cplusplus
}
#endif
#endif

// (C) Nathaniel Roach 2015
// GNU GPLv2
// analyse() will store CAN messages in a linked list, and print them out when
//    stored, and when they change. Disable the timer, add to loop() to use it.
// Currently will print out statistics to the MAZDA 3 2007 (BK) Information display

#include "canscan.h"

MCP_CAN CAN1(SPI_CAN1CS_PIN);

#ifdef _USE_CAN2
MCP_CAN CAN2(SPI_CAN2CS_PIN);
#endif
#ifdef _INCLUDE_CANFUNC
BinaryTree *msgIndex;
#endif
VehicleData carState;
DeviceState settings;

uint8_t loopGetData = 0;
uint8_t loopWriteDisplay = 0;

void setup(){
  Serial.begin(115200);
  #ifdef _INCLUDE_CANFUNC
  msgIndex = binaryTreeCreate();
  #endif

  CAN1START_INIT:
  if(CAN_OK == CAN1.begin(CAN_125KBPS)){
    Serial.println("CAN1 INIT OK");
  } else {
    Serial.println("CAN1 INIT FAIL");
    delay(100);
    goto CAN1START_INIT;
  }

  #ifdef _USE_CAN2
  CAN2START_INIT:
  if(CAN_OK == CAN2.begin(CAN_125KBPS)){
    Serial.println("CAN2 INIT OK");
  } else {
    Serial.println("CAN2 INIT FAIL");
    delay(100);
    goto CAN2START_INIT;
  }
  #endif

  settings.analysisEnabled = false;
  settings.carState = &carState;
  settings.canBus = &CAN1;

  //attachInterrupt(digitalPinToInterrupt(MCP2515INT), handleMCP2515Int, RISING); 

  MsTimer2::set(5, handleTimer); // 5ms period
  MsTimer2::start();
}

void handleMCP2515Int(){
  loopGetData = 1;
}

void handleTimer(){
  static uint16_t macroCycles = 0;
  macroCycles++;

  if (macroCycles >= 20){ // Every 100ms
    loopWriteDisplay = 1;
    macroCycles = 0;
  } else {
    loopGetData = 1;
  }
}

void loop(){
  if (loopGetData){
    MsTimer2::stop();
    getData(&settings);
    loopGetData = 0;
    MsTimer2::start();
  }
  if (loopWriteDisplay){
    MsTimer2::stop();
    formatScreen(&settings);
    loopWriteDisplay = 0;
    MsTimer2::start();
  }
}

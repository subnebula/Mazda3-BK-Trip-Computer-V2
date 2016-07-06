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
LinkedList *msgIndex;
#endif
VehicleData carState;
DeviceState settings;

void setup(){
  Serial.begin(115200);
  #ifdef _INCLUDE_CANFUNC
  msgIndex = linkedListCreate();
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
  //settings.screenPage = displayPage; //FIX
  settings.carState = &carState;
  settings.canBus = &CAN1;

  MsTimer2::set(10, handleTimer); // 1ms period
  MsTimer2::start();
}

void handleTimer(){
  MsTimer2::stop();
  static uint16_t macroCycles = 0;
  macroCycles++;

  if (macroCycles >= 10){ // Every 10ms
    formatScreen(&settings);
    macroCycles = 0;
  } else {
    getData(&settings);
  }
  MsTimer2::start();
}

void loop(){} // loop() dies after a while when using this timer library, so don't bother

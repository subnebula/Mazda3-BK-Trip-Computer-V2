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
uint8_t loopLogWrite = 0;

void setup(){
  Serial.begin(115200);
  Serial.print("canscan compiled for Mazda 3 BK2, at ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.print("\r\n");
  Serial.print("(C) 2015 - 2017 Nathaniel Roach\r\n");
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
  settings.loggingEnabled = false;
  settings.carState = &carState;
  settings.canBus = &CAN1;

  pinMode(MCP2515INT, INPUT);
  attachInterrupt(digitalPinToInterrupt(MCP2515INT), handleMCP2515Int, RISING);
  pinMode(IN_BUTTON, INPUT_PULLUP);
  pinMode(OUT_WASHER,OUTPUT);

  set_sleep_mode(SLEEP_MODE_IDLE);

  MsTimer2::set(TIMER_PERIOD, handleTimer); // 5ms period
  MsTimer2::start();
}

void handleMCP2515Int(){
  loopGetData = 1;
//  digitalWrite(OUT_WASHER, 1);
}

void handleTimer(){
  static uint16_t macroCycles = 0;
  macroCycles++;

  if (!(macroCycles % SAMPLE_PERIOD)) // if SAMPLE_PERIOD cleanly goes into macroCycles
    loopGetData = 1;

  if (!(macroCycles % REDRAW_PERIOD))
    loopWriteDisplay = 1;

  if (!(macroCycles % LOGGING_PERIOD) && settings.loggingEnabled)
    loopLogWrite = 1;
    
  if (macroCycles >= 60000)
    macroCycles = 0;
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
  if (loopLogWrite){
    //MsTimer2::stop();
    stateToSerial(&settings);
    loopLogWrite = 0;
    //MsTimer2::start();
  }
  sleep_enable();
  sleep_mode();
  sleep_disable();
}

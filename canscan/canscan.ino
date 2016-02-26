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
boolean analyseActive = false;

VehicleData carState;

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

  MsTimer2::set(1, handleTimer); // 1ms period
  MsTimer2::start();
}

void formatScreen(){
  char output[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t formatting = 0;

  /*
  |            |
  |Axx Rxxxx G?| Throttle RPM GearGuess
  | Rxxx C---  | Remaining CurCons
  | C--- S---  | AvgCons AvgSpeed
  |R.Right Open|
  |   Boot Open|
  |   MadMaz   |
  */

  if (carState.doorState != 0){ // Prioritise door notification

    if (carState.doorState & 1<<3) {
      sprintf(output, "   Boot Open");
    } else if (carState.doorState & 1<<4){
      sprintf(output, "R.Right Open");
    } else if (carState.doorState & 1<<5){
      sprintf(output, " R.Left Open");
    } else if (carState.doorState & 1<<6){
      sprintf(output, " F.Left Open");
    } else if (carState.doorState & 1<<7){
      sprintf(output, "F.Right Open");
    }
  } else {
    switch(displayPage){
      case 1 : // Powertrain stats
        if (carState.engineRPM == 0){
          sprintf(output, "   MadMaz   ");
        } else {
          if (carState.throttlePosition == 200){ // Wide Open Throttle
            sprintf(output, "WOT R%04i G%c", carState.engineRPM, guessGear(
                  carState));
          } else {
            sprintf(output, "A%02i R%04i G%c", carState.throttlePosition/2,
                  carState.engineRPM, guessGear(carState));
          }
        }
        break;

      case 2 : // Trip stats page 1
        if (carState.tripUsageCur > 1500){
          sprintf(output, " R%03i  C---", carState.tripDistRemain);
        } else {
          formatting = 2; // Place a dot for the Current consumption
          sprintf(output, " R%03i C%03i", carState.tripDistRemain, carState.tripUsageCur);
        }
        break;

      case 3 : // Trip stats page 2
        formatting = 2; // Place a dot for the Current consumption
        sprintf(output, "AS%03i C%03i", carState.tripSpeedAvg, carState.tripUsageAvg);
        break;

      case 4 : // Misc Stats
        formatting = 2;
        sprintf(output, " Speed %05i", carState.bodySpeed);
        break;

      case 5 : // RPM:Speed ratio (debugging gearguessing)
        sprintf(output, "R%03i R%04i %c", carState.engineRPM/
        (carState.bodySpeed/100), carState.engineRPM, guessGear(carState));
        break;
      }
  }
  mazdaBKLCDPrint(CAN1, output, formatting, &analyseActive);
}

void getData(MCP_CAN subjCAN){
  BusMessage subjMsg = getMessage(subjCAN);
  if(subjMsg.ID != 0){ // Check if message available
    switch(subjMsg.ID) {
      case 0x201 :
        carState.engineRPM = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
        if (carState.engineRPM > 9000){ //Hacky overflow check, may not be needed
          carState.engineRPM = 0;
        }
        carState.bodySpeed = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
        if (carState.bodySpeed == 32768)
          carState.bodySpeed = 0;
        carState.throttlePosition = subjMsg.data[6];
        break;

      case 0x400 :
        carState.tripSpeedAvg = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
        carState.tripUsageCur = (uint16_t)(subjMsg.data[2]*256 + subjMsg.data[3]);
        carState.tripUsageAvg = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
        carState.tripDistRemain = (uint16_t)(subjMsg.data[6]*256 + subjMsg.data[7]);
        break;

      case 0x433 :
        carState.doorState = subjMsg.data[0];
        carState.handbrake = (subjMsg.data[3] & 1<<0); //0th bit
        carState.gearReverse = (subjMsg.data[3] & 1<<1); //1st bit

      //default:
        //Not a message we know/want
    }
  }
  #ifdef _WITHANALYSE
  if (analyseActive)
    analyseMessage(subjMsg, msgIndex);
  #endif
}

void handleTimer(){
  MsTimer2::stop();
  static uint16_t macroCycles = 0;
  macroCycles++;

  if (macroCycles >= 100){ // Every 100ms
    formatScreen();
    macroCycles = 0;
  } else {
    getData(CAN1);
  }
  MsTimer2::start();
}

void loop(){} // loop() dies after a while when using this timer library, so don't bother

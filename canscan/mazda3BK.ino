/* Nathaniel Roach*/
#include "mazda3BK.h"

#ifdef _MAZDA3BK

void getData(DeviceState *settings){
  if (settings == nullptr){
    Serial.println("ERR: getData() passed bad settings");
    abort(); //return();
  }
  boolean analyseActive = (*settings).analysisEnabled;
  VehicleData *carState = (*settings).carState;
  MCP_CAN subjCAN = *(*settings).canBus;
  
  BusMessage subjMsg = getMessage(subjCAN);
  if(subjMsg.ID != 0){ // Check if message available
    switch(subjMsg.ID) {
      case 0x201 :
        (*carState).engineRPM = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
        if ((*carState).engineRPM > 9000){ //Hacky overflow check, may not be needed
          (*carState).engineRPM = 0;
        }
        (*carState).bodySpeed = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
        if ((*carState).bodySpeed == 32768)
          (*carState).bodySpeed = 0;
        (*carState).throttlePosition = subjMsg.data[6];
        break;

      case 0x400 :
        (*carState).tripSpeedAvg = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
        (*carState).tripUsageCur = (uint16_t)(subjMsg.data[2]*256 + subjMsg.data[3]);
        (*carState).tripUsageAvg = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
        (*carState).tripDistRemain = (uint16_t)(subjMsg.data[6]*256 + subjMsg.data[7]);
        break;
        
      case 0x420 :
        (*carState).engineCoolTemp = abs(subjMsg.data[0] - 40); // engine temp? (MADOX)
        (*carState).engineOilTemp = subjMsg.data[2]; // engine temp? (mine)
        break;

      case 0x433 :
        (*carState).doorState = subjMsg.data[0];
        (*carState).guess3 = subjMsg.data[2];  // engine temp? 1 = .25 degc (MADOX)
        (*carState).handbrake = (subjMsg.data[3] & 1<<0); //0th bit
        (*carState).gearReverse = (subjMsg.data[3] & 1<<1); //1st bit
        break;

      default:
        break;
        //Not a message we know/want
    }
  }
  #ifdef _WITHANALYSE
  //if (false)
  if (analyseActive)
    analyseMessage(subjMsg, msgIndex);
  #endif
}

int getDesiredPage(int analogVal){
  int stepSize, retval;

  stepSize = abs(RHEOSTAT_RES_MAX - RHEOSTAT_RES_MIN)/RHEOSTAT_STEPS;
  retval = analogVal / stepSize;
  return retval;
}

void formatScreen(DeviceState *settings){
  int displayPage;
  if (settings == nullptr){
    Serial.println("ERR: formatScreen() passed bad settings");
    abort(); //return();
  }
  VehicleData carState = *(*settings).carState;
  char output[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t extras = 0;
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
  //if (false){
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
    displayPage = getDesiredPage(analogRead(RHEOSTAT_INPUT));
    switch(displayPage){
    
      case 0 : 
        sprintf(output, "   MadMaz   ");
        break;

      case 1 : // Powertrain stats
        if (carState.engineRPM == 0){
          sprintf(output, "   MadMaz   ");
        } else {
          if (carState.throttlePosition == 200){ // Wide Open Throttle
            sprintf(output, "WOT R%04i %2i", carState.engineRPM, carState.engineCoolTemp);
          } else {
            sprintf(output, "A%02i R%04i %2i", carState.throttlePosition/2,
                  carState.engineRPM, carState.engineCoolTemp);
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
        sprintf(output, " AS%03i C%03i", carState.tripSpeedAvg, carState.tripUsageAvg);
        break;

      case 4 : // Misc Stats
        formatting = 2;
        sprintf(output, " Speed %05i", carState.bodySpeed);
        break;

      case 5 : // RPM:Speed ratio (debugging gearguessing)
        sprintf(output, "R%03i R%04i %c", carState.engineRPM/
          (carState.bodySpeed/100), carState.engineRPM, guessGear(carState));
        break;

      case 6 : // Temperature debugging
        sprintf(output, "%03i %03i %03i", carState.engineCoolTemp, carState.engineOilTemp,
          carState.guess3);
        break;

      default :
        if (carState.engineRPM == 0){
          sprintf(output, "   MadMaz   ");
        } else {
          if (carState.throttlePosition == 200){ // Wide Open Throttle
            sprintf(output, "WOT R%04i %03i", carState.engineRPM, carState.engineCoolTemp);
          } else {
            sprintf(output, "A%02i R%04i %03i", carState.throttlePosition/2,
                  carState.engineRPM, carState.engineCoolTemp);
          }
        }
        extras = 0x80;
        break;
      }
  }
  mazda3BKLCDPrint(settings, output, extras, formatting);
}

void mazda3BKLCDPrint(DeviceState *settings, char inStr[], uint8_t extras,
                                                      uint8_t formatting){
  //uint8_t LCDText1[8] = {192, L1, L2, L3, L4, L5, L6, L7};
  //uint8_t LCDText2[8] = {135, L8, L9, L10, L11, L12, 32, 32};
  //uint8_t outputText[13] = {L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, 0}
  uint8_t BUSMsg28F[8] = {128, 0, 0, 0, 32, 0, 0, 0};
  //                                    ^ Set this to two to see all valid
  //                                      formatting locations

  if (settings == nullptr){
    Serial.println("ERR: getData() passed bad settings");
    abort(); //return();
  }
  boolean *analyseActive = &(*settings).analysisEnabled;
  MCP_CAN subjCAN = *(*settings).canBus;

  char outputText[13]; // 12 chars + NUL
  char inChar;
  uint8_t LCDMsg1[8];
  uint8_t LCDMsg2[8];

  BUSMsg28F[0] = extras; // Sets apostrophes and colons
  BUSMsg28F[3] = formatting; // Sets apostrophes and colons

  if (Serial.available() > 0) {
    inChar = Serial.read();
    if (inChar == 'S'){
      Serial.println("[msg] SET");
      BUSMsg28F[4] = 40;
    } else if (inChar == 'C'){
      Serial.println("[msg] CLOCK");
      BUSMsg28F[4] = 48;
    } else if (inChar == 'D'){
      Serial.println("[msg] DISON");
      BUSMsg28F[4] = 160;
    #ifdef _WITHANALYSE
    } else if (inChar == 'A'){
      Serial.print("[option] analysis ");
      *analyseActive = !(*analyseActive);
      Serial.println(*analyseActive, HEX);
    #endif
    }
  }

  while (Serial.available() > 0) {
    inChar = Serial.read(); // clear buffered up messages
  }

  for (int i=0; i < 13; i++){
    outputText[i] = inStr[i];
  }
  
  LCDMsg1[0] = 192; // Weird magic value, some others may be for scrolling
  for (int i = 0; i < 7; i++){
    LCDMsg1[i+1] = outputText[i];
  }

  LCDMsg2[0] = 135; // Ditto the above
  for (int i = 0; i < 6; i++){
    LCDMsg2[i+1] = outputText[i+7];
  }

  subjCAN.sendMsgBuf(0x28F, 0, 8, BUSMsg28F);
  subjCAN.sendMsgBuf(0x290, 0, 8, LCDMsg1);
  subjCAN.sendMsgBuf(0x291, 0, 8, LCDMsg2);
}

char guessGear(VehicleData carState){
  char retval;
  int ratio;

  if (carState.gearReverse){
    retval = 'R';
  } else if (carState.bodySpeed == 0){
    retval = 'N';
  } else {
    ratio = carState.engineRPM/(carState.bodySpeed/100);
    if (ratio > 130 && ratio < 170){ // Quantisation effects the calculation at lower speeds
      retval = '1';
    } else if (ratio > 70 && ratio < 83){
      retval = '2';
    } else if (ratio > 50 && ratio < 55){ // 52/53
      retval = '3';
    } else if (ratio > 34 && ratio < 38){ // 36
      retval = '4';
    } else if (ratio > 28 && ratio < 32){
      retval = '5';
    } else{
      retval = '?';
    }
  }

  return(retval);
}
#endif


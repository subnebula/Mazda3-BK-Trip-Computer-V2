/* Nathaniel Roach*/
#include "mazda3BK.h"

#ifdef _MAZDA3BK

boolean accHold;
int displayPage;


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
        if ((*carState).keyState == on){ // Values are nonsensical otherwise
          (*carState).engineRPM = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
          (*carState).bodySpeed = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
          (*carState).throttlePosition = subjMsg.data[6];
        }
        break;

      case 0x265 :
        (*carState).indicatorLeft = (subjMsg.data[0] & 1<<5);
        (*carState).indicatorRight = (subjMsg.data[0] & 1<<6);
        break;

      case 0x285 :
        if (subjMsg.data[0] & 1<<5){
          (*carState).keyState = on;
          (*carState).hasStarted = true;
        } else {
          (*carState).keyState = acc; // board is getting power but !ON
        }
        break;

      case 0x400 :
        (*carState).tripSpeedAvg = (uint16_t)(subjMsg.data[0]*256 + subjMsg.data[1]);
        (*carState).tripUsageCur = (uint16_t)(subjMsg.data[2]*256 + subjMsg.data[3]);
        (*carState).tripUsageAvg = (uint16_t)(subjMsg.data[4]*256 + subjMsg.data[5]);
        (*carState).tripDistRemain = (uint16_t)(subjMsg.data[6]*256 + subjMsg.data[7]);
        break;
     
      case 0x420 :
        (*carState).engineCoolTemp = abs(subjMsg.data[0] - 40); // engine temp? (MADOX)
        if ((*carState).keyState == on){
          (*carState).fuelUseCounter = subjMsg.data[2]; // increments per 0.0001 litres consumed
          (*carState).fuelUsed = fuelVolumeInc(subjMsg.data[2], (*carState).fuelUsed);
        }
        break;

      case 0x433 :
        (*carState).doorState = subjMsg.data[0];
        (*carState).guess3 = subjMsg.data[2];  // engine temp? 1 = .25 degc (MADOX)
        (*carState).handbrake = (subjMsg.data[3] & 1<<0); //0th bit
        if ((*carState).keyState == on){
          (*carState).gearReverse = (subjMsg.data[3] & 1<<1); //1st bit
        }
        break;

      default :
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

uint32_t fuelVolumeInc(uint8_t counter, uint32_t total){
  static uint8_t last = 0;
  static uint8_t first = 1;

  if (counter == last)
    return total;

  if (first){
    last = counter;
    first = 0;
  }

  if (counter < last){
    total = total + (uint32_t)((counter + 256) - last);
  } else {
    total = (uint32_t)(total + (counter - last));
  }
  last = counter;
  return total;
}

void formatScreen(DeviceState *settings){

  if (settings == nullptr){
    Serial.println("ERR: formatScreen() passed bad settings");
    abort(); //return();
  }
  VehicleData carState = *(*settings).carState;
  char output[14] = {0,0,0,0,0,0,0,0,0,0,0,0,0}; // +1 for the NUL at the end
  uint8_t extras1 = 0;
  uint8_t extras2 = 0;
  uint8_t formatting = 0;
  char doorBoot = ' ', doorReaRi = ' ', doorReaLef = ' ', doorFronLef = ' ', doorFronRi = ' ';


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
    if (carState.doorState & 0x08) { // Boot
      doorBoot = 31; // Down arrow in ASCII
    }
    if (carState.doorState & 0x10){ // Rear Right
      doorReaRi = 0xF0;
    }
    if (carState.doorState & 0x20){ // Rear Left
      doorReaLef = 0xF1;
    }
    if (carState.doorState & 0x40){ // Front Left
      doorFronLef = 0xF1;
    }
    if (carState.doorState & 0x80){ // Front Right
      doorFronRi = 0xF0;
    }
    sprintf(output, "%c%c%c Open %c%c%c", doorFronLef, doorReaLef, doorBoot, doorBoot, 
      doorReaRi, doorFronRi);
      //"<<_ Open _>>"
  } else {
    if (displayPage == 7)
      displayPage = 0;
    switch (displayPage){

      default :
        displayPage = 0;
        extras1 += 1; // Turn on AF bit

      case 0 :
      if (carState.keyState != on){
          sprintf(output, "   Mazda3   ");
        } else if (carState.keyState == on){
          sprintf(output, "R%3i %3iKm/h", carState.tripDistRemain, carState.bodySpeed/100);
        }
        break;

      case 1 : // RPM:Speed ratio (debugging gearguessing)
        sprintf(output, "%c    %4iRPM", guessGear(carState), carState.engineRPM);
        break;

      case 2 : // Trip stats page 1
        if (carState.tripUsageCur > 1500){
          sprintf(output, "R%-3i   C---", carState.tripDistRemain);
        } else {
          formatting = 2; // Place a dot for the Current consumption
          sprintf(output, "R%-3i   C%3i", carState.tripDistRemain, carState.tripUsageCur);
        }
        break;

      case 3 : // Trip stats page 2
        formatting = 2; // Place a dot for the Current consumption
        sprintf(output, "AS%-3i  C%3i", carState.tripSpeedAvg, carState.tripUsageAvg);
        break;

      case 4 : // Misc Stats
        sprintf(output, "Speed %3iKph", carState.bodySpeed/100);
        break;

      case 5 : // Powertrain stats
        if (carState.keyState != on){
          sprintf(output, "   Mazda3   ");
        } else if (carState.keyState == on){
          if (carState.throttlePosition == 200){ // Wide Open Throttle
            sprintf(output, "WOT %04i %3i", carState.engineRPM, carState.engineCoolTemp);
          } else {
            sprintf(output, "%03i %04i %3i", carState.throttlePosition,
                  carState.engineRPM, carState.engineCoolTemp);
          }
        }
        break;

      case 6 : // Fuel usage
        if ((carState.hasStarted) && (carState.keyState != on))
        {
          sprintf(output, "    %06lumL", carState.fuelUsed/5); // mL
        } else {
          sprintf(output, "    %07lu", carState.fuelUsed); // Raw units displayed
        }
        break;

      }
  }
  mazda3BKLCDPrint(settings, output, extras1, extras2, formatting);
}

uint8_t decideShiftLight(DeviceState *settings){
  uint8_t retval = 0;
  static uint8_t counting = 0;
  static unsigned long tstart;
  VehicleData carState = *(*settings).carState;

  if (carState.engineRPM >= ENGINE_RPM_REDLINE){
    retval = 1;
  } else if (carState.engineRPM >= 
    (unsigned)((SHIFT_LIGHT_M*carState.engineCoolTemp)+SHIFT_LIGHT_C)){
    retval = 1; // "Receding redline" similar to those used in BMWs re. oil
  } else if (carState.engineRPM >= ENGINE_RPM_SHIFT){
    if (!counting ){
      if (carState.engineRPM >= ENGINE_RPM_SHIFT + 100){ // Add some hysteresis
        counting = 1;
        tstart = millis();
      }
    } else {
      retval = (millis() >= (tstart + SHIFT_LIGHT_DELAY));
    }
  } else
    counting = 0;
  return retval;
}

void mazda3BKLCDPrint(DeviceState *settings, char inStr[], uint8_t extras1,
                                      uint8_t extras2, uint8_t formatting){
  //uint8_t LCDText1[8] = {192, L1, L2, L3, L4, L5, L6, L7};
  //uint8_t LCDText2[8] = {135, L8, L9, L10, L11, L12, 32, 32};
  //uint8_t outputText[13] = {L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, 0}
  uint8_t BUSMsg28F[8] = {128, 0, 0, 0, 32, 0, 0, 0};
  //                                       ^ Set this to two to see all valid
  //                                      formatting locations

  if (settings == nullptr){
    Serial.println("ERR: getData() passed bad settings");
    abort(); //return();
  }
  MCP_CAN subjCAN = *(*settings).canBus;

  char inChar;
  uint8_t LCDMsg1[8];
  uint8_t LCDMsg2[8];
  static uint8_t tickCount = 0;

  BUSMsg28F[0] += extras1; // Sets segments above free text (CD IN etc.)
  BUSMsg28F[1] += extras2;
  BUSMsg28F[3] = formatting; // Sets apostrophes and colons

  if ((tickCount == 0) && digitalRead(IN_BUTTON)){
    tickCount = 1;
    
    // Simulate button press on radio
    if (!accHold){
      Serial.println("[msg] SET");
      BUSMsg28F[4] = 40;
    } else {
      displayPage +=1 ;
    }
  }
  if (tickCount != 0){
    tickCount++;
  }
  if (tickCount >= 5 && !digitalRead(IN_BUTTON)){ // if button is no longer held
    tickCount = 0;
  }
  if ((tickCount % 21) == 20){
    accHold = !accHold;
  }
  
  if (accHold)
    BUSMsg28F[1] += 0b10000000; // Turn on PTY section

  if (Serial.available() > 0) {
    inChar = Serial.read();
    switch (inChar){
    
      case 'S':
        Serial.println("[msg] SET");
        BUSMsg28F[4] = 40;
        break;

      case 'C':
        Serial.println("[msg] CLOCK");
        BUSMsg28F[4] = 48;
        break;

      case 'D':
        Serial.println("[msg] DISON");
        BUSMsg28F[4] = 160;
        break;

      #ifdef _WITHANALYSE
      case 'A':
        Serial.println("[option] analysis ");
        (*settings).analysisEnabled = !(*settings).analysisEnabled;
        break;
      #endif

      case 'L':
        Serial.println("[option] datalogging ");
        (*settings).loggingEnabled = !(*settings).loggingEnabled;
    }
  }

  while (Serial.available() > 0) {
    inChar = Serial.read(); // clear buffered up messages
  }

  LCDMsg1[0] = 192; // Weird magic value, some others may be for scrolling
  for (int i = 0; i < 7; i++){
    LCDMsg1[i+1] = inStr[i];
  }

  LCDMsg2[0] = 135; // Ditto the above
  for (int i = 0; i < 6; i++){
    LCDMsg2[i+1] = inStr[i+7];
  }

  subjCAN.sendMsgBuf(0x28F, 0, 8, BUSMsg28F);
  subjCAN.sendMsgBuf(0x290, 0, 8, LCDMsg1);
  subjCAN.sendMsgBuf(0x291, 0, 8, LCDMsg2);
  digitalWrite(OUT_WASHER, decideShiftLight(settings));
}

char guessGear(VehicleData carState){
  char retval;
  int ratio;
  int last;

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

  if (retval == 'R'){
    digitalWrite(A0,LOW);
    last = LOW;
  } else {
    digitalWrite(A0,HIGH);
    last = HIGH;
  }
  return(retval);
}

void stateToSerial(DeviceState *settings){
  VehicleData *carState = (*settings).carState;
  static boolean firstLine = true;
  static uint32_t sample = 0;
  
  char *bufferStr = (char*)calloc(100,sizeof(char));
  
  
  // since we have no RTC we are not going to bother with the time
  // 
  
  if (firstLine){
    Serial.print("SAMPLE,RPM,SPEED,THROTTLE,T_COOL_ENG,FUEL_COUN,FUEL_TOT,");
    Serial.print("TRIP_DISTREM,TRIP_USE_INST,INDI_LEFT,INDI_RIGHT;\r\n");
    firstLine = false;
  }
  sprintf(bufferStr, "%li,%i,%u,%u,%u,%lu,%u,%u,%u,%u;\r\n", 
    sample,
    (*carState).engineRPM,
    (*carState).throttlePosition,
    (*carState).engineCoolTemp,
    (*carState).fuelUseCounter,
    (*carState).fuelUsed,
    (*carState).tripDistRemain,
    (*carState).tripUsageCur,
    (uint8_t)(*carState).indicatorLeft,
    (uint8_t)(*carState).indicatorRight
  );
  Serial.print(bufferStr);
  free(bufferStr);
  sample++;
}
#endif


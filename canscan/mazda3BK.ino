/* Nathaniel Roach*/
#include "mazda3BK.h"

#ifdef _MAZDA3BK

void mazdaBKLCDPrint(MCP_CAN subjCAN, char inStr[], uint8_t formatting, 
    boolean *analyseActive){
  //uint8_t LCDText1[8] = {192, L1, L2, L3, L4, L5, L6, L7};
  //uint8_t LCDText2[8] = {135, L8, L9, L10, L11, L12, 32, 32};
  //uint8_t outputText[13] = {L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, 0}
  uint8_t BUSMsg28F[8] = {128, 0, 0, 0, 32, 0, 0, 0};
  //                                    ^ Set this to two to see all valid
  //                                      formatting locations

  char outputText[13]; // 12 chars + NUL
  char inChar;
  uint8_t LCDMsg1[8];
  uint8_t LCDMsg2[8];

  BUSMsg28F[3] = formatting; // Sets apostrophes and colons

  if (Serial.available() > 0) {
    inChar = Serial.read();
    if (inChar == 'S'){
      Serial.println("Sending 28F - SET");
      BUSMsg28F[4] = 40;
    } else if (inChar == 'C'){
      Serial.println("Sending 28F - CLOCK");
      BUSMsg28F[4] = 48;
    } else if (inChar == 'D'){
      Serial.println("Sending 28F - DISON");
      BUSMsg28F[4] = 160;
    #ifdef _WITHANALYSE
    } else if (inChar == 'A'){
      Serial.println("Toggling Analysis");
      *analyseActive = !(*analyseActive);
    #endif
    }
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


/* Nathaniel Roach*/
#include "mazda3BK.h"

void mazdaBKLCDPrint(MCP_CAN subjCAN, char inStr[], uint8_t formatting){
  //uint8_t LCDText1[8] = {192, L1, L2, L3, L4, L5, L6, L7};
  //uint8_t LCDText2[8] = {135, L8, L9, L10, L11, L12, 32, 32};
  //uint8_t outputText[13] = {L1, L2, L3, L4, L5, L6, L7, L8, L9, L10, L11, L12, 0}
  uint8_t BUSMsg28F[8] = {128, 0, 0, 0, 32, 0, 0, 0};
  //                                    ^ Set this to two to see all valid
  //                                      formatting locations
  
  char outputText[13]; // 12 chars + NUL, also toCharArray is a bitch
  char inChar;
  uint8_t LCDMsg1[8];
  uint8_t LCDMsg2[8];

  BUSMsg28F[3] = formatting;
  
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
    }
  }

  
  //inStr.toCharArray(outputText, 12);
  
  //Serial.print(inStr);
  
  for (int i=0; i < 13; i++){
    outputText[i] = inStr[i];
  }
  //Serial.println(";");
  
  LCDMsg1[0] = 192;
  for (int i = 0; i < 7; i++){
    LCDMsg1[i+1] = outputText[i];
  }
  
  LCDMsg2[0] = 135;
  for (int i = 0; i < 6; i++){
    LCDMsg2[i+1] = outputText[i+7];
  }
  
  Serial.println(BUSMsg28F[4]);
  
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
    if (ratio > 130 && ratio < 170){
      retval = '1';
    } else if (ratio > 70 && ratio < 83){
      retval = '2';
    } else if (ratio > 42 && ratio < 47){
      retval = '3';
    } else if (ratio > 34 && ratio < 38){
      retval = '4';
    } else {
      retval = '?';
    }
  }
  
  return(retval);
}

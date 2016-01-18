// (C) Nathaniel Roach 2015
// GNU GPLv2 
// analyse() will store CAN messages in a linked list, and print them out when
//    stored, and when they change. Disable the timer, add to loop() to use it.
// Currently will print out statistics to the MAZDA 3 2007 (BK) Information display

extern "C"{
  #include <naz-linkList.h>
}

#include <SPI.h>
#include <mcp_can.h> // MCP2515 CAN Controller library from seeed-studios

#include <MsTimer2.h>

extern "C"{
  typedef struct {
    uint16_t ID;
    uint8_t length;
    uint8_t data[8];
  } BusMessage;
  
  typedef struct {
    int16_t bodySpeed; // 100 = 1km/h; can be negative but not known if signed
    uint16_t engineRPM;
    uint8_t throttlePedal;
    uint16_t tripSpeedAvg;
    uint16_t tripUsageCur; // 22 = 2.2l/100km
    uint16_t tripUsageAvg;
    uint16_t tripDistRemain;
    uint8_t doorState;
    boolean gearReverse;
    boolean handbrake;
  } VehicleData;
}

String infoDisplayStr;
uint8_t displayPage = 1;
const int SPI_CAN1CS_PIN = 9;
const int SPI_CAN2CS_PIN = 8;
MCP_CAN CAN1(SPI_CAN1CS_PIN);
LinkedList* msgIndex;
VehicleData carState;
//MCP_CAN CAN2(SPI_CAN2CS_PIN);

/*
uint8_t BUSMsg28FSetup[8] = {128, 0, 0, 0, 128, 0, 0, 0}; */

uint8_t BUSMsg28FClockButton[8] = {128, 0, 0, 0, 48, 0, 0, 0};
uint8_t BUSMsg28FSetButton[8] =   {128, 0, 0, 0, 40, 0, 0, 0};
uint8_t BUSMsg28FIdle[8] =        {128, 0, 0, 0, 32, 0, 0, 0};
uint8_t BUSMsg28FDISOn[8] =       {128, 0, 0, 0,160, 0, 0, 0};
uint8_t BUSMsg401ACK[1] = {12}; /*
uint8_t LCDText1[8] = {192, ' ', ' ', ' ', 'M', 'a', 'd', 'M'};
uint8_t LCDText2[8] = {135, 'a', 'z', ' ', ' ', ' ', 32, 32};
uint8_t LCDText1Alt[8] = {192, 'H', 'i', ' ', 'K', 'a', 'l', 'd'};
uint8_t LCDText2Alt[8] = {135, 'e', 'n', '!', ' ', ' ', ' ', ' '};
uint8_t LCDClear1[8] = {192, 32, 32, 32, 32, 32, 32, 32};
uint8_t LCDClear2[8] = {135, 32, 32, 32, 32, 32, 32, 32};
uint8_t LCDText1DISOn[8] = {192, 32, 68, 73, 83, 32, 79, 78}; */

void setup(){
  Serial.begin(115200);
  msgIndex = linkedListCreate();

CAN1START_INIT:
  if(CAN_OK == CAN1.begin(CAN_125KBPS)){
    Serial.println("CAN1 INIT OK");
  } else {
    Serial.println("CAN1 INIT FAIL");
    delay(100);
    goto CAN1START_INIT;
  }
/*
CAN2START_INIT:
  if(CAN_OK == CAN2.begin(CAN_125KBPS)){
    Serial.println("CAN2 INIT OK");
  } else {
    Serial.println("CAN2 INIT FAIL");
    delay(100);
    goto CAN2START_INIT;
  } */
  
  MsTimer2::set(1, handleTimer); // 500ms period
  MsTimer2::start();
}

void trigger(MCP_CAN subjCAN){
  static int triggered = -1;
  uint16_t msgID;
  uint8_t msgLen = 0;
  uint8_t msg[8];
  //Serial.print("x");
  if(CAN_MSGAVAIL == subjCAN.checkReceive())    {
    subjCAN.readMsgBuf(&msgLen, msg);
    msgID = subjCAN.getCanId();
    
    if (triggered == -1 && msgID == 0x28F && msg[4] == 40){
      triggered = 0;
      Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
    if (triggered <= 30 && triggered != -1 /*&& 
      msgID != 0x023 &&
      msgID != 0x201 &&
      msgID != 0x265 &&
      msgID != 0x285 &&
      msgID != 0x28F &&
      msgID != 0x290 &&
      msgID != 0x291 &&
      msgID != 0x400 &&
      msgID != 0x401 &&
      msgID != 0x420 &&
      msgID != 0x433 &&
      msgID != 0x460 &&
      msgID != 0x4F0 &&
      msgID != 0x501 &&
      msgID != 0x50C &&
      msgID != 0x511 */)
    {
      Serial.print(msgID, HEX);
      Serial.print(":");
      Serial.print(msgLen);
      Serial.print(":");
      if (msgLen >= 1){
        Serial.print(msg[0]); 
      }
      for(int i = 1; i<msgLen; i++)    // print the data
      {
        Serial.print(","); 
        Serial.print(msg[i]);             
      }
      if (msgID == 0x290 || msgID == 0x291){
        Serial.print("|");
        for(int i = 1; i<msgLen; i++)
        {
           Serial.write(msg[i]);
        }
        Serial.print("|");
      }
      Serial.println();
      triggered++;
      //trigger = 0;
    } if (triggered == 31){
          Serial.println("END RECORDING");
          triggered = 0;
    }
  }
}

LinkedListNode *linkedListFind(LinkedList *subjList, uint16_t msgID){
  boolean looping;
  uint8_t iterations = 0;
  BusMessage *subjMsg; 
  LinkedListNode *retval, *subjNode;
  uint8_t listSize = linkedListGetSize(subjList);
  if (listSize == 0){
    retval =  nullptr;
  } else {
    looping = true;
    subjNode = (*subjList).head;
    while (looping){
      subjMsg = (BusMessage*)(*subjNode).data;
      if ((*subjMsg).ID == msgID){
        looping = false;
        retval = subjNode;
      } else {
        iterations++;
        subjNode = (*subjNode).follower;
        if (subjNode == nullptr){
          looping = false;
          retval = nullptr;
        }
      }
    }
  }
  return retval;
}

void analyse(MCP_CAN subjCAN, LinkedList *subjList){
  LinkedListNode *subjNode;
  BusMessage *subjMsg;
  boolean changed = false;
  uint16_t msgID;
  uint8_t msgLen;
  uint8_t msg[8];
  
  if(CAN_MSGAVAIL == subjCAN.checkReceive())    {
    subjCAN.readMsgBuf(&msgLen, msg); // Message needs to be pulled from the chip
    msgID = subjCAN.getCanId();       // Before the ID can be read
    subjNode = linkedListFind(subjList, msgID);
    //if (msgID == 0x401 && msg[0] == 40){
    //  Serial.println("ACKing...");
    //  CAN1.sendMsgBuf(0x401, 0, 8, BUSMsg401ACK);
    //}
    if (msgID != 0x501 && msgID != 0x511 && msgID != 0x420){
      if (subjNode != nullptr){
        subjMsg = (BusMessage*)(*subjNode).data;
        // Compare the new message with the old one
        if (msgLen == (*subjMsg).length){ // If it's the same length
          for (int i = 0; i < msgLen; i++){ // Check all bytes
            if (msg[i] != (*subjMsg).data[i]){
              if (changed == false){
                Serial.print(" AT:"); // Print out locations of the deltas
              } else {
                Serial.print(":");
              }
              Serial.print(i);
              changed = true;
            }
          }
          if (changed){ // Print out the changed message
            Serial.println();
            Serial.print("MOD:");
            Serial.print(msgID, HEX);
            Serial.print(":");
            Serial.print(msgLen);
            if (msgLen > 0){
              Serial.print(":");
              Serial.print(msg[0]);
              (*subjMsg).data[0] = msg[0]; // Update the stored message
              for (int i = 1; i < msgLen; i++){
                Serial.print(",");
                Serial.print(msg[i]);
                (*subjMsg).data[i] = msg[i];
              }
              if (CAN1.isRemoteRequest()){
                Serial.println("?");
              } else {
                Serial.println();
              }
            }
          }
        } else { // Apparently the length changed, probably shouldn't happen
          Serial.print("LEN:");
          Serial.print(msgID, HEX);
          Serial.print(":");
          Serial.print(msgLen);
          Serial.print(", was ");
          Serial.println((*subjMsg).length);
          (*subjMsg).length = msgLen;
        }
      } else { // It's not a message we've already seen, stash it
        subjMsg = (BusMessage*)calloc(1, sizeof(BusMessage));
        if (subjMsg == nullptr){
          Serial.println("Error allocating memory, aborting");
          abort();
        }
        Serial.print("NEW:");
        Serial.print(msgID, HEX);
        (*subjMsg).ID = msgID;
        
        Serial.print(":");
        Serial.print(msgLen);
        (*subjMsg).length = msgLen;
        
        if (msgLen > 0){
          Serial.print(":");
          Serial.print(msg[0]);
          (*subjMsg).data[0] = msg[0];
          for (int i = 1; i < msgLen; i++){
            Serial.print(",");
            Serial.print(msg[i]);
            (*subjMsg).data[i] = msg[i];
          }
        }
        Serial.println();
        linkedListAppend(subjList, subjMsg);
      } // End else add to list
    } // end if not 0x920
  } // end if new message
}

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
  
  subjCAN.sendMsgBuf(0x28F, 0, 8, BUSMsg28F);
  subjCAN.sendMsgBuf(0x290, 0, 8, LCDMsg1);
  subjCAN.sendMsgBuf(0x291, 0, 8, LCDMsg2);
}

char guessGear(uint16_t inRPM, int8_t inSpeed){
  char retval;
  
  if (inSpeed == 0){
    retval = 'N';
  } else if (inSpeed < 0){
    retval = 'R';
  } else {
    retval = '?';
  }
  
  return(retval);
}

void formatScreen(){
  char output[13] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
  uint8_t formatting = 0;
  
/*
            |
Axx Rxxxx G | Pedal RPM Gear Guess
 Rxxx C---  | Remaining CurCons
 C--- S---  | AvgCons AvgSpeed
R.Right Open|
 Boot Open  |
   MadMaz   
*/

  if (carState.doorState != 0){
    //Serial.println(carState.doorState);
    
    if (carState.doorState & 1<<3) {
      sprintf(output, "   Boot Open");
    } else if (carState.doorState & 1<<4){
      sprintf(output, " R.Right Open");
    } else if (carState.doorState & 1<<5){
      sprintf(output, "R.Left Open");
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
          if (carState.throttlePedal == 200){
            sprintf(output, "WOT R%04i G%c", carState.engineRPM, guessGear(
                  carState.engineRPM, carState.bodySpeed));
          } else {
            sprintf(output, "A%02i R%04i G%c", carState.throttlePedal/2, 
                  carState.engineRPM, guessGear(carState.engineRPM, carState.bodySpeed));
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
        sprintf(output, "  Speed %05i", carState.bodySpeed);
        break;
      }
  }
  mazdaBKLCDPrint(CAN1, output, formatting);
}

void getData(MCP_CAN subjCAN){
  uint16_t msgID;
  uint8_t msgLen;
  uint8_t msg[8];
  
  if(CAN_MSGAVAIL == subjCAN.checkReceive())    {
    subjCAN.readMsgBuf(&msgLen, msg); // Message needs to be pulled from the chip
    msgID = subjCAN.getCanId();       // Before the ID can be read
    switch(msgID) {
      case 0x201 :
        carState.engineRPM = (uint16_t)(msg[0]*256 + msg[1]);
        if (carState.engineRPM > 9000){
          carState.engineRPM = 0;
        }
        carState.bodySpeed = (uint16_t)(msg[4]*256 + msg[5]);
        carState.throttlePedal = msg[6];
        break;
    
      case 0x400 :
        carState.tripSpeedAvg = (uint16_t)(msg[0]*256 + msg[1]);
        carState.tripUsageCur = (uint16_t)(msg[2]*256 + msg[3]);
        carState.tripUsageAvg = (uint16_t)(msg[4]*256 + msg[5]);
        carState.tripDistRemain = (uint16_t)(msg[6]*256 + msg[7]);
        break;
        
      case 0x433 :
        carState.doorState = msg[0];
        carState.handbrake = (msg[3] & 1<<0);
        carState.gearReverse = (msg[3] & 1<<1);
        
      //default:
        //Not a message we know/want
    }
  }
}

void handleTimer(){
  MsTimer2::stop();
  static uint16_t macroCycles = 0;
  macroCycles++;
  
  if (macroCycles >= 200){
    formatScreen();
    macroCycles = 0;
  } else {
    getData(CAN1);
  }
  MsTimer2::start();
}

void loop(){}

void loop2(){
  static boolean idleSent = false;
  static int counter = 0;
  uint8_t inChar;
  analyse(CAN1, msgIndex);
  counter++;
  if (Serial.available() > 0) {
    inChar = Serial.read();
    if (inChar == 'S'){
      Serial.println("Sending 28F - SET");
      CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FSetButton);
      counter = 0;
      idleSent = false;
    } else if (inChar == 'C'){
      Serial.println("Sending 28F - CLOCK");
      CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FClockButton);
      counter = 0;
      idleSent = false;
    } else if (inChar == 'D'){
      Serial.println("Sending 28F - DISON");
      CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FDISOn);
      counter = 0;
      idleSent = false;
    }
  }
  
  if (idleSent == false && counter > 4000){
    Serial.println("Sending 28F__");
    CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FIdle);
    idleSent = true;
  }

}

/*
void loopKalden(){
  bool triggered = false;
  uint8_t subjMsg[8];
  uint8_t subjMsgLen;
  //trigger(CAN1);
  //analyse(CAN1, msgIndex);
  
  while (1){
    while (!triggered){
        CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FSetup);
        CAN1.sendMsgBuf(0x290, 0, 8, LCDText1Alt);
        CAN1.sendMsgBuf(0x291, 0, 8, LCDText2Alt);
        delay(20);
        if(CAN_MSGAVAIL == CAN1.checkReceive())    {
            CAN1.readMsgBuf(&subjMsgLen, subjMsg);
            if (CAN1.getCanId() == 0x201 && subjMsg[6] >= 10){
                triggered = true;
            }
        }
    }
    CAN1.sendMsgBuf(0x28F, 0, 8, BUSMsg28FSetup);
    CAN1.sendMsgBuf(0x290, 0, 8, LCDText1);
    CAN1.sendMsgBuf(0x291, 0, 8, LCDText2);
    delay(100);
  }
} */

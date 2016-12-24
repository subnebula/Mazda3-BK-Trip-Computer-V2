/* Nathaniel Roach*/

#include "canfunctions.h"

#ifdef _INCLUDE_CANFUNC

BusMessage getMessage(MCP_CAN subjCAN){
  BusMessage subjMsg;
  subjMsg.ID = 0;

  if(CAN_MSGAVAIL == subjCAN.checkReceive()){
    subjCAN.readMsgBuf(&(subjMsg.length), subjMsg.data); // Message needs to be pulled from the chip
    subjMsg.ID = subjCAN.getCanId();                // Before the ID can be read
  }
  return subjMsg;
}

void CAN2UART(char *prefix, uint16_t msgID, uint8_t msgLen, uint8_t *msg,
    boolean printRaw){
  if (msg == NULL){return;}
  if (prefix != NULL){
    Serial.print(prefix);
  }

  Serial.print(msgID, HEX);
  Serial.print(":");
  Serial.print(msgLen);
  Serial.print(":");
  if (msgLen >= 1){
    Serial.print(msg[0]);
  }
  for(int i = 1; i<msgLen; i++)    // print the data as ints
  {
    Serial.print(",");
    Serial.print(msg[i]);
  }
  if (printRaw){
    Serial.print("|");
    for(int i = 1; i<msgLen; i++)
    {
       Serial.write(msg[i]);
    }
    Serial.print("|");
  }
  Serial.println();
}

//Triggers when ID[INDEX] = CONTENTS
// Call every time you recieve a message, or just in a loop
void trigger(MCP_CAN subjCAN, uint16_t triggerOnID, uint16_t triggerOnIndex,
    uint16_t triggerOnContents){

  static int triggered = -1;
  uint16_t msgID;
  uint8_t msgLen = 0;
  uint8_t msg[8];
  //Serial.print("x");
  if(CAN_MSGAVAIL == subjCAN.checkReceive())    {
    subjCAN.readMsgBuf(&msgLen, msg);
    msgID = subjCAN.getCanId();

    if (triggered == -1 && msgID == triggerOnID && msg[triggerOnIndex] == triggerOnContents){
      triggered = 0;
      Serial.println("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
    }
    if (triggered <= 30 && triggered != -1)
    {
      Serial.print(msgID, HEX);
      Serial.print(":");
      Serial.print(msgLen);
      Serial.print(":");
      if (msgLen >= 1){
        Serial.print(msg[0]);
      }
      for(int i = 1; i<msgLen; i++)    // print the data as ints
      {
        Serial.print(",");
        Serial.print(msg[i]);
      }
      if (msgID == 0x290 || msgID == 0x291){ // Specific to the mazda, change to
        Serial.print("|");                   // true to print all bytes recieved
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

void analyse(MCP_CAN subjCAN, BinaryTree *subjTree){
  BusMessage subjMsg = getMessage(subjCAN);

  analyseMessage(subjMsg, subjTree);
}

void analyseMessage(BusMessage inMsg, BinaryTree *subjTree){
  BinaryTreeNode *subjNode;
  BusMessage *subjMsg; // The stored message
  boolean changed = false;

  if (inMsg.ID != 0 && inMsg.ID != 0x501 && inMsg.ID != 0x511){
    subjNode = binaryTreeFindR(subjTree, inMsg.ID, 0); // fetch existing message
    if (subjNode != nullptr){
      subjMsg = (BusMessage*)(*subjNode).data;
      // Compare the new message with the old one
      if (inMsg.length == (*subjMsg).length){ // If it's the same length
        for (int i = 0; i < inMsg.length; i++){ // Check all bytes
          if (inMsg.data[i] != (*subjMsg).data[i]){
            if (changed == false){ // if it's the first time we've noticed
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
          Serial.print(inMsg.ID, HEX);
          Serial.print(":");
          Serial.print(inMsg.length);
          if (inMsg.length > 0){
            Serial.print(":");
            Serial.print(inMsg.data[0]);
            (*subjMsg).data[0] = inMsg.data[0]; // Update the stored message
            for (int i = 1; i < inMsg.length; i++){
              Serial.print(",");
              Serial.print(inMsg.data[i]);
              (*subjMsg).data[i] = inMsg.data[i];
            }
            Serial.println();
          }
        }
      } else { // Apparently the length changed, probably shouldn't happen
        Serial.print("LEN:");
        Serial.print(inMsg.ID, HEX);
        Serial.print(":");
        Serial.print(inMsg.length);
        Serial.print(", was ");
        Serial.println((*subjMsg).length);
        (*subjMsg).length = inMsg.length;
      }
    } else { // It's not a message we've already seen, stash it
      subjMsg = (BusMessage*)calloc(1, sizeof(BusMessage));
      if (subjMsg == nullptr){
        Serial.println("Error allocating memory, aborting");
        abort();
      }
      Serial.print("NEW:");
      Serial.print(inMsg.ID, HEX);
      (*subjMsg).ID = inMsg.ID;

      Serial.print(":");
      Serial.print(inMsg.length);
      (*subjMsg).length = inMsg.length;

      if (inMsg.length > 0){
        Serial.print(":");
        Serial.print(inMsg.data[0]);
        (*subjMsg).data[0] = inMsg.data[0];
        for (int i = 1; i < inMsg.length; i++){
          Serial.print(",");
          Serial.print(inMsg.data[i]);
          (*subjMsg).data[i] = inMsg.data[i];
        }
      }
      Serial.println();
      binaryTreeInsert(subjTree, (*subjMsg).ID, subjMsg);
    } // End else add to list
  }
}
#endif


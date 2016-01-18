/* Nathaniel Roach*/

#include "canfunctions.h"

#ifdef _INCLUDE_CANFUNC

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
  // Change these values
  /*
  const uint16_t triggerOnID = 0x28F;
  const uint16_t triggerOnIndex = 4;
  const uint16_t triggerOnContents = 40; */

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


#endif

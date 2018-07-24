#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <painlessMesh.h> // Wireless library which forms mesh network https://github.com/gmag11/painlessMesh
#include <ArduinoJson.h>
#include "Communication.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////INDEX////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 0. Setup variables...
// 1. Info regarding the state model
// 2. Inbox/outbox maintaiance
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////0. Info regarding the state model//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Parameters for the the WIFI mesh network
#define   BLINK_PERIOD    3000000 // microseconds until cycle repeat
#define   BLINK_DURATION  100000  // microseconds LED is on for
#define   MESH_SSID       "mblocks3d"
#define   MESH_PASSWORD   "mblocks3d"
#define   MESH_PORT       5555

// Class instance for the WIFI mesh
painlessMesh mesh;

/* INBOX
   These variables hold messages that need to be sent, and recieved messages that need to be
   processed
*/
inboxEntry inbox[INBOX_SIZE]; //inbox stores incomming messages into an array of mailboxes
            
int inboxHead = 0; // Current empty position to add more messages
int inboxTail = 0; // The oldest message in the buffer... first one to process

/** OUTBOX
   In the outbox, we need to keep track of each message that we are transmitting
   We represent the outbox as a 2D array of outboxEntry Objects.
   To access the current message for cube with ID # 4, we would access it with:
      outbox[5][outboxTail[5]...
*/
outboxEntry outbox[NUM_CUBES][OUTBOX_SIZE] ; // 2D Array of OutboxEntry Instances
int outboxHead[NUM_CUBES] = {};
int outboxTail[NUM_CUBES] = {};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////1. Info regarding the state model//////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateStateModel(int cubeID)
{
  for (int i=0; i<6; i++)
  {
    //cubesState[cubeID][i] = inbox[inboxTail].faceStates[i];
  }
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////2. INBOX / OUTBOX///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
   This function checks the inbox to see if it is an ack for a message currently in the outbox.
   If so, it clears both the inbox and the outbox.
   If not, or if no message is in the inbox, it checks to see if the outbox message needs to be resent 
   (and does so if needed).
*/

void updateBoxes()
{
  // Clear out the inbox
  while (!inboxIsEmpty()) // This means there is something in the inbox
  {
    Serial.println("Checking to see if message matches...");
    Serial.print("senderID: ");
    Serial.println(inbox[inboxTail].senderID);
    int cubeThatSentMessage = inbox[inboxTail].senderID;
    if (outbox[cubeThatSentMessage][outboxTail[cubeThatSentMessage]].mID == inbox[inboxTail].mID) 
    /*
     * This massive if statement checks to see if the message ID from the most recent message matched the 
     * outbox entry for that cube e.g. if cube_3 sent a message and it is in the inbox with messageID of 666, 
     * then we look in outbox[Cube_3] and check to see if the message ID is 666.
     * 
     * If so then we have received the ACK for messageID# 666, and we cross it off the list, 
     * update our state, and then move to the next item in the inbox
     */
    {
       // if inbox is ack for outbox
       updateStateModel(inbox[inboxTail].senderID); // process ack for the cube 
       outbox[inbox[inboxTail].senderID][outboxTail[inbox[inboxTail].senderID]].mID = 0; // clear outbox entry
       advanceOutboxTail(inbox[inboxTail].senderID); // move on to next outbox slot
       Serial.print("senderID: ");
       Serial.println(inbox[inboxTail].senderID);
       Serial.println("Clearing out Message");
    }
    else 
    {       
      Serial.println("Spurious ACK");
      /*
       * If the message received does not match the item in the outbox...
       * We are still going to process the message... 
       *  1. Add the cubeID to the list of active cubes
       *  2. 
       */
    }
    inbox[inboxTail].mID = 0; // clear inbox
    advanceInboxTail();
  }
  
  mesh.update();

  /*
   * We have now processed all of the incomming messages by doing the following:
   *  1. Crossed the message off of the outbox it matched a message we previously sent
   *  2. Added the information to our database of cubes
   *  
   * Now we need to go through the entire outbox and send messages if their time limit as 
   * been reached. 
   * 
   * We also need to check and see if a message has taken too many tries to send, if so
   * we just delete the message from the que.
   */
  for(int cubeID = 0; cubeID < NUM_CUBES; cubeID++)
    /* loop over all cubes, send outbox messages if the time for that specific message has 
     *  expired
     */
  {
    if (outbox[cubeID][outboxTail[cubeID]].mID != 0) 
    /*
     * If the outboxTail for the current cube cubeID is not set to be zero, then we
     * assume that there is a message in the outbox
     */
    { 
      if (millis() > outbox[cubeID][outboxTail[cubeID]].mDeadline)
      /*
       * Now we check to see if it is time to check the message by comparing
       * the system time (Long) to the outbox.cube_ID.Tail.Deadline 
       */
      {
        //generate message
        sendMessage(cubeID, generateMessageText(outbox[cubeID][outboxTail[cubeID]].cmd, outbox[cubeID][outboxTail[cubeID]].mID)); // send it...
        outbox[cubeID][outboxTail[cubeID]].mDeadline = 
        millis() + random((1UL << outbox[cubeID][outboxTail[cubeID]].backoff) * AVERAGE_FIRST_DELAY_MS * 2);
        // set the next deadline using exponential backoff,
        
        outbox[cubeID][outboxTail[cubeID]].backoff++;
        // and increment the counter to reflect the number of tries.
        Serial.print(" Just Processed outbox for Cube #:");
        Serial.println(cubeID);
        Serial.print("mID: ");
        Serial.println(String(outbox[cubeID][outboxTail[cubeID]].mID));
        Serial.print("cmd: ");
        Serial.println(String(outbox[cubeID][outboxTail[cubeID]].cmd));
        Serial.print("backoff: ");
        Serial.println(String(outbox[cubeID][outboxTail[cubeID]].backoff));
        Serial.print("mDeadline: ");
        Serial.println(String(outbox[cubeID][outboxTail[cubeID]].mDeadline));
        Serial.print("senderID: ");
        Serial.println(String(outbox[cubeID][outboxTail[cubeID]].senderID));
      }
    }
  }
}

void receivedCallback(uint32_t from, String & stringMsg)
/*
 * This function gets called by the mesh library whenever we receive a wifi Message
 * 
 * The goal is to check to see if it is a new message, and if so, we add it to a 
 * list of messages to be processed by inboxbuffer
 */
{
  Serial.print("Received message from ");
  Serial.println(from);
  Serial.println("Message Contents:");
  Serial.println(stringMsg);
  // if Inbox is full...
  // Use bool inboxIsFull()

  if (inbox[inboxHead].mID == 0) //if there is space in the inbox circular buffer
  {
    StaticJsonBuffer<256> jsonMsgBuffer; // allocate memory for json
    JsonObject& jsonMsg = jsonMsgBuffer.parseObject(stringMsg); // parse message
    int senderCubeID = jsonMsg["sID"];
    int tempBottomFace = jsonMsg["bFace"];
    /*
     * Update the parameters that go into the ibox entry
     */
    inbox[inboxHead].mID = jsonMsg["mID"];
    inbox[inboxHead].bottomFace = tempBottomFace;
    inbox[inboxHead].senderID = senderCubeID;

    /*
     * Update the parameters in the database
     */
    database[
    database[senderCubeID][face_0] = jsonMsg["f0"];
    database[senderCubeID][face_1] = jsonMsg["f1"];
    database[senderCubeID][face_2] = jsonMsg["f2"];
    database[senderCubeID][face_3] = jsonMsg["f3"];
    database[senderCubeID][face_4] = jsonMsg["f4"];
    database[senderCubeID][face_5] = jsonMsg["f5"];

    /*
     * We have now processed this message ... so message is 
     */
    advanceInboxHead();
  }
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Connection Event\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {}

void delayReceivedCallback(uint32_t from, int32_t delay) {}

String generateMessageText(String cmd, uint32_t mID)
/*
 * This function takes a string "cmd" which is the message to be sent,
 * and it takes the message ID.
 * 
 * It then formats them in the JSON format, and returns the string containing
 * all of the formatted message, including the following components
 * 
 * message ID       | "mID"   |  
 * message type     | "type"  |
 * sender ID        | "sID"   |
 * command to send  | "cmd"   |
 */
{
  if (mID == 0) {
    mID = advanceLfsr();
  }
  StaticJsonBuffer<512> jsonBuffer; // Space Allocated to construct json instance
  JsonObject& jsonMsg = jsonBuffer.createObject(); // & is "c++ reference"
  //jsonMsg is our output, but in JSON form

  jsonMsg["mID"] =  mID;
  jsonMsg["type"] = "c"; // "c" for command...
  jsonMsg["sID"] =  SERVER_ID;
  jsonMsg["cmd"] =  cmd;
  String strMsg; // generate empty string
  //strMsg is our output in String form
  jsonMsg.printTo(strMsg); // print to JSON readable string...
  return strMsg;
}

void pushBlinkMessage(int cubeID)
{
  pushMessage(cubeID, "b");
  Serial.println("Pushing Blink Message to: ");
  Serial.println(cubeID);
}

void pushForwardMessage(int cubeID)
{
  pushMessage(cubeID, "f");
  Serial.println("Pushing Forward Message to: ");
  Serial.println(cubeID);
}

void pushReverseMessage(int cubeID)
{
  pushMessage(cubeID, "r");
  Serial.println("Pushing Reverse Message to: ");
  Serial.println(cubeID);
}

void pushSleepMessage(int cubeID)
{
  pushMessage(cubeID, "s");
  Serial.println("Pushing Sleep Message to: ");
  Serial.println(cubeID);
}

void pushStatusMessage(int cubeID)
{
  pushMessage(cubeID, "q");
  Serial.println("Pushing Status Message to: ");
  Serial.println(cubeID);
}

void pushColorMessage(int cubeID, char Color)
{
  //pushMessage(cubeID, char(Color));
  Serial.println("Pushing Color Message to: ");
  Serial.println(cubeID);
}

void pushMessage(int cubeID, String command)
{
  if (getAddressFromCubeID(cubeID) == 0)
    return;
  if (outboxIsFull(cubeID))
    return;
  outbox[cubeID][outboxHead[cubeID]].mID = advanceLfsr();
  outbox[cubeID][outboxHead[cubeID]].senderID = SERVER_ID;
  outbox[cubeID][outboxHead[cubeID]].backoff = 0;
  outbox[cubeID][outboxHead[cubeID]].mDeadline = 0;
  outbox[cubeID][outboxHead[cubeID]].cmd = command;
  advanceOutboxHead(cubeID);
}

// Outbox circular buffer functions
void advanceOutboxHead(int cubeID)
{
  outboxHead[cubeID]++;
  if (outboxHead[cubeID] == OUTBOX_SIZE) outboxHead[cubeID] = 0;
}

void advanceOutboxTail(int cubeID)
{
  outboxTail[cubeID]++;
  if (outboxTail[cubeID] == OUTBOX_SIZE) outboxTail[cubeID] = 0;
}

bool outboxIsFull(int cubeID)
{
  return outbox[cubeID][outboxHead[cubeID]].mID != 0;
}

// Inbox circular buffer functions
void advanceInboxHead()
{
  inboxHead++;
  if (inboxHead == NUM_CUBES) inboxHead = 0;
}

void advanceInboxTail()
{
  inboxTail++;
  if (inboxTail == NUM_CUBES) inboxTail = 0;
}

bool inboxIsFull()
{
  return inbox[inboxHead].mID != 0;
}

bool inboxIsEmpty()
{
  return (inbox[inboxTail].mID == 0);
}

// Cube Data Object
int cubesState[NUM_CUBES][6];

// Misc Helper Functions

uint32_t advanceLfsr() // this call returns a message ID. these are not sequential.
{
  static uint32_t lfsr = 0xfefefe;

  uint32_t lsb = lfsr & 0x01u;
  lfsr >>= 1;
  if (lsb)
  {
    lfsr ^= (1u << 31) | (1u << 21) | (1u << 1) | (1u << 0);
  }
  return lfsr;
}

bool calc_delay = false;
SimpleList<uint32_t> nodes;

void initializeWifiMesh()
{
  mesh.init(MESH_SSID, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);
  randomSeed(analogRead(A0));
}


bool sendMessage(int recipientID, String msg)
{
  Serial.println("sending: ");
  Serial.println(msg);
  if (recipientID == -1)
  {
    return (mesh.sendBroadcast(msg));
  }
  else
  {
    uint32_t address = getAddressFromCubeID(recipientID);
    return (mesh.sendSingle(address, msg));
  }
}

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <painlessMesh.h> // Wireless library which forms mesh network https://github.com/gmag11/painlessMesh
#include <ArduinoJson.h>
#include "Communication.h"
#include "CBuff.h"
#include "Defines.h"

#define   BLINK_PERIOD    3000000 // microseconds until cycle repeat
#define   BLINK_DURATION  100000  // microseconds LED is on for
#define   MESH_SSID       "mblocks3d"
#define   MESH_PASSWORD   "mblocks3d"
#define   MESH_PORT       5555

// Create class variable mesh
painlessMesh mesh;
uint32_t previousMessageID; // This stores the 

bool calc_delay = false;
SimpleList<uint32_t> nodes;

// JSON circular buffer to hold incomming messages
String jsonBufferSpace[40];
CircularBuffer<String, true> jsonCircularBuffer(ARRAY_SIZEOF(jsonBufferSpace), jsonBufferSpace);

// Create user defined data objects to store incoming messages
/*
  struct faceState
  {
  char faceID;
  char connectedCube;
  char connectedFace;
  char connectedAngle;
  };

  struct cubeState
  {
  char bottomFace;
  char plane;
  faceState faceA;
  faceState faceB;
  faceState faceC;
  faceState faceD;
  faceState faceE;
  faceState faceF;
  };

*/

// Defined 
//struct outboxEntry {
//  String mContents;
//  uint32_t mID;
//  uint32_t mDeadline;
//  unsigned char backoff;
//};

//struct inboxEntry {
//  String mContents;
//  uint32_t mID;
//};

//outboxEntry 
//outboxMemoryReservation[NUM_CUBES * NUM_MESSAGES_TO_BUFFER_OUTBOX];



// inboxEntry inboxMemoryReservation[NUM_CUBES * WINDOW_SIZE];
// CircularBuffer<inboxEntry> inbox(NUM_CUBES * WINDOW_SIZE, &inboxMemoryReservation[0]);
/**
   In the outbox, we need to keep track of each message that we are transmitting
*/

/**
   These variables hold messages that need to be sent, and recieved messages that need to be
   processed
*/

// this is where the cube data object will be built in the future
//
//

uint32_t advanceLfsr() // this call returns a message ID. these are not sequential.
{
  static uint32_t lfsr = 0xfefefe;

  uint32_t lsb = lfsr & 0x01u;
  lfsr >>= 1;
  if (lsb)
  {
    lfsr ^= (1u << 31) | (1u << 21) | (1u << 1) | (1u << 0);
  }
  Serial.print("LFSR = ");
  Serial.println(lfsr);
  return lfsr;
}

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

/**
   This function looks through newly-recieved messages, and prunes waiting
   messages in the outbox, sending them if appropriate.

   It also updates the state of the cube data model
*/

void initializeBoxes()
{

}

void updateInboxAndOutbox()
/*
 * This function does the following
 * 
 * 1. Clears out the inbox... Basically checks the circular buffer for accrued message
 */
{
  
}

//void updateBoxes(CircularBuffer<inboxEntry> &inbox, CircularBuffer<outboxEntry> (&outbox)[NUM_CUBES])
//{
//  // Clear out the inbox
//  while(!inbox.empty())
//  {
//    // Retrieve the most recent thing in the inbox
//    inboxEntry inboxItem = inbox.pop();
//
//    // See if we need to mark any outbox messages as "acked"
//    bool foundflag = false;
//    for(int cub = 0; cub < (NUM_CUBES); cub++)
//    {
//      if(outbox[cub].access(0).mID == inboxItem.mID) {
//        foundflag = true;
//        outbox[cub].pop();
//      }
//    }
//    if(!foundflag) {
//      Serial.println("Spurious ACK for message ID: " + inboxItem.mID);
//    }
//
//    // updateCubeModelWithAck() TODO incorporate new inboxItem into the state of the cubes
//  }
//
//  // Decide which messages to (re)send.
//  for (int cub = 0; cub < (NUM_CUBES); cub++) 
//    if (!outbox[cub].empty()){ // for the outbox queues with messages in them...
//      if (millis()>outbox[cub].access(0).mDeadline) // if the time has come to resend the message...
//      {
//        sendMessage(cub, outbox[cub].access(0).mContents); // send it...
//        outbox[cub].access(0).mDeadline = millis() + random((1UL << outbox[cub].access(0).backoff) * 
//        AVERAGE_FIRST_DELAY_MS);
//        // set the next deadline using exponential backoff...
//        outbox[cub].access(0).backoff++; // and increment the counter to reflect the number of tries.
//        }
//    }
//  }
//}

void receivedCallback(uint32_t from, String & stringMsg)
{
  if(MAGIC_DEBUG)
  {
    Serial.print("Message Received from: ");
    Serial.println(from);
    Serial.print("Message Contents: ");
    Serial.println(stringMsg);
  }

  // Check and see if this message is a dupe by "manually" extracting the message
  // id field.
  // check to see if it's new, if so, do something with it
  
  StaticJsonBuffer<512> jsonMsgBuffer;
  JsonObject& jsonMsg = jsonMsgBuffer.parseObject(stringMsg);
  String mIDstring = jsonMsg["mID"];
  int mID = mIDstring.toInt();
  //if (mID != prevMID)
  if (mID == previousMessageID)
  {
    Serial.println("MESSAGE ACKNOWLEDGED!!!");
  }
  previousMessageID = mID;
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Connection Event\n");
  //  Serial.printf("Changed connections %s\n", mesh.subConnectionJson().c_str());
  //  nodes = mesh.getNodeList();
  //  Serial.printf("Num nodes: %d\n", nodes.size());
  //  Serial.printf("Connection list:");
  //  SimpleList<uint32_t>::iterator node = nodes.begin();
  //  while (node != nodes.end()) {
  //    Serial.printf(" %u", *node);
  //    node++;
  //  }
  //  calc_delay = true;
}

void nodeTimeAdjustedCallback(int32_t offset) {
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void delayReceivedCallback(uint32_t from, int32_t delay) {
  //Serial.printf("Delay to node %u is %d us\n", from, delay);
}

String newBlinkCommand()
{
  //====== Generate a blink message =========
  StaticJsonBuffer<256> jsonBuffer; //Space Allocated to construct json instance
  JsonObject& jsonMsg = jsonBuffer.createObject(); // & is "c++ reference"
  //jsonMsg is our output, but in JSON form
  jsonMsg["mID"] = 	advanceLfsr();
  jsonMsg["type"] = "cmd";
  jsonMsg["sID"] = 	SERVER_ID;
  jsonMsg["cmd"] = 	"blink";
  String strMsg; // generate empty string
  //strMsg is our output in String form
  jsonMsg.printTo(strMsg); // print to JSON readable string...
  return strMsg;
}

String newStatusCommand()
{
  //====== Generate a status request =========
  StaticJsonBuffer<256> jsonBuffer; //Space Allocated to construct json instance
  JsonObject& jsonMsg = jsonBuffer.createObject(); // & is "c++ reference"
  //jsonMsg is our output, but in JSON form
  jsonMsg["mID"] = 	advanceLfsr();
  jsonMsg["type"] = "cmd";
  jsonMsg["sID"] = 	SERVER_ID;
  jsonMsg["cmd"] = 	"statReq";
  String strMsg; // generate empty string
  //strMsg is our output in String form
  jsonMsg.printTo(strMsg); // print to JSON readable string...
  return strMsg;
}

String newForwardCommand()
{
  //====== Generate a Move command =========
  StaticJsonBuffer<256> jsonBuffer; //Space Allocated to construct json instance
  JsonObject& jsonMsg = jsonBuffer.createObject(); // & is "c++ reference"
  //jsonMsg is our output, but in JSON form
  jsonMsg["mID"] = 	advanceLfsr();
  jsonMsg["type"] = "cmd";
  jsonMsg["sID"] = 	SERVER_ID;
  jsonMsg["cmd"] = 	"F";
  String strMsg; // generate empty string
  //strMsg is our output in String form
  jsonMsg.printTo(strMsg); // print to JSON readable string...
  return strMsg;
}

String newReverseCommand()
{
  //====== Generate a Move command =========
  StaticJsonBuffer<256> jsonBuffer; //Space Allocated to construct json instance
  JsonObject& jsonMsg = jsonBuffer.createObject(); // & is "c++ reference"
  //jsonMsg is our output, but in JSON form
  jsonMsg["mID"] =  advanceLfsr();
  jsonMsg["type"] = "cmd";
  jsonMsg["sID"] =  SERVER_ID;
  jsonMsg["cmd"] =  "F";
  String strMsg; // generate empty string
  //strMsg is our output in String form
  jsonMsg.printTo(strMsg); // print to JSON readable string...
  return strMsg;
}
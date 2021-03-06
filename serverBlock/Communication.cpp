#include "Communication.h"

/* -----------------------------------------------------------------------------------------------------------
 * ------------------------------- More Defines and Initial Varaibles ----------------------------------------
 * -----------------------------------------------------------------------------------------------------------
 */

// Parameters for the the WIFI mesh network
#define   BLINK_PERIOD    3000000 // microseconds until cycle repeat
#define   BLINK_DURATION  100000  // microseconds LED is on for
#define   MESH_SSID       "mblocks3d"
#define   MESH_PASSWORD   "mblocks3d"
#define   MESH_PORT       5555

#define   MAXIMUM_MESSAGE_ATTEMPTS 7

/*
 *  Actual class which implements the painless message class
 */
painlessMesh mesh;

/* INBOX
 *  These variables hold messages that need to be sent, and recieved messages that need to be
 *  processed //inbox stores incomming messages into an array of mailboxes
 *  This creates an array of inboxes of length INBOX_SIZE - this is mangges as a circular bugger
 */
inboxEntry inbox[INBOX_SIZE]; 
            
int inboxHead = 0; // Current empty position to add more messages
int inboxTail = 0; // The oldest message in the buffer... first one to process

/** OUTBOX
   In the outbox, we need to keep track of each message that we are transmitting
   We represent the outbox as a 2D array of outboxEntry Objects.
   To access the current message for cube with ID # 4, we would access it with:
      outbox[5][outboxTail[5]...
*/
outboxEntry outbox[NUM_CUBES][OUTBOX_SIZE] ; // 2D Array of OutboxEntry Instances
/*
 * The head and tail are both 1 dimensional arrays, one for each of the Cube Numbers...
 * Each one stores the current state of the outbox heads in reference to its status 
 * as a circular buffer
 */
int outboxHead[NUM_CUBES] = {};
int outboxTail[NUM_CUBES] = {};

/*
   This function checks the inbox to see if it is an ack for a message currently in the outbox.
   If so, it clears both the inbox and the outbox.
   If not, or if no message is in the inbox, it checks to see if the outbox message needs to be resent 
   (and does so if needed).
*/

void updateBoxes()
{
  while (!inboxIsEmpty()) // This means there is something in the inbox
  {
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
       //updateStateModel(inbox[inboxTail].senderID); // process ack for the cube 
       outbox[inbox[inboxTail].senderID][outboxTail[inbox[inboxTail].senderID]].mID = 0; // clear outbox entry
       advanceOutboxTail(inbox[inboxTail].senderID); // move on to next outbox slot...
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
    /*
     * Set the message ID of the inbox message pointed to by the TAIL
     * to be equal to 0. This essentially clears this message from existance
     */
    inbox[inboxTail].mID = 0;
    /*
     * We then advance the inbvox tail, since we processed this specific message
     */
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
       *
       *  We also check to see if the message has been sent more times than
       *  MAXIMUM_MESSAGE_ATTEMPTS, which if this is true, we delete message 
       *  from the que...
       */
      {
        //generate message and actually send it...
        sendMessage(cubeID, generateMessageText(outbox[cubeID][outboxTail[cubeID]].cmd, outbox[cubeID][outboxTail[cubeID]].mID)); 

        /*
         * Updated the message deadline
         */
        outbox[cubeID][outboxTail[cubeID]].mDeadline = 
          millis() + random((1UL << outbox[cubeID][outboxTail[cubeID]].backoff) * AVERAGE_FIRST_DELAY_MS * 2);
                  
        outbox[cubeID][outboxTail[cubeID]].backoff++;
        
        /*
         * This means that we have tried to send the message a lot of times... remove from que
         * now
         */
         
        if(outbox[cubeID][outboxTail[cubeID]].backoff > MAXIMUM_MESSAGE_ATTEMPTS)
        {
          /*
           *  This effectivly deletes the current message.
           */
           
          outbox[cubeID][outboxTail[cubeID]].mID = 0; // clear outbox entry
          advanceOutboxTail(cubeID); // move on to next outbox slot...
          database[cubeID][failedMessageCount]++;
          
          Serial.println("MESSAGE REMOVED DUE TO FAILURE TO DELIVER");
          
          if(database[cubeID][failedMessageCount] > 2)
          {
            if(MESSAGE_DEBUG)
            {
              Serial.print("Cube: ");
              Serial.print(cubeID);
              Serial.print(" has failed to respond to several messages...");
            }
            
            database[cubeID][ACTIVE] = 0;
            database[cubeID][failedMessageCount] = 0;
          }
        }
      }
    }
  }
}

void receivedCallback(uint32_t from, String & stringMsg)
/*
 * This function gets called by the mesh library whenever we receive a wifi Message
 * 
 * The goal is to check to see if it is a new message, and if so, we add it to a 
 * list of messages to be processed by inboxbuffer, and also update the database
 */
{
  if(MESSAGE_DEBUG)
  {
    Serial.print("M From: ");
    Serial.print(from);
    Serial.print(" | ");
    Serial.println(stringMsg);
  }

  /*
   * The following set of lines creates space in a temporary buffer
   * then parses the JSON message, and teases out the two variables
   * senderCubeID: the wifi ID of the cube which sent the message
   * recievedMID: The message ID of the message
   */
  StaticJsonBuffer<256> jsonMsgBuffer; 
  JsonObject& jsonMsg = jsonMsgBuffer.parseObject(stringMsg); 
  int senderCubeID = jsonMsg["sID"];
  uint32_t recievedMID = jsonMsg["mID"];

  /*
   * If the message ID is 42, then it is just a very quick message
   * telling us that the cube is active, we are not going to 
   */
  if(recievedMID == SPECIAL_MESSAGE_ID)
  {
    /*
     * Mark this particular cube as active in the database
     */
    database[senderCubeID][ACTIVE] = 1;
  }
  else if(recievedMID == CONFIRM_MESSAGE_ID)
  {
    CONFIRMED = true;
  }
  /*
   * If there is space in the inbox
   */
  if (inbox[inboxHead].mID == 0)
  {
    int tempBottomFace = jsonMsg["bFace"];
    /*
     * Update the parameters that go into the inbox entry if the
     * message is NOT the special message... SPECIAL_MESSAGE_ID
     */
    if(recievedMID != SPECIAL_MESSAGE_ID)
    {
      inbox[inboxHead].mID = recievedMID;
      inbox[inboxHead].senderID = senderCubeID;
      advanceInboxHead();
    }

    
    /*
     * Update the parameters in the database
     */
    database[senderCubeID][bottom_Face] = tempBottomFace;

    if(jsonMsg.containsKey("f0"))
      database[senderCubeID][face_0] = jsonMsg["f0"];
    else
      database[senderCubeID][face_0] = -1;
      
    if(jsonMsg.containsKey("f1"))
      database[senderCubeID][face_1] = jsonMsg["f1"];
    else
      database[senderCubeID][face_1] = -1;
      
    if(jsonMsg.containsKey("f2"))
      database[senderCubeID][face_2] = jsonMsg["f2"];
    else
      database[senderCubeID][face_2] = -1;
      
    if(jsonMsg.containsKey("f3"))
      database[senderCubeID][face_3] = jsonMsg["f3"];
    else
      database[senderCubeID][face_3] = -1;
      
    if(jsonMsg.containsKey("f4"))
      database[senderCubeID][face_4] = jsonMsg["f4"];
    else
      database[senderCubeID][face_4] = -1;
      
    if(jsonMsg.containsKey("f5"))
      database[senderCubeID][face_5] = jsonMsg["f5"];
    else
      database[senderCubeID][face_5] = -1;
  }
  else
  {
    Serial.println(" NO SPACE IN THE INBOX BUFFER!!!!!, HELP!!!");
  }
}

void newConnectionCallback(uint32_t nodeId)
/*
 * This function gets called if we form a new connection
 */
{
  /*
   * Convert the wifi ID to cubeID space...
   */
  int tempCubeID = getCubeIDFromAddress(nodeId);
  
  Serial.print("New Cube: ");
  Serial.println(tempCubeID);
  /*
   * Make 100% sure we are protecting the indexing of the array...
   */
  if(tempCubeID > -1 && tempCubeID < NUM_CUBES)
  {
    database[getCubeIDFromAddress(tempCubeID)][ACTIVE] = 1;
  }
}

void changedConnectionCallback()
/*
 * We don't use this yet...
 */
{
  //Serial.printf("Connection Event\n");
}

void nodeTimeAdjustedCallback(int32_t offset) 
/*
 * We don't use this yet either...
 */
{

}

void delayReceivedCallback(uint32_t from, int32_t delay) 
/*
 * We also don't use this yet either...
 */
{

}

String generateMessageText(String cmd, uint32_t mID)
/*
 * This function takes a string "cmd" which is the message to be sent,
 * and it takes the message ID.
 * 
 * It then formats them in the JSON format, and returns the string containing
 * all of the formatted message, including the following components
 * 
 * message ID       | "mID"   |  
 * sender ID        | "sID"   |
 * command to send  | "cmd"   |
 */
{
  if (mID == 0) {
    mID = advanceLfsr();
  }
  /*
   * Allocate Space to construct a json instance
   */
  StaticJsonBuffer<512> jsonBuffer;
  JsonObject& jsonMsg = jsonBuffer.createObject();
  /*
   * jsonMsg is our output
   * We are the server so our ID goes in the sID field
   */
  jsonMsg["mID"] =  mID;
  jsonMsg["sID"] =  SERVER_ID;
  jsonMsg["cmd"] =  cmd;

  /*
   * Generate en empty string, then print the 
   * JSON message to this string, and return it
   */
  String strMsg;
  jsonMsg.printTo(strMsg);
  return strMsg;
}

/*
 *  Special list of messages
 */
void pushBlinkMessage(int cubeID)
{
  pushMessage(cubeID, "b");
  Serial.print("Pushing Blink Message to: ");
  Serial.println(cubeID);
}

void pushForwardMessage(int cubeID)
{
  pushMessage(cubeID, "f");
  Serial.print("Pushing Forward Message to: ");
  Serial.println(cubeID);
}

void pushReverseMessage(int cubeID)
{
  pushMessage(cubeID, "r");
  Serial.print("Pushing Reverse Message to: ");
  Serial.println(cubeID);
}

void pushSleepMessage(int cubeID)
{
  pushMessage(cubeID, "s");
  Serial.print("Pushing Sleep Message to: ");
  Serial.println(cubeID);
}

void pushStatusMessage(int cubeID)
{
  pushMessage(cubeID, "q");
  Serial.print("Pushing Status Message to: ");
  Serial.println(cubeID);
}

void pushMessage(int cubeID, String command)
{
  /*
   * If we aren't given a valid address, we return
   */
  if (getAddressFromCubeID(cubeID) == 0)
    return;
  /*
   * Or if the outbox is full, we also return without doing anything
   */
  if (outboxIsFull(cubeID))
    return;
  /*
   * This section updates all of the parameters of the outbox entry
   * for this specific cube
   */
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
  if (outboxHead[cubeID] == OUTBOX_SIZE) 
    outboxHead[cubeID] = 0;
}

void advanceOutboxTail(int cubeID)
{
  outboxTail[cubeID]++;
  if (outboxTail[cubeID] == OUTBOX_SIZE)
    outboxTail[cubeID] = 0;
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
  if (inboxTail == NUM_CUBES)
    inboxTail = 0;
}

bool inboxIsFull()
{
  return inbox[inboxHead].mID != 0;
}

bool inboxIsEmpty()
{
  return (inbox[inboxTail].mID == 0);
}

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
/*
 * This function starts the WIFI mesh
 */
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
/*
 * This function actually sends the message, if the recipientID is -1, then it sends it as a broadcast
 * to all cubes, else it sends it to the specific cube that it is intended for
 */
{
  if(MESSAGE_DEBUG)
  {
    Serial.print("To:  ");
    Serial.print(recipientID);
    Serial.print(" | ");
    Serial.println(msg);
  }
  
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

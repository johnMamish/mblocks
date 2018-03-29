#include "Behavior.h"
#include "Initialization.h"     // Includes .h files for each of the "tabs" in arduino
#include "Cube.h"               // Includes .h files for each of the "tabs" in arduino
#include "Face.h"               // Includes .h files for each of the "tabs" in arduino
#include "CBuff.h"              // Includes .h files for each of the "tabs" in arduino
#include "Communication.h"      // Includes wifi
#include "Defines.h"

//=============================================================================================
//=============================WIFI Checking  CHECKING=========================================
//=============================================================================================
Behavior checkForWifiCommands(Cube* c, Behavior currentBehavior)
{
  int messagesRead = 5;
  Behavior resultBehavior = currentBehavior;
  while(!jsonCircularBuffer.empty() && messagesRead > 0) // while there are still messages, and we haven't tried 5 times
  {
    StaticJsonBuffer<700> jb; // Create a buffer to store our Jason Objects...
    JsonObject& root = jb.parseObject(jsonCircularBuffer.pop());
    if((root["targetID"] == getCubeIDFromEsp(ESP.getChipId())) || // If message matches your ID
       (root["targetID"] == -1))                                  // or if message is brodcast
    {
      /* At this point, we have determined that the message is for us... so now we try to decode the contents
       * this extracts the contents of "cmd" and puts it into a local variable
       */
      String receivedCMD = root["cmd"]; 

      /*  checks to see if the recieved message matches a behavior... 
       *  If it doesn't we default to the currentBehavior
       */     
      resultBehavior = cmdToBehaviors(receivedCMD, currentBehavior); 
       
      /*  If the command is "update" then we are going to generate a message containing relevant information                                                 
       *   and then send it over the wifi channel
       */
      if(receivedCMD == "update")
      {
        
      }
      
      /*  If the command is "blink" then we will really quickly blink the face LED's
       *  This can be used to visually verify which cubes are actually connectd to the wifi mesh
       *  and are working properly
       */
      else if(receivedCMD == "blink")
        blinkFaceLeds(c, 50);

      /*  A simple way to turn the cube to various colors works by receiving a message
       *  that is just an integer, it will change to a color depending on the integer
       */
      else if(isDigit(receivedCMD[0]))
      {
        wifiLightChange(c, receivedCMD.toInt(), true); // true turns off the lights afterwards
      }

      /* cubeID's over 40 means it is attached by a cable... not a real cube // so we print
       */
       
      if(c->cubeID > 40) 
      {
        String targetID = root["targetID"];
        String receivedCMD = root["cmd"];
        String senderID = root["senderID"];
        String messageString = "Message: From: " + senderID + " to: " + targetID + " Command is: " + receivedCMD;// + " Command is: ";
        Serial.println(messageString);
      }
    }
    messagesRead--;
  }
  return(resultBehavior);
}

Behavior relayBehavior(Cube* c, Behavior behaviorToRelay, int cubeToRelayTo, int timesToRelay)
{
  //======Temporarily Generated a Broadcast message =========
  StaticJsonBuffer<264> jsonBuffer; //Space Allocated to store json instance
  JsonObject& root = jsonBuffer.createObject(); // & is "c++ reference"
  //^class type||^ Root         ^class method                   
  root["type"] = "cmd";
  root["cubeID"] = cubeToRelayTo;
  root["cmd"] = behaviorsToCmd(behaviorToRelay);
  //^ "key"   |  ^ "Value"
  String str; // generate empty string
  root.printTo(str); // print to JSON readable string...
  //======== End Generating of Broadcast message ==========
  
  for(int i = 0; i < timesToRelay; i++);
  {
    mesh.sendBroadcast(str);
    wifiDelay(400);
  }
  return(behaviorToRelay);
}

//================================================================
//==========================DEMO==============================
//================================================================
Behavior demo(Cube* c)
{
  Serial.println("hey!");
  long loopCounter = 0;
  Behavior nextBehavior = DEMO;
  while(nextBehavior == DEMO) // loop until something changes the next behavior
  {
    nextBehavior = basicUpkeep_DEMO_ONLY(c, nextBehavior);
    nextBehavior = checkForWifiCommands(c, nextBehavior);
    wifiDelay(100);
  }
  return nextBehavior;
}

Behavior basicUpkeep_DEMO_ONLY(Cube* c, Behavior inputBehavior)
/*
 * Then it checks the wifi BUFFER and checks the magnetic tags
 * for actionable configurations
 */
{
  // update sensors, numberOfNeighbors, and check wifi commands...
  c->update(); // actually read all of the sensors
  //int numberOfNeighborz = checkForMagneticTagsDEMO(c);
  if(c->returnTopFace(0) != c->returnTopFace(1)) // checking to see if there is a change in its own orientation
  {
    Serial.print("trying to send wifi Message");
    wifiTargetFace(c, c->returnTopFace(0), -1);
  }
  return(inputBehavior);
}

void wifiTargetFace(Cube* c, int faceToSend, int recipientCube)
{
  //======Temporarily Generated a Broadcast message =========
  StaticJsonBuffer<512> jsonBuffer; //Space Allocated to store json instance
  JsonObject& root = jsonBuffer.createObject(); // & is "c++ reference"
             //^class type||^ Root   ^class method                   
  root["type"] = "cmd";
  root["cubeID"] = recipientCube;
  root["cmd"] = String(faceToSend);
  //^ "key"   |  ^ "Value"
  String str; // generate empty string
  root.printTo(str); // print to JSON readable string...
  //======== End Generating of Broadcast message ==========
  mesh.sendBroadcast(str);
}
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////


Behavior basicUpkeep(Cube* c, Behavior inputBehavior)
/*
 * This function does basic state machine switching
 * It (1) Updates the sensors, including magnetic sensors
 * light sensors, and IMU's
 * 
 * Then it checks the wifi BUFFER and checks the magnetic tags
 * for actionable configurations
 */
{
  if(MAGIC_DEBUG) Serial.println("Beginning basicUpKeep");
  // update sensors, numberOfNeighbors, and check wifi commands...
  c->update(); // actually read all of the sensors
  Behavior newBehavior = checkForWifiCommands(c, inputBehavior);
  int numberOfNeighborz = checkForMagneticTagsStandard(c);

  // Check for Light Digits
  //int lightDigit = processLightDigits(c);

//************************************
//NEIGHBORS == __ 0 __
//************************************
  if(numberOfNeighborz == 0)
  {
    newBehavior = SOLO_LIGHT_TRACK;
  }
//************************************
//NEIGHBORS == __ 1 __
//************************************
  else if(numberOfNeighborz == 1)
  {
    if(MAGIC_DEBUG){Serial.println("***NEIGHBORS == 1");}
    int neighborFace = c->whichFaceHasNeighbor();  
    if(c->faces[neighborFace].returnNeighborType(0) == TAGTYPE_REGULAR_CUBE)
    {
      if(magicTheLight == true)
        newBehavior = CLIMB;
      else if(c->numberOfNeighbors(1, 0) == 1) // if the last one also shows that there is a neighbor... 
        newBehavior = DUO_LIGHT_TRACK;
    }
    else if(c->faces[neighborFace].returnNeighborType(0) == TAGTYPE_PASSIVE_CUBE)
    {
      magicTheLight = true;
      if(neighborFace == c->returnBottomFace())
      {
        newBehavior = ATTRACTIVE;
      }
      else if((c->returnForwardFace() == neighborFace) ||
              (c->returnReverseFace() == neighborFace)) 
        newBehavior = CLIMB;
      else
        newBehavior = ATTRACTIVE;
    }
  }
//************************************
//NEIGHBORS == __ 2 __
//************************************
  else if(numberOfNeighborz == 2)
  {
    int first_neighborFace = c->whichFaceHasNeighbor(0); 
    int second_neighborFace = c->whichFaceHasNeighbor(1); 
    if(MAGIC_DEBUG){Serial.println("***NEIGHBORS == 2");}
    
    if(magicTheLight == true)
    {
      if(MAGIC_DEBUG){Serial.println(" I SEE THE LIGHT - Don't do the next part...");}
      newBehavior = ATTRACTIVE;
    }
    else if(c->areFacesOpposite(first_neighborFace, second_neighborFace) == true)
    {
      c->blockingBlink(&purple, 50, 20);
      newBehavior = MULTI_LIGHT_TRACK;
    }
    else if((c->numberOfNeighbors(5, 0) == 2) &&
            (c->numberOfNeighbors(0, 0) == 2)) // if we have been connected for a while to two cubes, and haven't seen the light... we BOUNCE... disconnect
    {
      if(MAGIC_DEBUG){Serial.println(" SO IT HAS COME TO THIS.... PEACE OUT BROTHERS...");}
      c->blockingBlink(&red, 70, 50);
      c->update();
      checkForMagneticTagsStandard(c); // give me a change to turn it off before it explodes...
      delay(1000);
      c->moveOnLattice(&explode_F);
      newBehavior = SOLO_LIGHT_TRACK;
    }
  }
  
//************************************
//NEIGHBORS == __ 2+ __
//************************************

  else if(numberOfNeighborz > 2)
  {
    if(MAGIC_DEBUG){Serial.println("***NEIGHBORS == __ 2+ __");}
    newBehavior = ATTRACTIVE;
  }

  return(newBehavior);
}

Behavior sleep(Cube* c)
{
  c->blockingBlink(&red,10);
  for(int i = 0; i < 10; i++)
  {
    Serial.println("sleep");
    delay(300);
  }
}

Behavior followArrows(Cube* c) // purple
/*
 * This behavior is intended to only move ontop of a structure...
 * It will exit if it isn't connected one the bottom face.
 */
{
  if(MAGIC_DEBUG){Serial.println("***Beginning FOLLOWARROWS***");}
  
  Behavior nextBehavior = FOLLOW_ARROWS; // default is to keep doing what we are doing.
  int loopCounter = 0;
  nextBehavior = basicUpkeep(c, nextBehavior);  // check wifi and Magnetic Sensors
  c->superSpecialBlink(&purple, 100);
  c->superSpecialBlink(&white, 200);
  while((nextBehavior == FOLLOW_ARROWS) && (millis() < c->shutDownTime))
  {
    for(int i = 0; i < FACES; i ++)
    {
      if((c->faces[i].returnNeighborAngle(0) > -1) && // If the same face shows valid angles
         (c->faces[i].returnNeighborAngle(1) > -1))// twice in a row...
      {
        int otherFace = faceArrowPointsTo(i, c->faces[i].returnNeighborAngle(0)); // find out which direction the arrow points
        if(c->goToPlaneIncludingFaces(i, otherFace) == true) // then go to plane parallel to these two faces...
        {
          int CW_or_CCW = faceClockiness(otherFace, i);
          if(CW_or_CCW == 1)
          {
            c->moveOnLattice(&traverse_F);
            delay(500);
          }
          else if(CW_or_CCW == -1)
          {
            c->moveOnLattice(&traverse_R);
            delay(500);
          }
        }
      }
    }
    loopCounter++;
    nextBehavior = basicUpkeep(c, nextBehavior);
  }
  return(nextBehavior);
}

/*
 * Pre Solo Light is intented to run before we go into Solo Light Track
 * The reason why this needs to exist is that if the cubes are programmed to make this the default mode 
 * (In the case that they restart their processors) , then
 * we don't want the robots to start moving immediatly, so basically this starts blinking, and it gives the person
 * who turned on the robot a chance to stop it.
 */
Behavior Pre_Solo_Light(Cube* c)
{
  if(MAGIC_DEBUG) {Serial.println("***ABOUT TO BEGIN SOLO_LIGHT***");}  
  long loopCounter = 0;
  Behavior nextBehavior = PRE_SOLO_LIGHT;
  while(((nextBehavior == PRE_SOLO_LIGHT) || (nextBehavior == SOLO_LIGHT_TRACK)) && loopCounter < 3) // loop until something changes the next behavior
  {
    nextBehavior = basicUpkeep(c, nextBehavior);
    wifiDelay(100);
    c->blockingBlink(&yellow, 10, 75);
    loopCounter++;
  }
  return SOLO_LIGHT_TRACK;
}

/* Solo Light Seek is designed to have the module follow the brightest light source that it can find.
 *  The secondary goal is to connect to other modules.
 */
Behavior soloSeekLight(Cube* c) // green
{
  // General Starting things... initialize flags, etc...
  if(MAGIC_DEBUG) {Serial.println("***soloSeekLight***");}
  Behavior nextBehavior = SOLO_LIGHT_TRACK;
  int loopCounter = 0;
  bool iAmStuck  = false;
  nextBehavior = basicUpkeep(c, nextBehavior);  // check for neighbors, etc...
  
  while((nextBehavior == SOLO_LIGHT_TRACK) && // if we haven't changed state
        (c->numberOfNeighbors(0,0) == 0)  && // and if we still have ZERO neighbors
        (millis() < c->shutDownTime))        // and if we aren't feeling sleepy
  {
    // apply sorting function to generate list of brightest faces...
    // the "true" arguement excludes the top face...
    int brightestFace = c->returnXthBrightestFace(0, true);
    int nextBrightestFace = c->returnXthBrightestFace(1, true);
    int thirdBrightestFace = c->returnXthBrightestFace(2, true);
    //
    c->lightFace(brightestFace, &green);
    wifiDelay(300);
    c->lightFace(nextBrightestFace, &teal);
    wifiDelay(300);
    c->lightFace(thirdBrightestFace, &blue);
    wifiDelay(300);
    c->lightCube(&off);
    wifiDelay(100);
    
    // Figure out which way we should try to move
    bool direct = false; // false = reverse...
    if(brightestFace == c->returnForwardFace())
      direct = true;
    else if(brightestFace == c->returnReverseFace())
      direct = false;
      // Ok Nothing is directly Aligned... Checking Next Brightest
    else if(nextBrightestFace == c->returnForwardFace())
      direct = true;
    else if(nextBrightestFace == c->returnReverseFace())
      direct = false;
          // Ok Checking 3rd brightest face...
    else if(thirdBrightestFace == c->returnForwardFace())
      direct = true;
    else if(thirdBrightestFace == c->returnReverseFace())
      direct = false;
    
    /************* Begin if else if chain **************************
     *  The following if/else if chain represents the choices we can take
     *  give that we have recently updated our information, including light sensors,
     *  measurement if we are stuck or not (am I stuck FLAG), etc...
     * if(iAmStuck)
     * This means we have previosuly tried to move and we think we are stuck
     * but we have already run basic upkeep, so we will try to move the plane 
     * parallel with the ground to align with the lattice
     */
    if(c->currentPlaneBuffer.access(0) == PLANENONE)
    {
      c->superSpecialBlink(&red, 160);
      magicFace = c->returnTopFace(); 
      magicVariable = 1;
    }
    else if(iAmStuck)
    {
      // if we succeed in moving... we break out of this loop
      if(c->roll(direct, 8500) == true) // try to roll and succeed...
      {
        iAmStuck = false; 
        //iMightBeStuck = false;
      }
      else
      {
        // If we fail to move, we try to move plane parallel to ground
        // int attempts = 3;
        int topFace = c->returnTopFace();
        if((topFace > -1) && (topFace < FACES)) // if this is true we are on the ground... so we should try to change planes
        {
          magicFace = topFace; 
          magicVariable = 1; // this triggers a plane change later in the program...
        }
        else // we are not on the ground... So we jump a little bit...
        {
          c->moveOnLattice(&traverse_F);
        }
        
        if(c->returnForwardFace() == -1) // this is a proxy for plane being parallel to ground... or an error
        {
          if(c->numberOfNeighborsCheckNow() == 0)
            c->moveOnLattice(&traverse_F); 

            
          c->superSpecialBlink(&yellow, 50);
          wifiDelay(1500);
          
          if(c->numberOfNeighborsCheckNow() == 0)
            c->moveOnLattice(&traverse_R);
          wifiDelay(1500);
        }
      } 
    }
    //****************************
    else if(c->returnForwardFace() == -1)
    {
      if(c->moveBLDCACCEL(1, GlobalMaxAccel, 1700) == false)
          iAmStuck = true;
      wifiDelay(100);
    }
    //**************************** this is regular light tracking...
    else
    {
      
      if(c->roll(direct, 9500) == false)
      {
        iAmStuck = true;
      }
    }
 
//************** End if else if chain **************************
    loopCounter++;
    nextBehavior = basicUpkeep(c, nextBehavior);  // check for neighbors, etc...
  }
  return(nextBehavior);
}

/*
 * This is triggered when the cube has one neighbor... and when the neighbor
 * is on one of the faces other than the top and bottom faces...
 * goal is to move plane to be parallel and then to climb up ontop
 */
Behavior climb(Cube* c)
{
  if(MAGIC_DEBUG) {Serial.println("***Beginning CLIMB***");}
  int connectedFace = -1;
  long loopCounter = 0;
  Behavior nextBehavior = CLIMB;
  nextBehavior = basicUpkeep(c, nextBehavior);
  
  while((nextBehavior == CLIMB) && 
        (c->numberOfNeighbors(0, 0) == 1) && // loop until something changes the next behavior
        (millis() < c->shutDownTime))        // and if we aren't feeling sleepy
  {
    int connectedFace = c->whichFaceHasNeighbor(0);
    c->lightFace(connectedFace, &yellow);
    delay(300);
    c->lightFace(c->returnTopFace(),&red);
    delay(300);
    c->lightCube(&off);
    
    if((connectedFace > -1) && (connectedFace != c->returnTopFace()) &&
       (connectedFace != c->returnBottomFace()))
    {      
      if(c->isPlaneInPlaneOfFaces(connectedFace, c->returnTopFace()) == true)
      {
        delay(50);
        int CW_or_CCW = faceClockiness(connectedFace, c->returnBottomFace());
        if(CW_or_CCW == 1)
          c->moveOnLattice(&cornerClimb_F);
        if(CW_or_CCW == -1)
          c->moveOnLattice(&cornerClimb_R);
      }
      else
      {
        c->goToPlaneIncludingFaces(connectedFace, c->returnTopFace());
        delay(1000);
      }
    }      
    //////////////////////////////// loop upkeep...
    c->superSpecialBlink(&yellow, 50);
    c->superSpecialBlink(&white, 100);
    nextBehavior = basicUpkeep(c, nextBehavior);
    loopCounter++;
  }
  return nextBehavior;
}

//================================================================
//==========================CHILLING==============================
//================================================================

Behavior chilling(Cube* c)
{
  if(MAGIC_DEBUG) {Serial.println("***CHILLING***");}  
  Behavior nextBehavior = CHILLING;
  while(nextBehavior == CHILLING) // loop until something changes the next behavior
  {
    nextBehavior = basicUpkeep(c, nextBehavior);
    wifiDelay(400);
  }
  return nextBehavior;
}

//================================================================
//==========================ACTRACTIVE==============================
//================================================================
Behavior attractive(Cube* c)
/*
 * This behavior involves the cube just sitting around turning its lights on
 */
{
  Behavior nextBehavior = ATTRACTIVE;
  if(MAGIC_DEBUG) {Serial.println("***ATTRACTIVE***");}   
  while(nextBehavior == ATTRACTIVE)
  {
    nextBehavior = basicUpkeep(c, nextBehavior); 
    c->lightCube(&off);    
    for(int i = 0; i < 6; i++)
    {
      if((i == c->returnTopFace()) || (i == c->returnBottomFace())) // This ensures we only 
        delay(1);                                                   // turn on 4 faces in horizontal plane
        //break;
      else if(c->faces[i].returnNeighborType(0) == TAGTYPE_REGULAR_CUBE) // if we have a neighbor on that face....
        {
          c->faces[i].turnOnFaceLEDs(1,1,1,1); 
        }
      else if(c->faces[i].returnNeighborPresence(0) == true) // if we have a neighbor on that face....
        delay(1);
      else
        {
          c->faces[i].turnOnFaceLEDs(1,1,1,1); // turns on LEDs on any exposed face thats not top/bottom/connected
        }
    }
    wifiDelay(4000); // delay 10 seconds...
  }
  return(nextBehavior);
}

Behavior duoSeekLight(Cube* c)
/*
 * This tries to drive two robots together towards a light source
 */
{
 // General Starting things... initialize flags, etc...
  if(MAGIC_DEBUG) {Serial.println("***Beginning DuoSeekLight***");}
  Behavior nextBehavior = DUO_LIGHT_TRACK;
  int loopCounter = 0;
  bool correctPlane = false;
  int failedMoveCounter = 0;
  int connectedFace = c->whichFaceHasNeighbor();
  c->lightFace(connectedFace,&yellow);
  delay(200);
  c->lightFace(connectedFace,&red);
  delay(500);
  c->lightCube(&off);
 
  // perform basic upkeep... this involves updating sensors...
  nextBehavior = basicUpkeep(c, nextBehavior);

  if(c->returnForwardFace() == -1) // we try to nudge ourself into the correct orientation in the case that we are wrong...
  {
    for(int i = 0; i++; i < 2)
    {
      if(MAGIC_DEBUG){Serial.println("TRYING ALTERNATE");}
      c->update();
      int brightestFace = c->returnXthBrightestFace(0, true);
      int nextBrightestFace = c->returnXthBrightestFace(1, true);
      //
      c->lightFace(brightestFace, &white);
      delay(500);
      c->lightFace(nextBrightestFace, &purple);
      delay(500);
      c->lightCube(&off);
      // Figure out which way we should try to move
      //bool direct = false; // false = reverse...
      if(brightestFace == oppositeFace(connectedFace))
      {
        c->blockingBlink(&purple);
        if(faceClockiness(nextBrightestFace, connectedFace) == 1)
        {
        c->printString("bldcaccel r 2000 1000");
        delay(1200);
        c->printString("bldcstop b");
        }
        else if(faceClockiness(nextBrightestFace, connectedFace) == -1)
        {
        c->printString("bldcaccel f 2000 1000");
        delay(1200);
        c->printString("bldcstop b");
        }
      }
    }
  }
  // if basic upkeep decides to change behavior, we exit now...
  // otherwise we keep running in this loop until something 
  // changes the state
  while((nextBehavior == DUO_LIGHT_TRACK) && // if we haven't changed state
        (c->numberOfNeighbors(0,0) == 1)  && // and if we have ZERO neighbors
        (millis() < c->shutDownTime))        // and if we aren't feeling sleepy
  {
    if(correctPlane == false)
    {
      if(c->isPlaneParallel(connectedFace) == true)
      {
        correctPlane = true;
      }
      else
        {
          magicVariable = 1;
          magicFace = connectedFace; 
        }
  }
    // Regular Light Tracking...
    else
    {
      int brightestFace = c->returnXthBrightestFace(0, true);
      int nextBrightestFace = c->returnXthBrightestFace(1, true);
      //
      c->lightFace(brightestFace, &yellow);
      delay(400);
      c->lightFace(nextBrightestFace, &red);
      delay(400);
      c->lightCube(&off);
      delay(100);
      // Figure out which way we should try to move
      bool direct = false; // false = reverse...
      if(brightestFace == c->returnForwardFace())
        direct = true;
      else if(brightestFace == c->returnReverseFace())
        direct = false;
      // Ok Nothing is directly Aligned... Checking Next Brightest
      else if(nextBrightestFace == c->returnForwardFace())
        direct = true;
      else if(nextBrightestFace == c->returnReverseFace())
        direct = false;
      /*
       * Begin if/ else if statement tree...
       */
      if(failedMoveCounter > 2)
      {
        if((c->returnTopFace(0) > -1) && (c->returnTopFace(0) < FACES) && (c->returnForwardFace() != -1)) // if this is true we are on the ground... so we should try to change planes
        {
          magicFace = c->returnTopFace(); 
          magicVariable = 1; // this triggers a plane change later in the program...
        }  
        else // we are not on the ground... So we jump a little bit...
        {
          c->moveOnLattice(&traverse_F);
        }
        if(c->returnForwardFace() == -1) // this is a proxy for plane being parallel to ground... or an error
        {
          c->moveOnLattice(&cornerClimb_F);
          delay(1500);
        }
      }    
      else if(direct == true)
      {
        if(c->moveOnLattice(&rollDouble_F) == true)
        {
          c->superSpecialBlink(&teal, 40);
          failedMoveCounter = 0;
        }
        else
          failedMoveCounter++;
      }
      else if(direct == false)
      {
        if(c->moveOnLattice(&rollDouble_R) == true)
        {
          c->superSpecialBlink(&teal, 40);
          failedMoveCounter = 0;
        }
        else
          failedMoveCounter++;
      }      
    }
//************** End if else if chain **************************
    loopCounter++;
    nextBehavior = basicUpkeep(c, nextBehavior);  // check for neighbors, etc...
  }
  return(nextBehavior);
}

Behavior multiSeekLight(Cube* c)
{
 // General Starting things... initialize flags, etc...
  if(MAGIC_DEBUG) {Serial.println("***Beginning MULTISeekLight***");}
  Behavior nextBehavior = MULTI_LIGHT_TRACK;
  int loopCounter = 0;
  bool correctPlane = false;
  int failedMoveCounter = 0;
  int connectedFace1 = c->whichFaceHasNeighbor(0);
  int connectedFace2 = c->whichFaceHasNeighbor(1);
  c->lightFace(connectedFace1,&yellow);
  delay(300);
  c->lightFace(connectedFace2,&red);
  delay(300);
  c->lightCube(&off);
  // if basic upkeep decides to change behavior, we exit now...
  // otherwise we keep running in this loop until something 
  // changes the behaviour
  while((nextBehavior == MULTI_LIGHT_TRACK) && // if we haven't changed state
        (c->numberOfNeighbors(0,0) > 1)  && // and if we have more than one neighbor
        (millis() < c->shutDownTime))        // and if we aren't feeling sleepy
  {
    if(correctPlane == false)
    {
      if(c->isPlaneParallel(connectedFace1) == true) // if our plane is alligned with the direction we want to move...
      {
        correctPlane = true;
      }
      else
        {
          magicVariable = 1;
          magicFace = connectedFace1; 
        }
  }
    // Regular Light Tracking...
    else
    {
      int brightestFace = c->returnXthBrightestFace(0, true);
      int nextBrightestFace = c->returnXthBrightestFace(1, true);
      //
      c->lightFace(brightestFace, &purple);
      wifiDelay(400);
      c->lightFace(nextBrightestFace, &green);
      wifiDelay(400);
      c->lightCube(&off);
      wifiDelay(100);
      // Figure out which way we should try to move
      bool direct = false; // false = reverse...
      if(brightestFace == c->returnForwardFace())
        direct = true;
      else if(brightestFace == c->returnReverseFace())
        direct = false;
      // Ok Nothing is directly Aligned... Checking Next Brightest
      else if(nextBrightestFace == c->returnForwardFace())
        direct = true;
      else if(nextBrightestFace == c->returnReverseFace())
        direct = false;
      /*
       * Begin if/ else if statement tree...
       */
      if(failedMoveCounter > 2)
      {
        if((c->returnTopFace(0) > -1) && (c->returnTopFace(0) < FACES) && (c->returnForwardFace() != -1)) // if this is true we are on the ground... so we should try to change planes
        {
          magicFace = c->returnTopFace(); 
          magicVariable = 1; // this triggers a plane change later in the program...
        }  
        else // we are not on the ground... So we jump a little bit...
        {
          c->moveOnLattice(&traverse_F);
        }
        if(c->returnForwardFace() == -1) // this is a proxy for plane being parallel to ground... or an error
        {
          c->moveOnLattice(&cornerClimb_F);
          delay(1500);
        }
      }    
      else if(direct == true)
      {
        if(c->moveOnLattice(&cornerClimb_F) == true)
        {
          c->superSpecialBlink(&teal, 40);
          failedMoveCounter = 0;
        }
        else
          failedMoveCounter++;
      }
      else if(direct == false)
      {
        if(c->moveOnLattice(&cornerClimb_R) == true)
        {
          c->superSpecialBlink(&teal, 40);
          failedMoveCounter = 0;
        }
        else
          failedMoveCounter++;
      }      
    }
//************** End if else if chain **************************
    loopCounter++;
    nextBehavior = basicUpkeep(c, nextBehavior);  // check for neighbors, etc...
  }
  return(nextBehavior);
}

//=============================================================================================
//=============================MAGNETIC TAGS CHECKING==========================================
//=============================================================================================
//bool returnNeighborPresence(int index);
//int returnNeighborID(int index);
//int returnNeighborAngle(int index);
//int returnNeighborFace(int index);
//TagType returnNeighborType(int index);
//TagCommand returnNeighborCommand(int index);

int checkForMagneticTagsStandard(Cube* c)
/*
 * This function processes the 6 magnetic tags on all 6 faces... 
 */
{
  int neighbors = 0; // running count of how many actual cube neighbors we have...
  if(MAGIC_DEBUG) {Serial.println("Checking for MAGNETIC TAGS");}
  for(int face = 0; face < 6; face++)
  { 
    /* This gets activated if we are attached to an actual cube or passive cube
     * for at least two time steps...
     */
    if((c->faces[face].returnNeighborType(0) == TAGTYPE_PASSIVE_CUBE) ||
       (c->faces[face].returnNeighborType(0) == TAGTYPE_REGULAR_CUBE))
    {
      neighbors++;
      if(c->faces[face].returnNeighborAngle(0) > -1)
      {
        c->lightFace(faceArrowPointsTo(face, c->faces[face].returnNeighborAngle(0)), &purple);
        wifiDelay(200);
        c->lightCube(&off);
      }
    }
    
    /*
     * The following is the behavior we want to trigger if we see a specific
     * tag.
     */
    if(c->faces[face].returnNeighborCommand(0) == TAGCOMMAND_SLEEP)
      wifiDelay(100);
    
    if((c->faces[face].returnNeighborCommand(0) == TAGCOMMAND_PURPLE) ||
      (magicVariable == 1))
    {
      int z = face;
      if(magicVariable)
      {
        z = magicFace;
        magicFace = 0;
      }
      magicVariable = 0;
      c->goToPlaneParallel(z);
    }
    
    if(c->faces[face].returnNeighborCommand(0)      == TAGCOMMAND_27)
      wifiDelay(100);

    else if(c->faces[face].returnNeighborCommand(0) == TAGCOMMAND_19)
      wifiDelay(100);
      
    else if(c->faces[face].returnNeighborCommand(0) == TAGCOMMAND_23) 
      c->shutDown();
    
    else if(c->faces[face].returnNeighborCommand(0) == TAGCOMMAND_13_ESPOFF)
      c->shutDownESP();
  }
  return(neighbors);
}

/*********************************************************
 *  Everything involving adding/removing behaviors is here...
 *  (1) cmdToBehaviors | converts takes in a String, and 
 *    returns the matching behavior
 *  (2) behaviorsToCmd | works in the opposite way,
 *    converting from a Behavior into a string
 *  (3) CheckForBehaviors | This actually runs the behaviors
 *    it is running in the main loop...
 *   
 *   
 *   (1) and (2) are used to "pack" and "unpack" behaviors
 *   into strings, this is primarily used to allow cubes to 
 *   set the behaviors of their neighbors through WIFI
 */

Behavior cmdToBehaviors(String cmd, Behavior defaultBehavior)
/*  
 *   This function takes in a string, and if it matches to a behavior, it 
 *   returns that behavior. If the string doesn't match, it returns
 */
{
  Behavior behaviorToReturn = defaultBehavior;
       if(cmd == "solo_light_track")    {behaviorToReturn = SOLO_LIGHT_TRACK;}
  else if(cmd == "duo_light_track")     {behaviorToReturn = DUO_LIGHT_TRACK;}
  else if(cmd == "follow_arrows")       {behaviorToReturn = FOLLOW_ARROWS;}
  else if(cmd == "chilling")            {behaviorToReturn = CHILLING;}
  else if(cmd == "climb")               {behaviorToReturn = CLIMB;}
  else if(cmd == "attractive")          {behaviorToReturn = ATTRACTIVE;}
  else if(cmd == "shut_down")           {behaviorToReturn = SHUT_DOWN;}
  else if(cmd == "sleep")               {behaviorToReturn = SLEEP;}
  else if(cmd == "multi_light_track")   {behaviorToReturn = MULTI_LIGHT_TRACK;}
  else if(cmd == "pre_solo_light")      {behaviorToReturn = PRE_SOLO_LIGHT;}
  
return(behaviorToReturn);
}

String behaviorsToCmd(Behavior inputBehavior)
/*  
 *   This function takes in a string, and if it matches to a behavior, it 
 *   returns that behavior. If the string doesn't match, it returns
 */
{
  String stringToReturn = " ";
       if(inputBehavior == SOLO_LIGHT_TRACK)      {stringToReturn = "solo_light_track";} 
  else if(inputBehavior == DUO_LIGHT_TRACK)       {stringToReturn = "duo_light_track";}
  else if(inputBehavior == FOLLOW_ARROWS)         {stringToReturn = "follow_arrows";}
  else if(inputBehavior == CHILLING)              {stringToReturn = "chilling";}
  else if(inputBehavior == CLIMB)                 {stringToReturn = "climb";}
  else if(inputBehavior == ATTRACTIVE)            {stringToReturn = "attractive";}
  else if(inputBehavior == SHUT_DOWN)             {stringToReturn = "shut_down";}
  else if(inputBehavior == SLEEP)                 {stringToReturn = "sleep";}
  else if(inputBehavior == MULTI_LIGHT_TRACK)     {stringToReturn = "multi_light_track";}
  else if(inputBehavior == PRE_SOLO_LIGHT)        {stringToReturn = "pre_solo_light";}
return(stringToReturn);
}

Behavior checkForBehaviors(Cube* c, Behavior behavior)
{
  //c->behaviorBuffer.push(behavior); // adds the behavior into a buffer
  if (behavior == SOLO_LIGHT_TRACK)
    behavior = soloSeekLight(c);
  else if (behavior == DUO_LIGHT_TRACK)
    behavior = duoSeekLight(c);
  else if (behavior == MULTI_LIGHT_TRACK)
    behavior = multiSeekLight(c);
  else if (behavior == FOLLOW_ARROWS)
    behavior = followArrows(c);
  else if (behavior == CLIMB)
    behavior = climb(c);
  else if (behavior == CHILLING)
    behavior = chilling(c);
  else if (behavior == ATTRACTIVE)
    behavior = attractive(c);
  else if (behavior == DEMO)
    behavior = demo(c);
  else if (behavior == SLEEP)
    behavior = sleep(c);
  else if (behavior == PRE_SOLO_LIGHT)
    behavior = Pre_Solo_Light(c);
  else
  {
    //Serial.println("ERROR: unknown behavior.  Reverting to \"CHILLING\"");
    behavior = CHILLING;
  }
  return(behavior);
}

void blinkFaceLeds(Cube* c, int waitTime)
{
  for(int i = 0; i < 6; i++)
  {
    c->faces[i].turnOnFaceLEDs(0,1,0,1); 
  }
  wifiDelay(50);
  for(int i = 0; i < 6; i++)
  {
    c->faces[i].turnOffFaceLEDs(); 
  }
}

void wifiLightChange(Cube* c, int number, bool turnOff)
{
int waitTime = 500;
switch (number) 
  {
  //********************************
  case 0: 
    c->lightCube(&white);
    wifiDelay(waitTime);
    break;
  case 1: 
    c->lightCube(&yellow);
    wifiDelay(waitTime);
    break;
  case 2: 
    c->lightCube(&red);
    wifiDelay(waitTime);
    break;
  case 3: 
    c->lightCube(&purple);
    wifiDelay(waitTime);
    break;
  case 4: 
    c->lightCube(&blue);
    wifiDelay(waitTime);
    break;
  case 5: 
    c->lightCube(&green);
    wifiDelay(waitTime);
    break;
  case 6: 
    c->lightCube(&teal);
    wifiDelay(waitTime);
    break;
  }
  if(turnOff)
  {
    c->lightCube(&off);
  }
}       
//
//void printDebugThings(Cube* c, Behavior behaviorToReturnFinal)
//{ 
//  Serial.println("-------------------------------------------");
//  Serial.println("Ending Basic Upkeep, here is what we found:");
//  Serial.print("Top face: ");           Serial.println(c->returnTopFace());
//  Serial.print("Current Plane: ");      
//  Serial.print("forward Face ");        Serial.println(c->returnForwardFace());
//  Serial.print("# of Neighbors: ");     Serial.println(c->numberOfNeighbors(0,0));
//  Serial.print("Resultalt Behavior: "); Serial.println(behaviorsToCmd(behaviorToReturnFinal));
//  Serial.println("--------------Ending BASIC UPKEEP---------------");
//}
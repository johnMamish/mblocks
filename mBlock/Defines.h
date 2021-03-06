#ifndef DEFINES
#define DEFINES
#include <Arduino.h>

/*
 * Number of faces on a cube - often used in for loops to iterate over all 6 faces..
 */
#define FACES 6 
#define CUBES 17

/*
 * This is a special message ID which the cubes use to communicate to the
 * server
 */
#define CONFIRM_MESSAGE_ID 333

/*
 * This number is the cube's unique ID number, Populated from a look up table in initialization.cpp
 * This is used for WIFI messages, and for updating calibration data
 */
extern int thisCubeID;

/*
 * Top Level State machine, 
 * The most common values are:
 *    Line - try to assemble into a line
 *    LightSeek - try to follow light
 *    Cube - Turn into Giant Cube
 */
extern String Game;
extern String oldGame;
/*
 * These variables are involved with the light tracking state machine
 */
extern bool PART_OF_LINE;
extern bool THE_CHOSEN_ONE;
extern bool MAGIC_THE_LIGHT;
extern int FACES_LIGHTS[FACES];

/*
 * Global variables related to GRID
 */
extern bool LIGHT_TOGGLE_STATE;
extern int  LIGHT_TOGGLE_TIME;

/*
 * Double check makes it so thatwhenwe request a plane change, it only happens if we
 * ask for the same plane change twice in a row.
 */
extern bool DOUBLE_CHECK;

/*
 * Some of therobots havebad batteries, so inorder to prevent electrical problems
 * HALF_LIGHT limites the number of lights we turn on for specific cubes,
 * this is changed to true/false by initialization.cpp for each cube
 */
extern bool HALF_LIGHT;
extern bool HALF_BRIGHT;
extern int ARROW_OFFSETS[CUBES][FACES]; 

/*
 * The following are global variables involved with *CUBE game
 */
#define TOP_LIGHT_SAMPLES 12
extern int TOP_FACE_LIGHT[TOP_LIGHT_SAMPLES];
extern int TOTAL_LIGHT;
#define TOP_LIGHT_THRESHOLD 1000
extern int FACES_LIGHTS[FACES];
extern int LIGHT_TRANSITIONS;
extern int DIRECTION;
extern long timeLightTurnedOn;

/*
 * These Global variables are involved with sending an acknowledgement message
 * back to the server...
 */
extern int bFace;
extern int fFace;
extern int f0;
extern int f1;
extern int f2;
extern int f3;
extern int f4;
extern int f5;

//   
#define wifiAddress_SERVER  885790061   //  Sparkfun THing SERVER
#define wifiAddress_cube00  2133796284  //  Sparkfun Thing LArge Breadboard
//  16 Actual Cubes...
#define wifiAddress_cube01  887459591  //  =D  //  {960662, 1},    // PEI BROWN  - F1:E8:71:B2:99:B5

#define wifiAddress_cube02  882644344   // =D //  {10229112, 2},  // PC RED  - DF:DF:3C:A0:F1:77
#define wifiAddress_cube03  2131666780  //    //  {15044359, 3}   // ORANGE PC RED  CD:2B:5E:AB:3E:F3
#define wifiAddress_cube04  2139281740  //    //  {8575308, 4},   // PC Green : ED:A6:6A:8E:1B:58
#define wifiAddress_cube05  2139282624  // =D //  {960427, 5},    // PEI RED  D0:D5:6F:CB:32:4C
#define wifiAddress_cube06  8874595915  //** // =D //  {960348, 15},   // PEI GREEN ----- SWITCHED WITH PC BROWN
#define wifiAddress_cube07  2131666674  // =D //  {960242, 7},    // PEI ORANGE E6:F6:05:69:08:F2
#define wifiAddress_cube08  2131666141  // =D //  {959709, 8},    // PC YELLOW  - FB:0D:8F:2C:3B:B4           
#define wifiAddress_cube09  2131666126  //    //  {959694, 14},   // PC BLACK | FA:AA:25:19:C7:DF
#define wifiAddress_cube10  2131667020  // =D //  {960558, 10},   // PEI BLUE  f7:AE:59:2B:D9:4D
#define wifiAddress_cube11  2139283535  //    //  {959839, 16},   // PC RED
#define wifiAddress_cube12  2131667094  // =D //  {960043, 12},   // PEI YELLOW  CC:F1:4F:AF:64:A8  
#define wifiAddress_cube13  887459658   // =D //  {15044426, 13}, // PC Blue  D8:9C:4D:EA:27:65
#define wifiAddress_cube14  2139282946  // =D //  {8576514, 9},   // PEI PURPLE  E3:6B:C6:CE:DA:31 
#define wifiAddress_cube15  2139284147  // =D //  {8577715, 6},   // PEI GREEN  C5:FF:AB:04:3B:9D --- Switched with PC Green
#define wifiAddress_cube16  2131666271  // =D //  {8577103, 11},  // PEI BLACK DB:9D:99:1A:BA:23
// Hardware Pin Definitions

/*  Digital Output | Switch which controlls power to the face boards,
 *  High = power given to faceboards, Low = ability to charge
 */
#define Switch 12 

/*
 * Digital Output | Directly Controlls a small white LED on the "Master" circuit board
 */
#define LED             13    
#define SDA             2     //  Managed by the wire.begin in initializeCube()
#define SCL             14    //  Managed by the wire.begin in initializeCube() 
#define SPECIAL_MID     42

extern int GlobalPlaneAccel;
extern int GlobalMaxAccel;

extern int GlobalplaneChangeTime;
extern int GlobalplaneChangeRPM;
extern int magicVariable;

extern int MAGIC_DEBUG;
extern int magicFace;

extern int ADDITION_FACTOR_GRID_RPM;
extern int ADDITION_FACTOR_GRID_CURRENT;

extern int TRAVERSE_RPM_F;
extern int TRAVERSE_RPM_R;

extern int GRID_TRAVERSE_RPM_F;
extern int GRID_TRAVERSE_RPM_R;
extern int GRID_TRAVERSE_CURRENT_F;
extern int GRID_TRAVERSE_CURRENT_R;

extern int TRAVERSE_CURRENT_F;
extern int TRAVERSE_CURRENT_R;
extern int TRAVERSE_BRAKETIME_F;
extern int TRAVERSE_BRAKETIME_R;

extern int STEP_DOWN_STAIR_CURRENT_F;
extern int STEP_DOWN_STAIR_CURRENT_R;

extern int CC_RPM_F;
extern int CC_RPM_R;
extern int CC_CURRENT_F;
extern int CC_CURRENT_R;
extern int CC_BRAKETIME_F;
extern int CC_BRAKETIME_R;

extern int horizontal_Stair_RPM_F;
extern int horizontal_Stair_RPM_R; 

extern int horizontal_Stair_CURRENT_F;
extern int horizontal_Stair_CURRENT_R;

extern int horizontal_CURRENT_F;
extern int horizontal_CURRENT_R;

      

/*
 * Plane Enums are the possible states that the internal robot can be in
 * in regards to the frame. Functions involving PlaneEnums involve 
 * 
 * PlaneEnum Cube::findPlaneStatus(bool reset) - returns (And adds to buffer) the current plane 
 * bool Cube::setCorePlaneSimple(PlaneEnum targetCorePlane) - actually attempts to go to a certian plane
 * 
 * Also See:
 * bool Cube::isValidPlane() - returns TRUE if 
 * PlaneEnum returnPlane(int face1, int face2)
 */
typedef enum PlaneEnum 
{
  PLANE0425,  // Valid Plane, with face "1" On Top
  PLANE0123,  // Valid Plane, with face "5" On Top
  PLANE1453,  // Valid Plane, with face "2" On Top
  PLANENONE,  // Means we are inbetween two valid planes
  PLANEMOVING,// gyro values show we are moving, measurements are therefor unreliable
  PLANEERROR  // Error - maybe I2C bus is disconnected?
} PlaneEnum;

String planeEnumToString(PlaneEnum thePlaneNow);

PlaneEnum stringToPlaneEnum(String inputString);

/*
 * TagCommands are different types of "commands" that can be contained within 
 * the special magnetic tags. Commands are designated by having the two magnetic Values
 * to be about the same value, i.e. Both digits are 27, or one is 26 and the other is 27
 */

 
typedef enum TagCommand
{
  TAGCOMMAND_NONE,
  TAGCOMMAND_09_ORANGE,
  TAGCOMMAND_13_RED,
  TAGCOMMAND_23_BLUE,
  TAGCOMMAND_27_GREEN,
  TAGCOMMAND_05_PURPLE
} TagCommand;

typedef enum TagType
{
  TAGTYPE_NOTHING,
  TAGTYPE_INVALID,
  TAGTYPE_REGULAR_CUBE,
  TAGTYPE_PASSIVE_GRID,
  TAGTYPE_PASSIVE_CUBE,
  TAGTYPE_COMMAND
} TagType;

typedef enum Behavior
{
    PRE_SOLO_LIGHT,
    SOLO_LIGHT_TRACK,
    DUO_LIGHT_TRACK,
    MULTI_LIGHT_TRACK,
    FOLLOW_ARROWS,
    CHILLING,
    CLIMB,
    STAIRCLIMB,
    ATTRACTIVE,
    TESTING, 
    SHUT_DOWN,
    SLEEP,
    RELAY_SLEEP
} Behavior;

// This is a struct for organizing information about motions
typedef struct Motion
{
  String moveName;      // name of move, used when we instantiate the type of movement
  bool brake; // true = use mechanical brake  || false = use the electronic brake only
  int rpm;              // RPM that we 
  int timeout;       // time that we will wait untill for confirmation of the speed
  int current;     // Current we apply to the brake in mA ... Maximum is 6000 ma
  int brakeTime;        // Time (milli Seconds) we apply the brake for Maximum time*current < some value
  int expected_neighbors;     // number of neighbors we expect to have
  String for_rev;
} Motion;

// List of Possible Motions, defined in Defines.cpp
extern Motion traverse_F;
extern Motion traverse_R;

extern Motion roll_F;
extern Motion roll_R;

extern Motion grid_traverse_F;
extern Motion grid_traverse_R; 

extern Motion cornerClimb_F;
extern Motion cornerClimb_R;

extern Motion horizontal_F;
extern Motion horizontal_R;

extern Motion stepDownStair_R;
extern Motion stepDownStair_F;

extern Motion horizontal_Stair_F;
extern Motion horizontal_Stair_R;

extern Motion shake_F;
extern Motion softShake_F;

extern Motion explode_F;
extern Motion explode_R;

extern Motion rollDouble_F;
extern Motion rollDouble_R;

// Colors
typedef struct Color
{
  bool r;
  bool g;
  bool b;
} Color;

//  List of possible colors;
extern Color red;
extern Color green;
extern Color blue;
extern Color yellow;
extern Color purple;
extern Color teal;
extern Color white;
extern Color off;

//These tables define connectivity between faces for the cube
const extern PlaneEnum facePlanes[FACES][FACES];
const extern int faceRotations[FACES][4];
// Helper Functions
#define ARRAY_SIZEOF(x) ((sizeof(x) / sizeof(x[0])))

uint32_t getAddressFromCubeID(int CubeID);
PlaneEnum returnPlane(int face1, int face2);

int faceArrowPointsTo(int readingFace, int connectionAngle);
int faceClockiness(int faceTowards, int faceReference);

String planeEnumToString(PlaneEnum thePlaneNow);

#endif

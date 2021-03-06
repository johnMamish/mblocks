#ifndef FACE_H
#define FACE_H

#include "Defines.h"
#include "CBuff.h"
#include "Cube.h"
#include <Arduino.h>

// Face board pins
// Least significant byte
#define FB_LED1 7
#define FB_LEDB 5
#define FB_EN2  4
#define FB_EN1  3
#define FB_LEDA 2

// Most significant byte
#define FB_LEDC 7
#define FB_R1 5
#define FB_B1 4
#define FB_G1 3
#define FB_B2 2
#define FB_R2 1
#define FB_G2 0

/*
 * The IO expanders are the main "processor" on each face board,
 * they are basically remote controlled switches, turning on/off LED's
 * and other sensors.
 * 
 * Each Face knows its own number, which range from 32 to 37
 * Can be referenced like this...
 * switch(this->IOExpanderAddress)
  {
    case 32: //Face 0
      break;
    case 33: // Face 1
      break;
      So ...
      __ IOExpanderAddress - IO_Address_offset = face number from 0 to 5
 */
#define IO_Address_offset 32

//typedef enum TagCommand
//{
//  TAGCOMMAND_NONE,
//  TAGCOMMAND_SLEEP,
//  TAGCOMMAND_PURPLE,
//  TAGCOMMAND_27,
//  TAGCOMMAND_25,
//  TAGCOMMAND_23,
//  TAGCOMMAND_21,
//  TAGCOMMAND_19,
//} TagCommand;
//
//typedef enum TagType
//{
//  TAGTYPE_NOTHING,
//  TAGTYPE_INVALID,
//  TAGTYPE_REGULAR_CUBE,
//  TAGTYPE_PASSIVE_CUBE,
//  TAGTYPE_COMMAND
//} TagType;

typedef struct Tag
/*
 * Magnetic Tags are essentially like special barcodes which the module is able to read through its
 * magnetic sensors embedded on each face. These barcodes are a little bit "hackey" but the work
 * well eneough in order to detect other modules, or specialized command "tags"
 */
{
  TagType type;   
  //  TAGTYPE_NOTHING,
  //  TAGTYPE_INVALID,
  //  TAGTYPE_REGULAR_CUBE,
  //  TAGTYPE_PASSIVE_CUBE,
  //  TAGTYPE_COMMAND

  int angle ; // Either -1, 0, 1, 2 or 3 - corresponding to 90 deg angle
  int id; // ID or message code attached to tag
  int face; // face number (0,1,2,3,4,5) associated with a cube
  int strength; // Validitity of the tag, basically just agc1+agc2
  int lightDigit; // stores a digital light digit... 0 = off, digit is blinking, 2, digit is solid on
  TagCommand command; // Text of command or behavior to go to... if it exists
} Tag;

class Face
{ 
  private:
    // Cube attributes
    byte IOExpanderState[2]; // this stores 16 BITS which correspond to the outputs of the IO expander
    const int ambientSensorAddress = 0x39; // i2c address for ambient light sensor on face version == 1;
    int faceMagnetAddresses[2]; // addresses for the AS5048 Magentic encoders on each face...
    
    void getMagnetEncoderAddresses(int* target)
    {
      target[0] = 0x43; target[1] = 0x42;
    }
    
      // Data storage spaces
    int ambientData[30];
    CircularBuffer<int> ambientBuffer;

      // Magnetic data Buffers
    int magnetAngleData_A[4];
    int magnetStrengthData_A[4];
    int magnetAngleData_B[4];
    int magnetStrengthData_B[4];
    
    CircularBuffer<int> magnetAngleBuffer_B;
    CircularBuffer<int> magnetStrengthBuffer_B;
    CircularBuffer<int> magnetAngleBuffer_A;
    CircularBuffer<int> magnetStrengthBuffer_A;
    
      // Neighbor Information Buffers
    int neighborData[4];
    TagType     neighborTypeData[4];
    TagCommand  neighborCommandData[4];
    int neighborIDData[4];
    int neighborFaceData[4];
    int neighborAngleData[4];
    bool neighborPresenceData[4];
    
    int neighborLightDigitData[6];
    
      // Circular Buffers for Magnetic Tag Variables
    CircularBuffer<TagType>     neighborTypeBuffer;     // This stores the type of the tag on this face
    CircularBuffer<TagCommand>  neighborCommandBuffer;  // this stores the "command" associated 
    //with the tag... if any.
    CircularBuffer<int>         neighborIDBuffer;       
    CircularBuffer<int>         neighborFaceBuffer;
    CircularBuffer<int>         neighborAngleBuffer;
    CircularBuffer<bool>        neighborPresenceBuffer;
    CircularBuffer<int>         neighborLightDigitBuffer;
    
  public:
      // Constructors
    Face();   
    Face(int expanderAddress);
//**************************************************************************************************************
    bool updateFace(); // Enables sensors, Updates ambient values, updates magnetic sensors, neighbors...
//**************************************************************************************************************

      // Public Variables
    const int r_0 = 13; // Pins on PCF7585 IO expander on each face.
    const int g_0 = 11; // corner RGB pins... directly drive LED's
    const int b_0 = 12;
    const int r_1 = 9;
    const int g_1 = 8;
    const int b_1 = 10;
    const int led_A = 7;    // Second byte of pins
    const int led_B = 5;
    const int led_C = 15;
    const int led_D = 2;
    const int EN1 = 3;
    const int EN2 = 4; // ... Pins on PCF7585 IO expander on each face.
    
    int IOExpanderAddress; // address for the input expander on this particular face
    
      // Utility Commands
    bool updateIOExpander();
    bool setPinLow(int pin);
    bool setPinHigh(int pin);
    void setIOExpanderAddress(int a) {this->IOExpanderAddress = a;}

      //LED Related Commands
    bool turnOnFaceLEDs(bool LED_A = true, bool LED_B = true, bool LED_C = true, bool LED_D = true); 
    bool turnOffFaceLEDs();
    void blinkRing(int delayLength = 100, int numberOfTimes = 6);

      // Sensor Related Commands
    bool enableSensors();   
    bool disableSensors();  
    bool updateAmbient(bool activate = true);   
    bool readAmbient(bool activate = true);
    bool updateMagneticBarcode(); // Reads both magnet sensors,
    bool isThereNeighbor(); // true == yes! // false == NO
    int processLightData(int samplesTaken);
    
    void blinkOutMessage(int digit); 
    void blinkRingDigit(int digit, int numberOfTimes);

      // Magnetic Tag Related Commands
    bool processTag();
    
      // Return from Circular Buffer Commands
    int returnMagnetAngle_A(int index);
    int returnMagnetAngle_B(int index);
    int returnMagnetStrength_A(int index);
    int returnMagnetStrength_B(int index); 
    
    int returnAmbientValue(int index);
    void forceUpdateAmbientValue(int value); // used to simplify sorting...
      // int returnReflectivityValue(int index);

    bool returnNeighborPresence(int index);
    int returnNeighborID(int index);
    int returnNeighborAngle(int index);
    int returnNeighborFace(int index);
    int returnNeighborLightDigit(int index);
    TagType returnNeighborType(int index);
    TagCommand returnNeighborCommand(int index);
};

void activateLightSensor(int address);
int readMagnetSensorAngle(int i2cAddress);
int readMagnetSensorFieldStrength(int i2cAddress);
int magnetSensorRead(int i2cAddress, byte dataRegisterAddress);
void analyzeTag(int angle1, int agc1, int angle2, int agc2, Tag*);
int returnFaceNumber(int magDigit); // gives the value of the face given magnetic Digits... 
//converts from 0-30 to 

#endif


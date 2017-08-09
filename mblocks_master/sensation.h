#ifndef SENSATION_H
#define SENSATION_H

#include <ArduinoHardware.h>
#include "defines.h"

int readAmbient(int address);
void activateLightSensor(int address);
void readGyroAndAccel(int i2cAddress);
int readMagnetSensorAngle(int i2cAddress);
int readMagnetSensorFieldStrength(int i2cAddress);
int magnetSensorRead(int i2cAddress, byte dataRegisterAddress);
void wakeMPU(int i2cAddress);

#endif

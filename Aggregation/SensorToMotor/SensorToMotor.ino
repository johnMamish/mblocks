/******************************************************************************
 * Sensor reading should be an `int' array of 4.
 * Sensor Readings
 * ------------------------
 * 0 => no neighbors  
 * 1 => Light only
 * 2 => Neighbor in contact
 * Actions:
 * -----------------------
 * 0: Stop. 1
 * All possible sensor readings are given in AllSensors
******************************************************************************/

// Some definitions
const int nContext = 81; // Number of Unique Contexts
const int nSensor  = 4; // Number of Sensors
const int dt = 500; // Delay-time.

// Random Choice:
const int Weight = 1;
const int RandNumMax = 1000000;

// Definitions: Hard-coded.
int AllSensors[nContext][nSensor] = {
    {0,0,0,0},
    {0,0,0,2},
    {0,0,2,0},
    {0,0,2,2},
    {0,2,0,0},
    {0,2,0,2},
    {0,2,2,0},
    {0,2,2,2},
    {2,0,0,0},
    {2,0,0,2},
    {2,0,2,0},
    {2,0,2,2},
    {2,2,0,0},
    {2,2,0,2},
    {2,2,2,0},
    {2,2,2,2},
    {1,1,1,1},
    {1,1,1,0},
    {0,1,1,1},
    {1,0,1,1},
    {1,1,0,1},
    {1,1,1,2},
    {2,1,1,1},
    {1,2,1,1},
    {1,1,2,1},
    {2,1,2,1},
    {1,2,1,2},
    {2,1,0,1},
    {1,2,1,0},
    {0,1,2,1},
    {1,0,1,2},
    {0,1,0,1},
    {1,0,1,0},
    {1,1,2,2},
    {2,1,1,2},
    {2,2,1,1},
    {1,2,2,1},
    {1,1,0,2},
    {2,1,1,0},
    {0,2,1,1},
    {1,0,2,1},
    {1,1,2,0},
    {0,1,1,2},
    {2,0,1,1},
    {1,2,0,1},
    {1,1,0,0},
    {0,1,1,0},
    {0,0,1,1},
    {1,0,0,1},
    {1,2,2,2},
    {2,1,2,2},
    {2,2,1,2},
    {2,2,2,1},
    {1,0,2,2},
    {2,1,0,2},
    {2,2,1,0},
    {0,2,2,1},
    {1,2,0,2},
    {2,1,2,0},
    {0,2,1,2},
    {2,0,2,1},
    {1,2,2,0},
    {0,1,2,2},
    {2,0,1,2},
    {2,2,0,1},
    {1,0,0,2},
    {2,1,0,0},
    {0,2,1,0},
    {0,0,2,1},
    {1,0,2,0},
    {0,1,0,2},
    {2,0,1,0},
    {0,2,0,1},
    {1,2,0,0},
    {0,1,2,0},
    {0,0,1,2},
    {2,0,0,1},
    {1,0,0,0},
    {0,1,0,0},
    {0,0,1,0},
    {0,0,0,1}};

float AllParameters[nContext][nSensor+1] = {
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {1.000000,0.000000,0.000000,0.000000,0.000000},
      {0.992677,0.001831,0.001831,0.001831,0.001831},
      {0.664613,0.001601,0.333195,0.000591,0.000000},
      {0.664613,0.000000,0.001601,0.333195,0.000591},
      {0.664613,0.000591,0.000000,0.001601,0.333195},
      {0.664613,0.333195,0.000591,0.000000,0.001601},
      {0.941329,0.000070,0.044790,0.013810,0.000000},
      {0.941329,0.000000,0.000070,0.044790,0.013810},
      {0.941329,0.013810,0.000000,0.000070,0.044790},
      {0.941329,0.044790,0.013810,0.000000,0.000070},
      {0.008827,0.000000,0.495587,0.000000,0.495587},
      {0.008827,0.495587,0.000000,0.495587,0.000000},
      {0.043784,0.000000,0.465886,0.000000,0.490330},
      {0.043784,0.490330,0.000000,0.465886,0.000000},
      {0.043784,0.000000,0.490330,0.000000,0.465886},
      {0.043784,0.465886,0.000000,0.490330,0.000000},
      {0.003692,0.000000,0.498154,0.000000,0.498154},
      {0.003692,0.498154,0.000000,0.498154,0.000000},
      {0.996154,0.000463,0.003383,0.000000,0.000000},
      {0.996154,0.000000,0.000463,0.003383,0.000000},
      {0.996154,0.000000,0.000000,0.000463,0.003383},
      {0.996154,0.003383,0.000000,0.000000,0.000463},
      {0.165319,0.499262,0.335420,0.000000,0.000000},
      {0.165319,0.000000,0.499262,0.335420,0.000000},
      {0.165319,0.000000,0.000000,0.499262,0.335420},
      {0.165319,0.335420,0.000000,0.000000,0.499262},
      {0.496369,0.004311,0.499320,0.000000,0.000000},
      {0.496369,0.000000,0.004311,0.499320,0.000000},
      {0.496369,0.000000,0.000000,0.004311,0.499320},
      {0.496369,0.499320,0.000000,0.000000,0.004311},
      {0.002682,0.499544,0.497773,0.000000,0.000000},
      {0.002682,0.000000,0.499544,0.497773,0.000000},
      {0.002682,0.000000,0.000000,0.499544,0.497773},
      {0.002682,0.497773,0.000000,0.000000,0.499544},
      {0.998558,0.001442,0.000000,0.000000,0.000000},
      {0.998558,0.000000,0.001442,0.000000,0.000000},
      {0.998558,0.000000,0.000000,0.001442,0.000000},
      {0.998558,0.000000,0.000000,0.000000,0.001442},
      {0.556494,0.443506,0.000000,0.000000,0.000000},
      {0.556494,0.000000,0.443506,0.000000,0.000000},
      {0.556494,0.000000,0.000000,0.443506,0.000000},
      {0.556494,0.000000,0.000000,0.000000,0.443506},
      {0.009498,0.990502,0.000000,0.000000,0.000000},
      {0.009498,0.000000,0.990502,0.000000,0.000000},
      {0.009498,0.000000,0.000000,0.990502,0.000000},
      {0.009498,0.000000,0.000000,0.000000,0.990502},
      {0.999902,0.000098,0.000000,0.000000,0.000000},
      {0.999902,0.000000,0.000098,0.000000,0.000000},
      {0.999902,0.000000,0.000000,0.000098,0.000000},
      {0.999902,0.000000,0.000000,0.000000,0.000098},
      {0.000015,0.999985,0.000000,0.000000,0.000000},
      {0.000015,0.000000,0.999985,0.000000,0.000000},
      {0.000015,0.000000,0.000000,0.999985,0.000000},
      {0.000015,0.000000,0.000000,0.000000,0.999985},
      {0.373871,0.626129,0.000000,0.000000,0.000000},
      {0.373871,0.000000,0.626129,0.000000,0.000000},
      {0.373871,0.000000,0.000000,0.626129,0.000000},
      {0.373871,0.000000,0.000000,0.000000,0.626129},
      {0.984537,0.015463,0.000000,0.000000,0.000000},
      {0.984537,0.000000,0.015463,0.000000,0.000000},
      {0.984537,0.000000,0.000000,0.015463,0.000000},
      {0.984537,0.000000,0.000000,0.000000,0.015463},
      {0.005356,0.994644,0.000000,0.000000,0.000000},
      {0.005356,0.000000,0.994644,0.000000,0.000000},
      {0.005356,0.000000,0.000000,0.994644,0.000000},
      {0.005356,0.000000,0.000000,0.000000,0.994644}};


void setup(){
  Serial.begin(115200);
//  randomSeed(analogRead(0)); // Random seed. Not sure if we need this. Commented out for now.
}

void loop()
{
    int SensorReading[4] = {2,1,1,0}; // Given sensor reading. The only part to be modified. Cannot be constant.
    int Index = FindSensorIndex(SensorReading); // Index of the sensor reading in the look-up table (between 0 and 80).
    float randNumber = (float) random(RandNumMax) / RandNumMax; // Generate a random number with 6 significant figure accuracy.
    int ActionIndex =  DiscreteDist(Index, randNumber); // Randomly sample an action (0 to 4) from given weights using the uniformly random generated number `randNumber'.
    

//    Debugger(Index, ActionIndex); // Only use for debug purpose.
}


// Function definitions.
int FindSensorIndex(int Reading[])
{
  int Index = 0;
  int Counter = 0;
  while (Index<nContext)
  {
    Counter = 0;
    // Go through each sensor element and count how many of them are the same.
    for (int i=0;i<nSensor;i++) if (Reading[i]==AllSensors[Index][i]) Counter++;
    // If all of them are the same, break the loop. Otherwise, continue search.
    if (Counter==nSensor) break;
    Index++;
  }
  return Index;
}


int DiscreteDist(int Index, float randNumber)
{
    for (int i = 0; i < nSensor + 1; i++)
    {
      if (randNumber < AllParameters[Index][i])
          return i;
      randNumber -= AllParameters[Index][i];
    }
}

void Debugger(int Index, int ActionIndex)
{
      String strToPrint;
      Serial.print("SensorIndex: "+ String(Index) + " -- Sensor: [");
      for(int i = 0; i<nSensor; i++)
      {
        strToPrint = String(AllSensors[Index][i]);
        if (i != nSensor-1) strToPrint += ", ";
        Serial.print(strToPrint);
      }
      Serial.print("]  --  Parameters: [");
      for(int i = 0; i<nSensor+1; i++)
      {
        strToPrint = String(AllParameters[Index][i], 6);
        if (i != nSensor) strToPrint += ", ";
        Serial.print(strToPrint);
      }
      Serial.print("]");
      Serial.println(" -- ActionIndex: " + String(ActionIndex));
      delay(dt);
}

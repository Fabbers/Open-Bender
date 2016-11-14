#include "setup.h"

// pin assignments
// Motor pulse and solenoid pins
const int bendMotorPls = 9;
const int zMotorPls = 10;
const int feedMotorPls = 11;
const int benderPin = 12;

// AWO pins to allow motor shaft to free spin
const int bendMotorAWO = 3;
const int zMotorAWO = 4;
const int feedMotorAWO = 5;

// Direction pins to select drive direction
const int bendMotorDir = 6;
const int zMotorDir = 7;
const int feedMotorDir = 8;

//misc program constants
const int pulseWidth = 100;
const int pulseDelay = 1000;

// Drive directions in english
boolean ccw = true; // counter-clockwise drive direction
boolean cw = false; // clockwise drive direction
boolean curDir = cw; // resets direction before next angle command 

int lastbend = 0;
int fieldindex=0;
int values[300]; //creates array

void setup() {
  Serial.begin(9600);
  pinMode (bendMotorPls,  OUTPUT); //Declaring motor pins as out
  pinMode (zMotorPls,     OUTPUT); 
  pinMode (feedMotorPls,  OUTPUT); 
  pinMode (bendMotorAWO,  OUTPUT); 
  pinMode (zMotorAWO,     OUTPUT); 
  pinMode (feedMotorAWO,  OUTPUT); 
  pinMode (bendMotorDir,  OUTPUT);
  pinMode (zMotorDir,     OUTPUT); 
  pinMode (feedMotorDir,  OUTPUT); 
  pinMode (benderPin,     OUTPUT); 
  
  digitalWrite (bendMotorPls, LOW); //starts with everything off
  digitalWrite (zMotorPls,    LOW); 
  digitalWrite (feedMotorPls, LOW);
   
  digitalWrite (benderPin,    LOW);
  
  digitalWrite (zMotorAWO,    HIGH);
  digitalWrite (feedMotorAWO, HIGH);
  digitalWrite (bendMotorAWO, HIGH);
  
  delay(10);

  int numb_of_sides = 8;
  
for(int i=0;i<numb_of_sides;i++)
{
  feed (50);
  delay(1);
  bend (40);
  delay(1);
  }  
}

void loop() 
{

}

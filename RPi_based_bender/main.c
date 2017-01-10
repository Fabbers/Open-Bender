#include <bcm2835.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


#define FEED_MOTOR_STEPS_PER_REVOLUTION 4000 
#define FEED_MOTOR_SHAFT_DIAMETER 30.4
#define FEED_MOTOR_STEPS_PER_MILIMETER 41.88   //FEED_MOTOR_STEPS_PER_REVOLUTION / FEED_MOTOR_SHAFT_DIAMETER * PI

#define BEND_MOTOR_SHAFT_DIAMETER 10
#define BEND_MOTOR_SHOULDER 50
#define BEND_MOTOR_STEPS_PER_REVOLUTION 4000 
#define GEAR2_GEAR1_RATIO 3
#define BEND_MOTOR_STEPS_PER_DEGREE 33.33    //FEED_MOTOR_STEPS_PER_REVOLUTION * GEAR2_GEAR1_RATIO  / 360

#define WIRE_THICKNESS 3

#define CMD_STOP 127
#define CMD_FEED 126
#define CMD_2D_BEND 125
#define CMD_3D_BEND 124


// pin assignments
// Motor pulse and solenoid pins
const int bendMotorPls = 9; //RPI_GPIO_P1_9;
const int feedMotorPls = 11; //RPI_GPIO_P1_11;
const int benderPin = 12; //RPI_GPIO_P1_12;

// AWO pins to allow motor shaft to free spin
const int bendMotorAWO = 3; //RPI_GPIO_P1_3;
const int feedMotorAWO = 5; //RPI_GPIO_P1_5;

// Direction pins to select drive direction
const int bendMotorDir = 6; //RPI_GPIO_P1_6;
const int feedMotorDir = 8; //RPI_GPIO_P1_8;

//Limit Switches 
const int minSwitch = 23 //RPI_GPIO_P1_23;
const int maxSwitch = 24 //RPI_GPIO_P1_24;

//misc program constants
const int pulseWidth = 0.1; //100 microseconds
const int pulseDelay = 1; //1000 microseconds

// Drive directions in english
const bool ccw = true; // counter-clockwise drive direction
const bool cw = false; // clockwise drive direction
bool curDir = false;
bool coldStart = true;

const float homePosition = 0;
float lastPinPosition = 0;

FILE *file_handler;

void setup()	
{
	curDir = cw; // resets direction before next angle command // pin assignments

	bcm2835_init();

	bcm2835_gpio_fsel(bendMotorPls, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(feedMotorPls, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(benderPin, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(bendMotorAWO, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(feedMotorAWO, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(bendMotorDir, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(feedMotorDir, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(benderPin, BCM2835_GPIO_FSEL_OUTP);

	bcm2835_gpio_fsel(minSwitch, BCM2835_GPIO_FSEL_INP);
	bcm2835_gpio_fsel(maxSwitch, BCM2835_GPIO_FSEL_INP);

}

void waitmSec(float mseconds);
void motorImpulse(int motor);

void bendWire(float angle);
void feedWire(float lenght);
void rotatePin(bool direction, float steps);
void pinReturn(float steps_to_home, float wire_thickness, bool direction);
void homingRoutine();
void duckPin();
int readCurPos();

int main(int argc, char **argv)
{
	/********
	Runs bendWire() and feedWire() needed amount of time.
	Bending function and feeding function should be linked in future to provide lenght/angle interdependence.
	********/
	setup();
	//int steps_to_home = readCurPos();
	pinReturn(2000, WIRE_THICKNESS, curDir);
	int i = 0;

	while (i < 5)
	{
		feedWire(50);
		waitmSec(1);
		bendWire(45);
		waitmSec(1);
		i++;
	}

	bcm2835_close();
	return 0;
}


void waitmSec(float mseconds)
{
	clock_t start, diff;
	float msec = 0;
	start = clock();
	
	while (msec < mseconds)
	{
		diff = clock() - start;
		msec = diff * 2000 / CLOCKS_PER_SEC;
//		printf("%.3f seconds %.3f milliseconds %.0f microseconds\n", msec/1000, msec, msec*1000);
	}
}


void motorImpulse(int motor)
{
	bcm2835_gpio_write(motor, HIGH); //moves bender pin back to home position ready for next feed
	waitmSec(pulseWidth);
	bcm2835_gpio_write(motor, LOW); //moves bender pin back to home position ready for next feed
	waitmSec(pulseDelay);
}


/*WIRE BENDING FUNCTION
Currently passes only 1 arg - angle, in prospect will pass wire thickness also
*/
void bendWire(float angle)
{
	if (angle != 0)
	{
		printf("Bending to %.2f degrees \n", angle);
		bool dir = cw;
		bool back = ccw;
		
		if (angle < 0)
		{
			dir = ccw;
			back = cw;
		}

		float steps = 0;
		angle = abs(angle);
		steps = angle * BEND_MOTOR_STEPS_PER_DEGREE;

		rotatePin(dir, steps); //move bender pin
		coldStart = false; //set flag that bending started and even one bending move was done
		waitmSec(100);
		//rotatePin(back, steps); //return bender pin
		//int steps_from_file = readCurPos();
		pinReturn(steps, WIRE_THICKNESS, back);
	}
}
/**/


/*WIRE FEEDING FUNCTION*/
void feedWire(float lenght)
{
	int i = 0;
	if (lenght != 0)
	{
		printf("Feeding %.2f mm\n", lenght);

		float steps = 0;
		steps = lenght * FEED_MOTOR_STEPS_PER_MILIMETER;
		
		printf("Steps %.2f \n", steps);

		bcm2835_gpio_write(feedMotorDir, 1);  //feed motor only moves forward

		for (i=0; i < steps; i++) //rotate feed motor appropriate number of steps 
		{
			motorImpulse(feedMotorPls);
		}
	}
}
/**/


/*PIN ROTATION FUNCTION*/
void rotatePin(bool direction, float steps) //moves bender pin
{
	int i = 0;
	printf("Steps %.2f \n", steps);

	bcm2835_gpio_write(bendMotorDir, direction); //moves bender pin back to home position ready for next feed
	
	for (i=0; i < steps; i++)
	{
		motorImpulse(bendMotorPls);
//		printf("Bended to %.2f angle\n", i/BEND_MOTOR_STEPS_PER_DEGREE);
	}
/*	char lstep[2];
	char langle[2];
	memset(lstep, "\0", sizeof(lstep));
	memset(langle, "\0", sizeof(langle));
	sprintf(lstep, sizeof(i), "%f", i);
	sprintf(langle, sizeof(i), "%f", i/BEND_MOTOR_STEPS_PER_DEGREE);
	file_handler = fopen("position.txt", "w+");
	fputc(lstep, file_handler); //put last step to file
	fputc(langle, file_handler); //put last angle to file
	fclose(file_handler);*/
}
/**/


// void duckPin(bool direction, float wire_thickness)
// {
// 	bcm2835_gpio_write(benderPin, HIGH);
// 	waitmSec(200);
// 	bcm2835_gpio_write(benderPin, LOW);
// 	curDir = !curDir;    //direction reversed for next duck
// }


/*Reading position of bending pin*/
int readCurPos()
{
	char data[2];
	memset(data, "\0", sizeof(data));
	float steps_to_home = 0;
	if (coldStart) //check if system just started
	{
		file_handler = fopen("position.txt", "r");
    		fgets(data, 2, (FILE*)file_handler); // get data from file
		steps_to_home = atof(data[0]); //first element in array have to be steps
		printf("COLD START\nData from file, steps to home: %f, angle to home: %f\n", steps_to_home, atof(data[1]));
		fclose(file_handler);
	}
	else
	{
		if (lastPinPosition  != homePosition)
			steps_to_home = lastPinPosition - homePosition;
		else 
			steps_to_home = homePosition;
		printf("MACHINE IS RUNNING FOR SOME TIME\nCalculated data, steps to home: %.2f, angle to home: %.2f\n", steps_to_home, steps_to_home/BEND_MOTOR_STEPS_PER_DEGREE);
	}
	return steps_to_home;
}
/**/

/*Pin returning function
passes parameters:
1. float value of steps amount to reach home position
2. float value of wire diameter
3. boolean value of in what direction motor should move
*/
void pinReturn(float steps_to_home, float wire_thickness, bool direction)
{
	float steps = 0;
	steps = steps_to_home + wire_thickness/2.0;
	bcm2835_gpio_write(benderPin, HIGH);
	printf("Solenoid is hidden");

	if (direction == ccw)
	{
		printf("Pin is turning in CCW direction\n");
		rotatePin(cw, steps);
	}
	else if (direction == cw)
	{
		printf("Pin is turning in CW direction\n");
		rotatePin(ccw, steps);
	}
	
	bcm2835_gpio_write(benderPin, LOW);
	printf("Solenoid is in up position");
}
/**/

/*Homing routine function*/
void homePosition()
{
	float steps_count = 0;
	float middle_position = 0;
	while (minSwitch == HIGH)
		rotatePin(cw, 1);
	
	printf("Minimum limit switch reached\n");

	while (maxSwitch == HIGH)
	{
		rotatePin(ccw, 1);
		steps_count++;
	}
	printf("Maximum limit switch reached\nTotal amount of steps on scale: %.2f\n", steps_count);

	middle_position = steps_count/2;
	printf("Home position should be on step %.2f\n", middle_position);
	rotatePin(cw, middle_position);
}
	
/*readFile() - function which will open file with saved data about last pin location */
/*checkPinLocationValue() - check if location is home*/
/*countSteps() - count steps to practical home pos. if pin was at home pos, move pin only to set it closly to wire considering thickness
if wasn't move it to home considering wire thickness*/
/*checkDirection() - check next pin movement direction and move pin accordingly to this*/
/*
Runs rotatePin() with some constant number of steps to return it to home position, considering wire thickness and direction
*/

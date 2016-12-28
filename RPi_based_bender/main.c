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

}

void waitmSec(float mseconds);
void motorImpulse(int motor);

void bendWire(float angle);
void feedWire(float lenght);
void rotatePin(bool direction, float steps);
void pinReturn(float steps_to_home, float wire_thickness, bool direction);
void duckPin();
int readCurPos();

int main(int argc, char **argv)
{
	/********
	Runs bendWire() and feedWire() needed amount of time.
	Bending function and feeding function should be linked in future to provide lenght/angle interdependence.
	********/
	setup();

	int i = 0;

	while (i < 1)
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
		printf("Bending %.2f \n", angle);
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
		int steps_from_file = readCurPos();
		pinReturn(steps_from_file, WIRE_THICKNESS, back);
	}
}
/**/


/*WIRE FEEDING FUNCTION*/
void feedWire(float lenght)
{
	int i = 0;
	if (lenght != 0)
	{
		printf("Feeding %.2f \n", lenght);

		float steps = 0;
		steps = lenght * FEED_MOTOR_STEPS_PER_MILIMETER;
		
		printf("Steps %.0f \n", steps);

		bcm2835_gpio_write(feedMotorDir, 1);  //feed motor only moves forward

		for (i=0; i < steps; i++) //rotate feed motor appropriate number of steps 
		{
			motor_impulse(feedMotorPls);
		}
	}
}
/**/


/*PIN ROTATION FUNCTION*/
void rotatePin(bool direction, float steps) //moves bender pin
{
	int i = 0;
	printf("Steps %.0f \n", steps);

	bcm2835_gpio_write(bendMotorDir, direction); //moves bender pin back to home position ready for next feed
	
	for (i=0; i < steps; i++)
	{
		motor_impulse(bendMotorPls);
//		printf("Bended to %.2f angle\n", i/BEND_MOTOR_STEPS_PER_DEGREE);
	}
	
	file_handler = fopen("position.txt", "w+");
	fputc(i, file_handler); //put last step to file
	fputc(i/BEND_MOTOR_STEPS_PER_DEGREE, file_handler); //put last angle to file
	fclose(file_handler);
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
	float data[2];
	memset(data, 0x00, sizeof(data));
	float steps_to_home = 0;
	if (coldStart) //check if system just started
	{
		file_handler = fopen("position.txt", "r");
    	fgets(data, 2, (FILE*)file_handler); // get data from file
		steps_to_home = data[0]; //first element in array have to be steps
		printf("COLD START\nData from file, steps to home: %d, angle to home: %d\n", steps_to_home, data[1])
		fclose(file_handler);
	}
	else
	{
		if (lastPinPosition  != homePosition)
			steps_to_home = lastPinPosition - homePosition;
		else 
			steps_to_home = homePosition;
		printf("MACHINE IS RUNNING FOR SOME TIME\nCalculated data, steps to home: %.2f, angle to home: %.2f\n", steps_to_home, steps_to_home/BEND_MOTOR_STEPS_PER_DEGREE)
	}
	return steps_to_home;
}
/**/

/*Pin returning function
Homing routine*/
void pinReturn(float steps_to_home, float wire_thickness, bool direction)
{
	float steps = 0;
	steps = steps_to_home + wire_thickness/2.0;
	bcm2835_gpio_write(benderPin, HIGH);

	if (direction == ccw)
	{
		printf("Pin is turning in CCW direction");
		rotatePin(cw, steps);
	}
	else if (direction == cw)
	{
		printf("Pin is turning in CW direction");
		rotatePin(cww, steps);
	}
	
	bcm2835_gpio_write(benderPin, LOW);
}
/**/
	
/*readFile() - function which will open file with saved data about last pin location */
/*checkPinLocationValue() - check if location is home*/
/*countSteps() - count steps to practical home pos. if pin was at home pos, move pin only to set it closly to wire considering thickness
if wasn't move it to home considering wire thickness*/
/*checkDirection() - check next pin movement direction and move pin accordingly to this*/
/*
Runs rotatePin() with some constant number of steps to return it to home position, considering wire thickness and direction
*/
#include <bcm2835.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

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
const int bendMotorPls = RPI_GPIO_P1_9;
const int feedMotorPls = RPI_GPIO_P1_11;
const int benderPin = RPI_GPIO_P1_12;

// AWO pins to allow motor shaft to free spin
const int bendMotorAWO = RPI_GPIO_P1_3;
const int feedMotorAWO = RPI_GPIO_P1_5;

// Direction pins to select drive direction
const int bendMotorDir = RPI_GPIO_P1_6;
const int feedMotorDir = RPI_GPIO_P1_8;

//misc program constants
const int pulseWidth = 0.1/1000; //in microseconds
const int pulseDelay = 1/1000; //in microseconds

// Drive directions in english
bool ccw = true; // counter-clockwise drive direction
bool cw = false; // clockwise drive direction
bool curDir = cw; // resets direction before next angle command // pin assignments

bcm2835_gpio_fsel(bendMotorPls, BCM2835_GPIO_FSEL_OUTP);
bcm2835_gpio_fsel(feedMotorPls, BCM2835_GPIO_FSEL_OUTP);
bcm2835_gpio_fsel(benderPin, BCM2835_GPIO_FSEL_OUTP);

bcm2835_gpio_fsel(bendMotorAWO, BCM2835_GPIO_FSEL_OUTP);
bcm2835_gpio_fsel(feedMotorAWO, BCM2835_GPIO_FSEL_OUTP);

bcm2835_gpio_fsel(bendMotorDir, BCM2835_GPIO_FSEL_OUTP);
bcm2835_gpio_fsel(feedMotorDir, BCM2835_GPIO_FSEL_OUTP);

bcm2835_gpio_fsel(benderPin, BCM2835_GPIO_FSEL_OUTP);


void waitSec(float seconds)
{
	clock_t start, diff;
	float msec = 0;
	start = clock();

	while (msec/1000 < seconds)
	{
		diff = clock() - start;
		msec = diff * 1000 / CLOCKS_PER_SEC;
		printf("%.3f seconds %.3f milliseconds\n", msec/1000, msec);
	}
}


void motor_impulse(int motor)
{
	bcm2835_gpio_write(motor, HIGH); //moves bender pin back to home position ready for next feed
	waitSec(pulseWidth);
	bcm2835_gpio_write(motor, LOW); //moves bender pin back to home position ready for next feed
	waitSec(pulseDelay);
}


void bendWire(float angle);
void feedWire(float lenght);
void rotatePin(bool direction, float steps);
void duckPin();

int main()
{
	/********
	Runs bendWire() and feedWire() needed amount of time.
	Bending function and feeding function should be linked in future to provide lenght/angle interdependence.
	********/
}


/*WIRE BENDING FUNCTION
Currently passes only 1 arg - angle, in prospect will pass wire thickness also
*/
void bendWire(float angle)
{
	if (angle != 0)
	{
		printf("Bending %.2f \n", angle);
		bool dir=cw;
		bool back=ccw;
		
		if (angle < 0)
		{
			dir = ccw;
			back = cw;
		}

		float steps = 0;
		angle = abs(angle);
		steps = angle * BEND_MOTOR_STEPS_PER_DEGREE;

		printf("Steps %.0f \n", steps);

		rotatePin(dir, steps); //move bender pin
		waitSec(0.1);
		rotatePin(back, steps); //return bender pin
	}
}
/**/


/*WIRE FEEDING FUNCTION*/
void feedWire(float lenght)
{
	if (lenght != 0)
	{
		printf("Bending %.2f \n", lenght);

		float steps = 0;
		steps = lenght * FEED_MOTOR_STEPS_PER_MILIMETER;
		
		printf("Steps %.0f \n", steps);

		bcm2835_gpio_write(feedMotorDir, 1);  //feed motor only moves forward

		for (int i=0; i < steps; i++) //rotate feed motor appropriate number of steps 
		{
			motor_impulse(feedMotorPls);
		}
	}
}
/**/


/*PIN ROTATION FUNCTION*/
void rotatePin(bool direction, float steps) //moves bender pin
{
	printf("Steps %.0f \n", steps);

	bcm2835_gpio_write(bendMotorDir, direction); //moves bender pin back to home position ready for next feed
	
	for (int i=0; i < steps; i++)
	{
		motor_impulse(bendMotorPls);
		printf("Done %d steps\n", i);
	}
}
/**/

void duckPin()
{
	bcm2835_gpio_write(benderPin, HIGH); 
	waitSec(0.2);

	/***************************************
	Runs rotatePin() with some constant number of steps to return it to home position, considering wire thickness and direction
	***************************************/

 	bcm2835_gpio_write(benderPin, LOW);  //pin down move under wire
	curDir = !curDir;    //direction reversed for next duck
}

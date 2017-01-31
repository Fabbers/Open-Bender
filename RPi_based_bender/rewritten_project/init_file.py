#import RPi.GPIO as GPIO
import time

FEED_MOTOR_STEPS_PER_REVOLUTION = 4000 
FEED_MOTOR_SHAFT_DIAMETER = 30.4
FEED_MOTOR_STEPS_PER_MILIMETER = 41.88   #FEED_MOTOR_STEPS_PER_REVOLUTION / FEED_MOTOR_SHAFT_DIAMETER * PI

BEND_MOTOR_SHAFT_DIAMETER = 10
BEND_MOTOR_SHOULDER = 50
BEND_MOTOR_STEPS_PER_REVOLUTION = 4000 
GEAR2_GEAR1_RATIO = 3
BEND_MOTOR_STEPS_PER_DEGREE = 33.33    #FEED_MOTOR_STEPS_PER_REVOLUTION * GEAR2_GEAR1_RATIO  / 360

WIRE_THICKNESS = 3

CMD_STOP = 127
CMD_FEED = 126
CMD_2D_BEND = 125
CMD_3D_BEND = 124


# pin assignments
# Motor pulse and solenoid pins
bendMotorPls = 9 #RPI_GPIO_P1_9;
feedMotorPls = 11 #RPI_GPIO_P1_11;
benderPin = 12 #RPI_GPIO_P1_12;

# AWO pins to allow motor shaft to free spin
bendMotorAWO = 3 #RPI_GPIO_P1_3;
feedMotorAWO = 5 #RPI_GPIO_P1_5;

# Direction pins to select drive direction
bendMotorDir = 6 #RPI_GPIO_P1_6;
feedMotorDir = 8 #RPI_GPIO_P1_8;

# Limit Switches
minSwitch = 23 #RPI_GPIO_P1_23;
maxSwitch = 24 #RPI_GPIO_P1_24;

# misc program constants
pulseWidth = 0.0001 #100 microseconds
pulseDelay = 0.001 #1000 microseconds

# Drive directions in english
ccw = True # counter-clockwise drive direction
cw = False # clockwise drive direction
curDir = cw; # resets direction before next angle command, pin assignments

coldStart = True

homePosition = 0
lastPinPosition = 0


def setup():
	GPIO.setmode(GPIO.BCM)

	GPIO.setup(bendMotorPls, GPIO.OUT) 
	GPIO.setup(feedMotorPls, GPIO.OUT)
	GPIO.setup(benderPin, GPIO.OUT)

	GPIO.setup(bendMotorAWO, GPIO.OUT) 
	GPIO.setup(feedMotorAWO, GPIO.OUT)
	
	GPIO.setup(bendMotorDir, GPIO.OUT) 
	GPIO.setup(feedMotorDir, GPIO.OUT)
	
	GPIO.setup(minSwitch, GPIO.IN, pull_up_down=GPIO.PUD_UP) 
	GPIO.setup(maxSwitch, GPIO.IN, pull_up_down=GPIO.PUD_UP)

#Alternative timer
def wait(mseconds):
	start_time = time.time()
	#mseconds = mseconds/1000
	while True:
		current_time = time.time()
		#print (float(current_time - start_time))
		if float(current_time - start_time) >= mseconds:
			break

#Makes one motor impulse of specified motor as parameter
def motorImpulse(motor):
	GPIO.output(motor, GPIO.HIGH)
	wait(pulseWidth) 
	GPIO.output(motor, GPIO.LOW)
	wait(pulseDelay)

#Rotates bending to specified amount of steps in specified direction
def rotatePin(direction, steps):
	GPIO.output(bendMotorDir, direction)

	for i in range(int(steps)):
		motorImpulse(bendMotorPls)
		# print "Bended to {} angle".format(i/BEND_MOTOR_STEPS_PER_DEGREE)

#Bends wire for specified angle, could be negative
def bendWire(angle):
	if angle != 0:
		print "Bending to {} degrees".format(angle)
		direction = cw
		reversedir = ccw

		if angle < 0:
			direction = ccw
			reversedir = cw

		steps = 0
		angle = abs(angle)
		steps = angle * BEND_MOTOR_STEPS_PER_DEGREE
		print "Steps {}".format(steps)
		rotatePin(direction, steps)

#Feeds wire for specified length in mm
def feedWire(length):
	if length != 0:
		print "Feeding {}".format(length)
		steps = length * FEED_MOTOR_STEPS_PER_MILIMETER
		print "Steps {}".format(steps)
		GPIO.output(feedMotorDir, 1)

		for i in range(int(steps)):
			motorImpulse(feedMotorPls)

#Returns bending pin to specified as 1st param. amount of steps to home pos., considers wire diameter and direction
def pinReturn(steps_to_home, wire_thickness, direction):
	steps = steps_to_home + BEND_MOTOR_STEPS_PER_DEGREE * (wire_thickness/2.0)
	GPIO.output(benderPin, HIGH)
	print "Solenoid is hidden"

	if direction == ccw:
		print "Pin is turning in CCW direction"
		rotatePin(cw, steps)
	elif direction == cw:
		print "Pin is turning in CW direction"
		rotatePin(ccw, steps)

	GPIO.output(benderPin, LOW)
	print "Solenoid is in up position"

#Homing routine with limiting switches
def homingRoutine():
	steps_count = 0

	while not (GPIO.input(minSwitch) == 0):
		rotatePin(cw, 1)
	
	print "Minimum limit switch reached"

	while not (GPIO.input(maxSwitch) == 0):
		rotatePin(ccw, 1)
		steps_count += 1

	print "Maximum limit switch reached\nTotal amount of steps on scale: {}".format(steps_count)	
	middle_position = steps_count/2.0
	print "Home position should be on step {}".format(middle_position)
	rotatePin(cw, middle_position)

#Temporar cap for reading commands from Web App
def readCommandsFromUI():
	pass

#Creates CSV table with REAL and THEORETICAL angles
#1st column - theoretical, 2nd - real
def anglesTableCreation(theoretical_angle):
	bendWire(theoretical_angle)
	steps_count = 0

	while not readCommandsFromUI() == "reached home":
		if readCommandsFromUI() == "make 1 step"
			bendWire(-1*(theoretical_angle/theoretical_angle))
			steps_count += 1

	real_angle = steps_count/BEND_MOTOR_STEPS_PER_DEGREE

	with open("manual_tuning.csv") as csvfile:
		file = csv.writer(csvfile, delimiter = ',')

		file.writerow([theoretical_angle, real_angle])
		# for content in file:
		# 	carrier = content[0]
		# 	weekday = content[1]




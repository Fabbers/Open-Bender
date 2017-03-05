import RPi.GPIO as GPIO
import time
import csv
import os

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
bendMotorPls = 26 #RPI_GPIO_P1_9;
feedMotorPls = 21 #RPI_GPIO_P1_11;

benderPin = 14 #RPI_GPIO_P1_14;

# AWO pins to allow motor shaft to free spin
bendMotorAWO = 19 #RPI_GPIO_P1_3;
feedMotorAWO = 20 #RPI_GPIO_P1_5;

# Direction pins to select drive direction
bendMotorDir = 13 #RPI_GPIO_P1_6;
feedMotorDir = 16 #RPI_GPIO_P1_8;

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

    GPIO.output(bendMotorAWO, GPIO.HIGH)
    GPIO.output(feedMotorAWO, GPIO.HIGH)


#Alternative timer
def wait(seconds):
    start_time = time.time()
   #mseconds = seconds/1000
    while True:
        current_time = time.time()
        #print (float(current_time - start_time))
        if float(current_time - start_time) >= seconds:
            break


#Makes one motor impulse of specified motor as parameter
def motorImpulse(motor):
    GPIO.output(motor, GPIO.HIGH)
    wait(pulseWidth) 
    #time.sleep(0.0001)
    GPIO.output(motor, GPIO.LOW)
    wait(pulseDelay)
    #time.sleep(0.001)

#Rotates bending to specified amount of steps in specified direction
def rotatePin(direction, steps):
    GPIO.output(bendMotorDir, direction)

    for i in range(int(steps)):
        motorImpulse(bendMotorPls)
        #print "Bended to {} angle".format(i/BEND_MOTOR_STEPS_PER_DEGREE)


#Bends wire for specified angle, could be negative
def bendWire(angle):
    angles_table = open("manual_tuning.csv", "rb")
    file = csv.reader(angles_table, delimiter = ',')

    #dictionary to store all angles from file
    if file == True:
        angles_dict = {}
        for angle in file:
            theoretical_angle = int(angle[0])
            real_angle = int(angle[1])
            angles_dict[theoretical_angle] = real_angle

        bending_angle = angles_dict[angle]
    else:
        bending_angle = angle

    if bending_angle != 0:
        print "Bending to {} degrees".format(bending_angle)
        direction = cw
        reversedir = ccw

        if bending_angle < 0:
            direction = ccw
            reversedir = cw

        steps = 0
        bending_angle = abs(bending_angle)
        steps = bending_angle * BEND_MOTOR_STEPS_PER_DEGREE
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
#1st column - theoretical, 2nd - real, 3-rd - angle's type marker
def anglesTableCreation(theoretical_angle):
    bendWire(theoretical_angle)
    steps_count = 0
    real_angle = 0
    
    current_state = True
    prev_state = True
    
    # while not readCommandsFromUI() == "reached home":
    while not (GPIO.input(minSwitch) == 0):
        current_state = GPIO.input(maxSwitch)
        # if readCommandsFromUI() == "make 1 step"
        if current_state != prev_state:
            if (current_state == 0):
		print "first state check"
                wait(0.2)
                control_current_state = GPIO.input(maxSwitch)
                if control_current_state == 0:
		    print "control state check"
                    if theoretical_angle > 0:
                        bendWire(-0.5)
                    elif theoretical_angle < 0:
                        bendWire(0.5)
                    else: break
                    current_state = control_current_state
                    real_angle += 0.5
                    print "Angle turned back: ", real_angle
        prev_state = current_state
    
    #real_angle = steps_count/BEND_MOTOR_STEPS_PER_DEGREE
    print "Real angle is: ", real_angle

    try:
        csvfile = open("manual_tuning.csv", "r")
	
    except IOError as e:
        csvfile = open("manual_tuning.csv", "w")
        csvfile.close()
        csvfile = open("manual_tuning.csv", "r")

    freader = csv.reader(csvfile, delimiter = ',')        
    
    file_as_list = list(freader)
    csvfile.close()

    rangles_list = []
    tangles_list = []
    
    print "File's lenght: ", len(file_as_list)

    #check if CSV freader is empty
    if len(file_as_list) > 0:
        csvfile = open("manual_tuning.csv")
        freader = csv.reader(csvfile, delimiter=',')

        for content in freader:
            tangle = content[0]
            tangles_list.append(int(tangle))
            print "Theoretical angle: ",tangle

        print "Before sort: ",tangles_list
        tangles_list.sort()
	print "After sort: ",tangles_list
	
        largestTAngle = float(tangles_list[-1]) #largest theoretical angle
    else: 
        largestTAngle = 0

    csvfile.close()
    print "Largest angle in file is: ", largestTAngle
    csvfile = open("manual_tuning.csv", "a")

    fwriter = csv.writer(csvfile, delimiter = ",")

    rangle_in_tangle = real_angle/theoretical_angle
    print "Real angle in theoretical: ", rangle_in_tangle
        
    #CHILD angles filling procedure. procedure runs until we meet meet largest element in theoretical angles list, 
    #what means that it is 'PARENT' angle.
    for angle in range(int(largestTAngle), int(abs(theoretical_angle)+1)):
        if theoretical_angle < 0:
            angle *= -1
        
        if abs(angle) is int(abs(theoretical_angle)):
            fwriter.writerow([angle, angle*rangle_in_tangle, "PARENT"])
        else:
            fwriter.writerow([angle, angle*rangle_in_tangle, "CHILD"])

    csvfile.close()

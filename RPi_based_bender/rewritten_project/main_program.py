from init_file import *
import json


setup()
#anglesTableCreation(30)

#wait(5000)
# homingRoutine()
#with open('json_sample.json') as data_file:    
#    data = json.load(data_file)

#print data
assert (GPIO.LOW) == False
assert (GPIO.LOW) == 0
for i in range(2):
#	feedWire(50)
#	bendWire(-45)
#	wait(1)	
#	feedWire(50)	
#	motorImpulse(bendMotorPls)
	rotatePin(1, 2094)
#wait(5)

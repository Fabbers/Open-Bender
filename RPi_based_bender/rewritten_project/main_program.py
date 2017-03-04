from init_file import *
import json


setup()
#anglesTableCreation(30)

#wait(5000)
# homingRoutine()
#with open('json_sample.json') as data_file:    
#    data = json.load(data_file)

#print data
#assert (GPIO.HIGH) == True
for i in range(2):
	feedWire(50)
	bendWire(45)
#	wait(1)	
#	feedWire(50)	
#	motorImpulse(bendMotorPls)
#	rotatePin(GPIO.LOW, 20)
#wait(5)

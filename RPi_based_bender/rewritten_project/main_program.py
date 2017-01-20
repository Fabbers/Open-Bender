from init_file import *

setup()
#wait(5)
homingRoutine()
for i in range(2):
	feedWire(50)
	wait(0.001)
	bendWire(45)
	wait(0.001)	
	

wait(5)

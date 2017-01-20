from init_file import *


def main():
	wait(500)
	homingRoutine()
	while i < 2:
		feedWire(50)
		wait(1)
		bendWire(45)
		wait(1)
		i += 1		

if __name__ == "main":
	main()

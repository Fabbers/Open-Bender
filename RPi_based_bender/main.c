#include "init.c"

int main(int argc, char **argv)
{
	/********
	Runs bendWire() and feedWire() needed amount of time.
	********/
	//setup();
	//homingRoutine();
	int i = 0;
	waitmSec(500);
	json_value* content = getFileContent(argv[1]);
	printf("%d\n", content);
	//getOffset(content);
	//motorImpulse(bendMotorPls);
	/*while (i < 5)
	{
		feedWire(50);
		waitmSec(1);
		bendWire(45);
		waitmSec(1);
		i++;
	}
	*/

	bcm2835_close();
	return 0;
}


/* Use File->Load Prog to
   load a different Program
*/
#include <Servo.h>
Servo servo; //Create an object
int i = 0;

void setup()
{
	servo.attach(9); //attach pin 9 to servo motor
}

void loop()
{
	for(i=0;i<=180;i++)
	{
		servo.write(i);
		delay(15);
	}
	for(i=180;i>=0;i--)
	{
		servo.write(i);
		delay(15);
	}
}

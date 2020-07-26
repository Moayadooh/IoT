
/* Use File->Load Prog to
   load a different Program
*/

const int buzzer = 9; //set buzzer pin to 9

void setup()
{
	pinMode(buzzer,OUTPUT); //set pin mode as an output
}

void loop()
{
	tone(buzzer,1000); //send 1 KHz sound signal
	delay(1000); //for 1 second
	noTone(buzzer); //stop sound
	delay(1000); //for 1 second
}

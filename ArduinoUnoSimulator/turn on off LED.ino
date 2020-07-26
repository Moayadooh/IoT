
/* Use File->Load Prog to
   load a different Program
*/

int ledpin = 8; //set led pin to number 8

void setup()
{
	pinMode(ledpin,OUTPUT); //set ledpin as an output
}

void loop()
{
	digitalWrite(ledpin,HIGH); //function to tur on LED
	delay(1500); //delay 1.5 second
	digitalWrite(ledpin,LOW); //function to tur off LED
	delay(1500); //delay 1.5 second
}

int button = 5 ; // button connected to pin 5
int LED = 6; // LED connected to pin 6

void setup () {
	pinMode(button, INPUT_PULLUP); 
	// set the digital pin as input with pull-up resistor
	pinMode(LED, OUTPUT); // set the digital pin as output
}

void loop () {
	if (digitalRead(button) == LOW){	// if button pressed
		digitalWrite(LED,HIGH); // turn on led
		delay(500); // delay for 500 ms
		digitalWrite(LED,LOW); // turn off led
		delay(500); // delay for 500 ms
	}
}
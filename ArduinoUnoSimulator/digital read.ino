
int ledpin = 10;
int val = 0;
void setup() {
	Serial.begin(9600);
	pinMode(ledpin, OUTPUT); 
}

void loop() {
	digitalWrite(ledpin, HIGH);
	val = digitalRead(ledpin);
	Serial.println(val);
	delay(1000);
	
	digitalWrite(ledpin, LOW);
	val = digitalRead(ledpin);
	Serial.println(val);
	delay(1000);
}

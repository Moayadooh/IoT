
String str;
int Temperature = 45;
void setup()
{
	Serial.begin(9600);
	Serial.println("Temperature Levels:");
}

void loop(){
	str = Serial.readString();
	if (str.length() > 0) {
		Temperature = str.toInt();
		Serial.print("Temperature = ");
		Serial.print(Temperature);
		Serial.println(" degrees Celsius");
	}
}

#include <ESP8266WiFi.h>
//#include <WiFiClient.h> 
//#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "";    
const char* password = "";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  WiFi.mode(WIFI_OFF); //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA); //This line hides the viewing of ESP as wifi hotspot
  WiFi.begin(ssid, password); //Connect to your WiFi router
  
  Serial.begin(9600);
  Serial.println("");
  Serial.print("Connecting");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, LOW);
    delay(250);
    Serial.print(".");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
  }
  
  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); //IP address assigned to your ESP
}

void loop() {
  double temperature = 40.50;
  double soil_moisture = 20.22;
  
  HTTPClient http; //Declare object of class HTTPClient
  http.begin("http://192.168.8.107/Nodemcu/InsertDB.php"); //Specify request destination
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); //Specify content-type header
  
  String data = "temp="+String(temperature) +"&soil_mo="+String(soil_moisture) +"&insert="+1;
  int httpCode = http.POST(data); //Send the request
  String payload = http.getString(); //Get the response payload
  
  int n = payload.length(); 
  char json[n+1]; 
  strcpy(json, payload.c_str()); 
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    //Serial.print(F("deserializeJson() failed: "));
    //Serial.println(error.c_str());
    Serial.println("Record not inserted");
    Serial.println(payload);
  }
  else
  {
    Serial.println("Record inserted");
    //Serial.print("http return code: ");
    //Serial.println(httpCode);
    double a = doc["temp"];
    double b = doc["soil_mo"];
    String c = doc["date_time"];
    Serial.print("Temperature is: ");
    Serial.println(a);
    Serial.print("Soil Moisture is: ");
    Serial.println(b);
    Serial.print("Date & Time is: ");
    Serial.println(c);
  }
  http.end(); //Close connection
  delay(1000000);
}

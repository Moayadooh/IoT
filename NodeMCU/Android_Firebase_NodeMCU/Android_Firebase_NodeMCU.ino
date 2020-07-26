#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define FIREBASE_HOST "iotfirebaseproject-d6670.firebaseio.com"
#define FIREBASE_AUTH ""

#define FIREBASE_FCM_SERVER_KEY ""
#define FIREBASE_FCM_DEVICE_TOKEN_1 ""
#define FIREBASE_FCM_DEVICE_TOKEN_2 ""

FirebaseData firebaseData;
void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  WiFi.mode(WIFI_OFF); //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA); //This line hides the viewing of ESP as wifi hotspot
  
  // connect to wifi.
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  Firebase.setInt(firebaseData, "STATUS", 0);
  Firebase.setBool(firebaseData, "FLAG", false);

  sendMessage();
}

int n;
bool x;
void loop() {
  Firebase.getInt(firebaseData, "STATUS", n);
  Firebase.getBool(firebaseData, "FLAG", x);
  if (n==1) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else{
    digitalWrite(LED_BUILTIN, HIGH);
  }
  if(x){
    Display();
    Firebase.setBool(firebaseData, "FLAG", false);
  }
}

void Display(){
	String path = "INFO";
	String Name;
  int age;
  Firebase.getString(firebaseData, path+"/Name", Name);
  Firebase.getInt(firebaseData, path+"/Age", age);
	Serial.println(Name);
	Serial.println(age);
}

int count = 0;
void sendMessage()
{
    firebaseData.setBSSLBufferSize(1024, 1024);
    firebaseData.setResponseSize(1024);
    firebaseData.fcm.begin(FIREBASE_FCM_SERVER_KEY);
    firebaseData.fcm.addDeviceToken(FIREBASE_FCM_DEVICE_TOKEN_1);
    //firebaseData.fcm.addDeviceToken(FIREBASE_FCM_DEVICE_TOKEN_2);
    firebaseData.fcm.setPriority("high");
    firebaseData.fcm.setTimeToLive(1000);
    
    Serial.println("------------------------------------");
    Serial.println("Send Firebase Cloud Messaging...");

    firebaseData.fcm.setNotifyMessage("Notification", "The watering system is running...");
    firebaseData.fcm.setDataMessage("{\"myData\":" + String(count) + "}");

    if (Firebase.sendMessage(firebaseData, 0))
    {
        Serial.println("PASSED");
        Serial.println(firebaseData.fcm.getSendResult());
        Serial.println("------------------------------------");
        Serial.println();
    }
    else
    {
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason());
        Serial.println("------------------------------------");
        Serial.println();
    }
    count++;
}

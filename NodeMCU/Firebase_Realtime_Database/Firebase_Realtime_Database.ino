#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""
#define FIREBASE_HOST "iotfirebaseproject-d6670.firebaseio.com"
#define FIREBASE_AUTH ""

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;
String key;
void setup()
{
  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println();
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(firebaseData, 1000 * 60);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(firebaseData, "tiny");

  
  String path = "INFO";
  
  Firebase.setString(firebaseData, path+"/Name", "Moayad");
  Firebase.setInt(firebaseData, path+"/Age", 23);
  //Firebase.pushInt(firebaseData, path+"/Age", 55);
  //Firebase.pushJSON(firebaseData, path, json);
  //key = firebaseData.pushName();
  
  Firebase.deleteNode(firebaseData, "Test");
  
  String Name;
  Firebase.getString(firebaseData, path+"/Name", Name);
  int age;
  Firebase.getInt(firebaseData, path+"/Age", age);
  
  Serial.println(Name);
  Serial.println(age);

  Serial.println("PATH: " + firebaseData.dataPath());
  Serial.println("TYPE: " + firebaseData.dataType());
  Serial.println("ETag: " + firebaseData.ETag());

  /*if(Firebase.pathExist(firebaseData, "System Status")){
    Serial.println("is set");
  }else{
    Serial.println("is not");
  }*/
  
}

void loop()
{
}

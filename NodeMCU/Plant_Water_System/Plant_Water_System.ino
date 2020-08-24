#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Fuzzy.h>

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
#define FIREBASE_FCM_SERVER_KEY ""
#define ROOT_URL ""
#define HOST_NAME ""
#define USERNAME ""
#define PASSWORD ""
#define DB_NAME ""

FirebaseData firebaseData;
HTTPClient http;

bool connectionRequest = false, disconnectionRequest = false, Connected = false, tempSensorRunning = false, soilSensorRunning = false, systemStoped = false, sensorFail = false, recordsAvailable = false;
String wifiSSID, wifiKey, randomNumber, verificationKey, fcmDeviceToken, systemStatus, path = "Watering Operations Records", str, strDay = "", strMonth = "", strYear = "", dateTime;
float temperature = 0, soilMoisture = 0, waterAmount, value;
int plantID, day, month, year, count = 0, i, j, size;

//One Second = 1UL * 1000UL
//One Minute = 1UL * 60UL * 1000UL
//One Hour = 1UL * 60UL * 60UL * 1000UL
//One Day = 1UL * 24UL * 60UL * 60UL * 1000UL
unsigned long millisTimer;
unsigned long startMillisTimer = 0UL;
unsigned long startMillisWateringTime = 0UL;
unsigned long startMillisPlantAge = millis();

//Connect to WiFi
void WifiConnection()
{
  Serial.print("WiFi SSID: ");
  while(true)
  {
    if(Serial.available())
    {
      wifiSSID = Serial.readString();
      break;
    }
  }
  Serial.println(wifiSSID);
  Serial.println("WiFi Key: ");
  while(true)
  {
    if(Serial.available())
    {
      wifiKey = Serial.readString();
      break;
    }
  }
  Serial.print("Connecting to ");
  Serial.println(wifiSSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID, wifiKey);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//Connect to Firebase
void FirebaseConnection()
{
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  Firebase.setReadTimeout(firebaseData, 1000 * 60);  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setwriteSizeLimit(firebaseData, "unlimited");  //tiny, small, medium, large and unlimited. Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  firebaseData.setBSSLBufferSize(2048, 2048);  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(2048);  //Set the size of HTTP response buffer. Minimum size is 400 bytes
}

//Stop watering operation initially
void StopWateringOperation()
{
  Serial.println("*************************************************************");
  if(Firebase.setString(firebaseData, "System Status", "Off"))
    Serial.println("System is off");
  else
    Serial.println(firebaseData.errorReason());
  Serial.println("*************************************************************");
}

//Generate verification key
void GenerateVerificationKey()
{
  Serial.println();
  Serial.println("*************************************************************");
  srand(time(0)); 
  for(i=0;i<5;i++)
  {
    int num = rand() % 10;
    randomNumber += " "+String(num);
  }
  Serial.print("Verification Key:");
  Serial.println(randomNumber);
  Serial.println("*************************************************************");
}

//Connect the mobile application
void ConnectMobileApp()
{
  if(Firebase.getBool(firebaseData, "Connection Request", connectionRequest))
  {
    if(connectionRequest)
    {
      Serial.println("\n\n*************************************************************");
      Serial.println("Connetion request is received");
      connectionRequest = false;
      Firebase.setBool(firebaseData, "Connection Request", false);
      if(Firebase.getString(firebaseData, "Verification Key", verificationKey))
      {
        Serial.println("-------------------------------------------------------------");
        if(verificationKey == randomNumber)
        {
          Firebase.setString(firebaseData, "Connection Status", "Connected");
          Serial.println("You are now connected");
        }
        else
        {
          Serial.println("The entered key is invalid");
          Firebase.setBool(firebaseData, "Invalid Key", true);
        }
        Serial.println("*************************************************************");
      }
      else
        Serial.println(firebaseData.errorReason());
      Firebase.setBool(firebaseData, "Connection Request", false);
    }
  }
}

//Disconnect the mobile application
void DiconnectMobileApp()
{
  if(Firebase.getBool(firebaseData, "Disconnection Request", disconnectionRequest))
  {
    if(disconnectionRequest)
    {
      Serial.println("\n\n*************************************************************");
      disconnectionRequest = false;
      if(Firebase.setBool(firebaseData, "Disconnection Request", false))
      {
        if(Firebase.setString(firebaseData, "Connection Status", "Not Connected"))
        {
          Serial.println("The application is disconnected");
          Serial.println("*************************************************************");
          Firebase.deleteNode(firebaseData, "FCM Device Token");
        }
        else
          Serial.println(firebaseData.errorReason());
      }
      else
        Serial.println(firebaseData.errorReason());
      Firebase.setBool(firebaseData, "Disconnection Request", false);
    }
  }
}

//Check and update plant age
void PlantAgeUpdate()
{
  if(Firebase.getInt(firebaseData, "Plant ID", plantID))
  {
    millisTimer = 1UL * 24UL * 60UL * 60UL * 1000UL;
    if(millis() - startMillisPlantAge >= millisTimer)
    {
      http.begin(ROOT_URL);
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      
      String data = "hostname="+String(HOST_NAME) +"&username="+String(USERNAME) +"&password="+String(PASSWORD) +"&dbname="+String(DB_NAME) +"&plant_id="+String(plantID) +"&CHECK_PLANT_AGE_UPDATE_DATE=1";
      http.POST(data);
      String payload = http.getString();
      http.end();
      
      if(payload=="Plant Age Updated")
        sendMessage(payload);
      startMillisPlantAge = millis();
    }
  }
}

//Get temperature value
float getTemperature()
{
  Serial.println("\n\n*************************************************************");
  Serial.print("Temperature: ");
  str = "0";
  startMillisTimer = millis();
  while(true)
  {
    Firebase.getString(firebaseData, "System Status", systemStatus);
    Firebase.getBool(firebaseData, "Connection Request", connectionRequest);
    Firebase.getBool(firebaseData, "Disconnection Request", disconnectionRequest);
    if(systemStatus=="Off" || connectionRequest || disconnectionRequest)
      break;
    if(Serial.available())
    {
      str = Serial.readString();
      Firebase.setBool(firebaseData, "Temperature Sensor Status", true);
      tempSensorRunning = true;
      break;
    }
    millisTimer = 10UL * 1000UL;
  	if(millis() - startMillisTimer >= millisTimer)
  	{
  	  Firebase.setBool(firebaseData, "Temperature Sensor Status", false);
  	  sendMessage("Temperature sensor is not available/working");
  	  tempSensorRunning = false;
  		break;
  	}
  }
  value = str.toFloat();
  return value;
}

//Get soil moisture value
float getSoilMoisture()
{
  Serial.print("Soil Moisture: ");
  str = "0";
  startMillisTimer = millis();
  while(true)
  {
    Firebase.getString(firebaseData, "System Status", systemStatus);
    Firebase.getBool(firebaseData, "Connection Request", connectionRequest);
    Firebase.getBool(firebaseData, "Disconnection Request", disconnectionRequest);
    if(systemStatus=="Off" || connectionRequest || disconnectionRequest)
      break;
    if(Serial.available())
    {
      str = Serial.readString();
      Firebase.setBool(firebaseData, "Soil Moisture Sensor Status", true);
      soilSensorRunning = true;
      break;
    }
    millisTimer = 10UL * 1000UL;
  	if(millis() - startMillisTimer >= millisTimer || str.toFloat() <= -1000)
  	{
  	  Firebase.setBool(firebaseData, "Soil Moisture Sensor Status", false);
  	  sendMessage("Soil Moisture sensor is not available/working");
  	  soilSensorRunning = false;
  		break;
  	}
  }
  value = str.toFloat();
  return value;
}

//Perform watering operations
void WateringOperatin()
{
  Firebase.getInt(firebaseData, "Plant ID", plantID);
	millisTimer = 3UL * 1000UL;
  if(millis() - startMillisWateringTime >= millisTimer) //Check the watering time
  {
    temperature = getTemperature(); //Get temperature value
    if(!tempSensorRunning || systemStatus=="Off" || connectionRequest || disconnectionRequest)
      Serial.println("NONE");
    else
    {
      Serial.print(temperature);
      Serial.println("°C");
    }

    soilMoisture = getSoilMoisture(); //Get soil moisture value
    if(!soilSensorRunning || systemStatus=="Off" || connectionRequest || disconnectionRequest)
      Serial.println("NONE");
    else
    {
      Serial.print(soilMoisture);
      Serial.println("%");
    }

    if(systemStatus!="Off" && !connectionRequest && !disconnectionRequest)
    {
      recordsAvailable = CheckRecords(); //Check if there is records in the trends table
      sensorFail = CheckSensorsStatus(); //Check sensors status
      if(!recordsAvailable && sensorFail)
      {
        Serial.println("-------------------------------------------------------------");
        Serial.println("There is no record in the database!");
        startMillisWateringTime = millis();
        PrintVerificationKey();
        return;
      }
      if(!sensorFail)
        waterAmount = ComputeWaterAmount("COMPUTE_WATER_AMOUNT_CBR");
      else
        waterAmount = ComputeWaterAmount("COMPUTE_WATER_AMOUNT_MLR");
      Serial.print("Water Amount: ");
      Serial.print(waterAmount);
      Serial.println(" Liter");
      PrintVerificationKey();
      SendWateringRecord();
    }
    startMillisWateringTime = millis();
  }
}

//Print verification key
void PrintVerificationKey()
{
  Serial.println("-------------------------------------------------------------");
  Serial.print("Verification Key:");
  Serial.println(randomNumber);
  Serial.println("*************************************************************");
}

//Check if there is records in the trends table
bool CheckRecords()
{
  http.begin(ROOT_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String data = "hostname="+String(HOST_NAME) +"&username="+String(USERNAME) +"&password="+String(PASSWORD) +"&dbname="+String(DB_NAME) +"&CHECK_RECORDS_AVAILABILITY=1";
  http.POST(data);
  String payload = http.getString();
  http.end();

  if(payload=="Records are Available")
    return true;
  else
    return false;
}

//Check sensors status
bool CheckSensorsStatus()
{
  if(!tempSensorRunning || !soilSensorRunning)
    return true;
  else
    return false;
}

//Send watering record as notification to the mobile app
void SendWateringRecord()
{
  if(sensorFail)
    str = "Temperature: NONE\nSoil Moisture: NONE\nWater Amount: "+String(waterAmount);
  else
    str = "Temperature: "+String(temperature)+"\nSoil Moisture: "+String(soilMoisture)+"\nWater Amount: "+String(waterAmount);
  sendMessage(str);
}

//Compute water amount based on the CBR or MLR technique
float ComputeWaterAmount(String technique)
{
  http.begin(ROOT_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String data = "temp_level="+String(temperature) +"&soil_mois_level="+String(soilMoisture) +"&plant_id="+String(plantID) +"&"+String(technique) +"=1"
   +"&hostname="+String(HOST_NAME) +"&username="+String(USERNAME) +"&password="+String(PASSWORD) +"&dbname="+String(DB_NAME);
  http.POST(data);
  String payload = http.getString();
  http.end();

  size = payload.length(); 
  char json[size+1]; 
  strcpy(json, payload.c_str());
  StaticJsonDocument<20000> doc;
  DeserializationError error = deserializeJson(doc, json);
  if(error)
  {
    Serial.println("-------------------------------------------------------------");
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    Serial.print("Mysql connectioin error: ");
    Serial.println(payload);
    return -1;
  }
  Firebase.setBool(firebaseData, "Records Updated", true);
  return doc["water_amount"];
  doc.clear();
}

//Send notification
void sendMessage(String message)
{
  if(Firebase.getString(firebaseData, "FCM Device Token", fcmDeviceToken))
  {
    firebaseData.fcm.begin(FIREBASE_FCM_SERVER_KEY);
    firebaseData.fcm.addDeviceToken(fcmDeviceToken);
    firebaseData.fcm.setPriority("high");
    firebaseData.fcm.setTimeToLive(1000);
    firebaseData.fcm.setNotifyMessage("Plant Water System", message);
    firebaseData.fcm.setDataMessage("{\"myData\":" + String(count) + "}");
    Firebase.sendMessage(firebaseData, 0);
    firebaseData.fcm.clearDeviceToken();
    firebaseData.fcm.clearNotifyMessage();
    firebaseData.fcm.clearDataMessage();
    count++;
  }
}

//Check system status (on/off)
void SystemStatus()
{
  if(Firebase.getString(firebaseData, "System Status", systemStatus))
  {
    Firebase.getBool(firebaseData, "System Stopped", systemStoped);
    if(systemStatus == "On")
      WateringOperatin();
    else if(systemStoped)
    {
      Serial.println("\n\n*************************************************************");
      Serial.println("System is off");
      Serial.println("*************************************************************");
      systemStoped = false;
      Firebase.setBool(firebaseData, "System Stopped", false);
    }
  }
  else
  {
    if(!Firebase.setString(firebaseData, "System Status", "Off"))
      Serial.println(firebaseData.errorReason());
  }
}

void setup()
{
  Serial.begin(74880);
  Serial.println();
  Serial.println();
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  WifiConnection();          //Connect to Wi-Fi
  FirebaseConnection();      //Connect to Firebase
  StopWateringOperation();   //Stop watering operation initially
  GenerateVerificationKey(); //Generate the verification key
}

void loop()
{
  ConnectMobileApp();   //Recieve connection request from the mobile application
  DiconnectMobileApp(); //Recieve disconnection request from the mobile application
  PlantAgeUpdate();     //Check and update plant age
  SystemStatus();       //Check system status (on/off)
  firebaseData.clear();
}

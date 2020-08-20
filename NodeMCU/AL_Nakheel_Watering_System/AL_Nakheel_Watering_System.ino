#include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Fuzzy.h>
#include <time.h>
#include <sys/time.h>

#define FIREBASE_HOST ""
#define FIREBASE_AUTH ""
#define FIREBASE_FCM_SERVER_KEY ""
#define ROOT_URL ""
#define HOST_NAME ""
#define USERNAME ""
#define PASSWORD ""
#define DB_NAME ""
#define TZ       -5  //(utc+) TZ in hours
#define DST_MN   60 //use 60mn for summer time in some countries
#define TZ_SEC  ((TZ)*3600)
#define DST_SEC ((DST_MN)*60)

FirebaseData firebaseData;
HTTPClient http;
time_t now;

bool connectionRequest = false, disconnectionRequest = false, Connected = false, tempSensorRunning = false, soilSensorRunning = false, palmAgeUpdated = false, systemStoped = false;
String wifiSSID, wifiKey, randomNumber, verificationKey, fcmDeviceToken, systemStatus, path = "Watering Operations Records", str, strDay = "", strMonth = "", strYear = "", dateTime;
float temperature = 0, soilMoisture = 0, waterAmount, value;
int palmAge, day, month, year, count = 0, i, j, size;

//One Day = 1UL * 24UL * 60UL * 60UL * 1000UL;
unsigned long startMillisWateringTime = 0UL;
unsigned long numOfSeconds;

unsigned long startMillisTimer = 0UL;
unsigned long oneMinute = 1UL * 60UL * 1000UL;

unsigned long startMillisPalmAge = 0UL;
unsigned long fiveMinute = 5UL * 60UL * 1000UL;

//Create Fuzzy Object
Fuzzy *fuzzy = new Fuzzy();

//Setting the membership value ranges for temperature
FuzzySet *lowTemperature = new FuzzySet(0, 0, 10, 25);
FuzzySet *mediumTemperature = new FuzzySet(15, 25, 25, 35);
FuzzySet *highTemperature = new FuzzySet(25, 40, 50, 50);

//Setting the membership value ranges for soil moisture
FuzzySet *lowSoilMoisture = new FuzzySet(0, 0, 20, 50);
FuzzySet *mediumSoilMoisture = new FuzzySet(30, 50, 50, 70);
FuzzySet *highSoilMoisture = new FuzzySet(50, 80, 100, 100);

//Setting the membership value ranges for water amount
FuzzySet *minimum = new FuzzySet(0, 0, 14, 35);
FuzzySet *veryless = new FuzzySet(14, 35, 35, 59);
FuzzySet *less = new FuzzySet(35, 59, 59, 86);
FuzzySet *littleless = new FuzzySet(59, 86, 86, 113);
FuzzySet *medium = new FuzzySet(86, 113, 113, 140);
FuzzySet *littlemore = new FuzzySet(113, 140, 140, 168);
FuzzySet *more = new FuzzySet(140, 168, 168, 195);
FuzzySet *muchmore = new FuzzySet(168, 195, 195, 222);
FuzzySet *maximum = new FuzzySet(195, 222, 250, 250);

//Building Fuzzy rule function
void BuildingFuzzyRule(FuzzySet *condition1, FuzzySet *condition2, FuzzySet *result, int ruleNumber)
{
  FuzzyRuleAntecedent *fuzzyRuleAntecedent = new FuzzyRuleAntecedent();
  fuzzyRuleAntecedent->joinWithAND(condition1, condition2);

  FuzzyRuleConsequent *fuzzyRuleConsequent = new FuzzyRuleConsequent();
  fuzzyRuleConsequent->addOutput(result);

  FuzzyRule *fuzzyRule = new FuzzyRule(ruleNumber, fuzzyRuleAntecedent, fuzzyRuleConsequent);
  fuzzy->addFuzzyRule(fuzzyRule);
}

//Create Fuzzy input and output
void CreateFuzzyIO()
{
  //Create temperature membership function as Input
  FuzzyInput *temperature = new FuzzyInput(1);
  temperature->addFuzzySet(lowTemperature);
  temperature->addFuzzySet(mediumTemperature);
  temperature->addFuzzySet(highTemperature);
  fuzzy->addFuzzyInput(temperature);

  //Create soil moisture membership function as Input
  FuzzyInput *soilMoisture = new FuzzyInput(2);
  soilMoisture->addFuzzySet(lowSoilMoisture);
  soilMoisture->addFuzzySet(mediumSoilMoisture);
  soilMoisture->addFuzzySet(highSoilMoisture);
  fuzzy->addFuzzyInput(soilMoisture);

  //Create water amount membership function as Output
  FuzzyOutput *waterAmount = new FuzzyOutput(1);
  waterAmount->addFuzzySet(minimum);
  waterAmount->addFuzzySet(veryless);
  waterAmount->addFuzzySet(less);
  waterAmount->addFuzzySet(littleless);
  waterAmount->addFuzzySet(medium);
  waterAmount->addFuzzySet(littlemore);
  waterAmount->addFuzzySet(more);
  waterAmount->addFuzzySet(muchmore);
  waterAmount->addFuzzySet(maximum);
  fuzzy->addFuzzyOutput(waterAmount);

  //Building Fuzzy rules
  BuildingFuzzyRule(highTemperature, highSoilMoisture, less, 1);
  BuildingFuzzyRule(mediumTemperature, highSoilMoisture, veryless, 2);
  BuildingFuzzyRule(lowTemperature, highSoilMoisture, minimum, 3);
  BuildingFuzzyRule(highTemperature, mediumSoilMoisture, littlemore, 4);
  BuildingFuzzyRule(highTemperature, lowSoilMoisture, maximum, 5);
  BuildingFuzzyRule(mediumTemperature, lowSoilMoisture, muchmore, 6);
  BuildingFuzzyRule(mediumTemperature, mediumSoilMoisture, medium, 7);
  BuildingFuzzyRule(lowTemperature, mediumSoilMoisture, littleless, 8);
  BuildingFuzzyRule(lowTemperature, lowSoilMoisture, more, 9);
}

//Compute water amount "Fuzzyfication & Defuzzyfication"
float ComputeWaterAmount(float temp, float soiMoi)
{
  //Fuzzyfication
  fuzzy->setInput(1, temp);
  fuzzy->setInput(2, soiMoi);
  fuzzy->fuzzify();
  
  //Defuzzyfication
  return fuzzy->defuzzify(1);
}

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

//Turn off the system initially
void SetSystemStatusOff()
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
          sendMessage("You are now connected");
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

//Display palm age
void DisplayPalmAge()
{
  Serial.println("\n\n*************************************************************");
  palmAgeUpdated = false;
  Firebase.setBool(firebaseData, "Palm Age Updated", false);
  if(Firebase.getInt(firebaseData, "Palm Age", palmAge))
  {
    Serial.print("Palms age is updated to ");
    Serial.print(palmAge);
    Serial.println(" years old");
    Firebase.getString(firebaseData, "Palm Age Update Date", dateTime);
    Serial.print("(Auto update on ");
    Serial.print(dateTime);
    Serial.println(")");
    Serial.println("*************************************************************");
  }
  else
    Serial.println(firebaseData.errorReason());
}

//Check next update date for palm age
void PalmAgeUpdate()
{
  //Set update date of palms age
  if(Firebase.getBool(firebaseData, "Palm Age Updated", palmAgeUpdated))
  {
    if(palmAgeUpdated)
    {
      Firebase.setString(firebaseData, "Palm Age Update Date", String(GetDay())+"/"+String(GetMonth())+"/"+String((GetYear()+1)));
      DisplayPalmAge();
      startMillisPalmAge = millis();
    }
  }
  if(startMillisPalmAge != 0UL)
  {
    if(millis() - startMillisPalmAge >= fiveMinute)
    {
      if(Firebase.getString(firebaseData, "Palm Age Update Date", dateTime))
      {
        for(i=0;i<dateTime.length();i++)
        {
          if(isdigit(dateTime[i]))
            strDay += dateTime[i];
          else
            break;
        }
        for(i=i+1;i<dateTime.length();i++)
        {
          if(isdigit(dateTime[i]))
            strMonth += dateTime[i];
          else
            break;
        }
        for(i=i+1;i<dateTime.length();i++)
        {
          if(isdigit(dateTime[i]))
            strYear += dateTime[i];
          else
            break;
        }
        day = strDay.toInt();
        month = strMonth.toInt();
        year = strYear.toInt();
        strDay = "";
        strMonth = "";
        strYear = "";
        //if(day==GetDay() && month==GetMonth() && year==GetYear())
        //{
          if(Firebase.getInt(firebaseData, "Palm Age", palmAge))
          {
            Firebase.setInt(firebaseData, "Palm Age", (palmAge+1));
            //Firebase.setString(firebaseData, "Palm Age Update Date", String(GetDay())+"/"+String(GetMonth())+"/"+String((GetYear()+1)));
            Firebase.setString(firebaseData, "Palm Age Update Date", String(day)+"/"+String(month)+"/"+String(year+1)); //For demonstration purpose
            DisplayPalmAge();
          }
          else
            Serial.println(firebaseData.errorReason());
        //}
        str = "Palm age updated to " + String(palmAge) + " years old";
        sendMessage(str);
        startMillisPalmAge = millis();
      }
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
      Firebase.setString(firebaseData, "Temperature Sensor Status", "Running");
      tempSensorRunning = true;
      break;
    }
  	if(millis() - startMillisTimer >= oneMinute)
  	{
  	  Firebase.setString(firebaseData, "Temperature Sensor Status", "Not Running");
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
      Firebase.setString(firebaseData, "Soil Moisture Sensor Status", "Running");
      soilSensorRunning = true;
      break;
    }
  	if(millis() - startMillisTimer >= oneMinute || str.toFloat() <= -1000)
  	{
  	  Firebase.setString(firebaseData, "Soil Moisture Sensor Status", "Not Running");
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
  //Get time of next watering operation based on palm age
  Firebase.getInt(firebaseData, "Palm Age", palmAge);
  if(palmAge <= 4)
    numOfSeconds = 30UL * 1000UL;
  else
    numOfSeconds = 70UL * 1000UL;
  
  if(millis() - startMillisWateringTime >= numOfSeconds) //Check the watering time
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
    {
      Serial.println("NONE");
      Serial.println("*************************************************************");
    }
    else
    {
      Serial.print(soilMoisture);
      Serial.println("%");
    }

    if(systemStatus!="Off" && !connectionRequest && !disconnectionRequest)
    {
      int returnValue = CheckSensorsStatus(); //Check sensors status
      if(returnValue==0)
        return;
  
      //Compute water amount
      waterAmount = ComputeWaterAmount(temperature, soilMoisture);
      Serial.print("Water Amount: ");
      Serial.print(waterAmount);
      Serial.println(" Liter");
  
      dateTime = GetDateTime(); //Get the current date & time
      Serial.println(dateTime);
  
      PrintSensorsStatus();
      SendWateringRecord();
    }
    startMillisWateringTime = millis();
  }
}

//Check sensors status
int CheckSensorsStatus()
{
  //Retrieve the data from database if one or both sensors are not running
  if(!tempSensorRunning || !soilSensorRunning)
  {
    int returnValue = RetrieveData();
    if(returnValue==0)
    {
      startMillisWateringTime = millis();
      return 0;
    }
  }
  return 1;
}

//Send watering record
void SendWateringRecord()
{
  str = "Temperature: "+String(temperature)+"\nSoil Moisture: "+String(soilMoisture)+"\nWater Amount: "+String(waterAmount)+"\nDate & Time: "+dateTime;
  sendMessage(str);
}

//Print sensors status
void PrintSensorsStatus()
{
  Serial.println("-------------------------------------------------------------");
  if(tempSensorRunning)
    Serial.println("Temperature Sensor Status: RUNNING");
  else
    Serial.println("Temperature Sensor Status: NOT RUNNING");
  if(soilSensorRunning)
    Serial.println("Soil Moisture Sensor Status: RUNNING");
  else
    Serial.println("Soil Moisture Sensor Status: NOT RUNNING");
  if(tempSensorRunning && soilSensorRunning)
    StoreData();
  Serial.println("-------------------------------------------------------------");
  Serial.print("Verification Key:");
  Serial.println(randomNumber);
  Serial.println("*************************************************************");
}

//Store temperature, soil moisture, water amount, and date & time in the database
void StoreData()
{
  http.begin(ROOT_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String data = "temp_level="+String(temperature) +"&soil_mois_level="+String(soilMoisture) +"&water_amount="+String(waterAmount) +"&insert=1"
   +"&hostname="+String(HOST_NAME) +"&username="+String(USERNAME) +"&password="+String(PASSWORD) +"&dbname="+String(DB_NAME);
  http.POST(data);
  String payload = http.getString();
  http.end();

  if(payload=="Record Inserted")
    Firebase.setBool(firebaseData, "Records Updated", true);
  else
  {
    Serial.println("-------------------------------------------------------------");
    Serial.print("Mysql connectioin error: ");
    Serial.println(payload);
  }
}

//Retrieve average of last 5 records from database
int RetrieveData()
{
  http.begin(ROOT_URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  String data = "hostname="+String(HOST_NAME) +"&username="+String(USERNAME) +"&password="+String(PASSWORD) +"&dbname="+String(DB_NAME) +"&retrieve=1";
  http.POST(data);
  String payload = http.getString();
  http.end();

  if(payload=="No Records")
  {
    Serial.println("-------------------------------------------------------------");
    Serial.println("There are no records in the database!");
    PrintSensorsStatus();
    return 0;
  }
  
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
  }
  else
  {
    temperature = doc["temp_level"];
    soilMoisture = doc["soil_mois_level"];
    Serial.println("-------------------------------------------------------------");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
    Serial.print("Soil Moisture: ");
    Serial.print(soilMoisture);
    Serial.println("%");
  }
  doc.clear();
  return 1;
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
    firebaseData.fcm.setNotifyMessage("AL-Nakheel Watering System", message);
    firebaseData.fcm.setDataMessage("{\"myData\":" + String(count) + "}");
    Firebase.sendMessage(firebaseData, 0);
    firebaseData.fcm.clearDeviceToken();
    firebaseData.fcm.clearNotifyMessage();
    firebaseData.fcm.clearDataMessage();
    count++;
  }
}

//Get current date and time
String GetDateTime()
{
  now = time(nullptr);
  dateTime = ctime(&now);
  char c[dateTime.length()];
  j = 0;
  str = "";
  for(i=0;i<sizeof(c);i++)
  {
    c[i] = dateTime[i];
    if(j<24)
    {
      str += c[i];
      j++;
    }
  }
  return str;
}

//Get current day
int GetDay()
{
  dateTime = GetDateTime();
  strDay += dateTime[8];
  for(i=9;i<dateTime.length();i++)
  {
    if(isdigit(dateTime[i]))
      strDay += dateTime[i];
    else
      break;
  }
  day = strDay.toInt();
  strDay = "";
  return day;
}

//Get current month
int GetMonth()
{
  dateTime = GetDateTime();
  for(i=4;i<dateTime.length();i++)
  {
    if(dateTime[i]==' ')
      break;
    else
      strMonth += dateTime[i];
  }
  if(strMonth=="Jan")
    month = 1;
  else if(strMonth=="Feb")
    month = 2;
  else if(strMonth=="Mar")
    month = 3;
  else if(strMonth=="Apr")
    month = 4;
  else if(strMonth=="May")
    month = 5;
  else if(strMonth=="Jun")
    month = 6;
  else if(strMonth=="Jul")
    month = 7;
  else if(strMonth=="Aug")
    month = 8;
  else if(strMonth=="Sep")
    month = 9;
  else if(strMonth=="Oct")
    month = 10;
  else if(strMonth=="Nov")
    month = 11;
  else if(strMonth=="Dec")
    month = 12;
  strMonth = "";
  return month;
}

//Get current year
int GetYear()
{
  dateTime = GetDateTime();
  j = 0;
  for(i=0;i<dateTime.length();i++)
  {
    if(j==3)
      strYear += dateTime[i];
    if(dateTime[i]==':')
      j++;
    if(j==2)
    {
      j = 3;
      i = i + 3;
    }
  }
  year = strYear.toInt();
  strYear = "";
  return year;
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
  configTime(TZ_SEC, DST_SEC, "asia.pool.ntp.org");
  
  WifiConnection();          //Connect to Wi-Fi
  FirebaseConnection();      //Connect to Firebase
  SetSystemStatusOff();      //Set system to off initially
  GenerateVerificationKey(); //Generate the verification key
  CreateFuzzyIO();           //Create the Fuzzy Logic module

  if(Firebase.getString(firebaseData, "Palm Age Update Date"))
    startMillisPalmAge = millis();
}

void loop()
{
  ConnectMobileApp();   //Recieve connection request from the mobile application
  DiconnectMobileApp(); //Recieve disconnection request from the mobile application
  PalmAgeUpdate();      //Update and display palms age
  SystemStatus();       //Check system status (on/off)
  
  firebaseData.clear();
}
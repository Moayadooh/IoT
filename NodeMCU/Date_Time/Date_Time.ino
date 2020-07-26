#include <ESP8266WiFi.h>
#include <time.h>
#include <sys/time.h>

#define SSID            "HUAWEI-5GCPE-A997"
#define SSIDPWD         "H3B96N3Q1YM"
#define TZ              -9       // (utc+) TZ in hours
#define DST_MN          60      // use 60mn for summer time in some countries

//#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)

time_t now;

void setup() {
  Serial.begin(9600);
  configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
  WiFi.begin(SSID, SSIDPWD);
}

void loop() {
  now = time(nullptr);
  Serial.println(ctime(&now));
  delay(1000);
}

/* TimeSyncExample
 * Example sketch for how to use the clock.h file
 */


#include <ESP8266WiFi.h>
#include "clock.h"

const char * ssid = "Cole";
const char * pass = "cole1234";

int last_t = 0;


void setupWiFi(const char * _ssid, const char * _pass) {
  //Connect to a WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  //IPAddress ip = WiFi.localIP();
}

void setup() {
  Serial.begin(115200);
  
  Serial.println("Connecting to WiFi");
  setupWiFi(ssid, pass);

  Serial.println("Setting up time sync");
  setupTime();

  Serial.println("Time is synced, will update on first second of every minute");
  Serial.println(getTimeDateStr());
  
  //so the minutes are updated 1 s after minute changes
  last_t = millis() - (1000 * getSecond() ) + 1000;    
}


void loop() {
  //Every minute, update time
  if (millis() - last_t >= 1000 * 60) {
    last_t = millis();
    String test_datetime = getClock() + " " + getDate(false, true);
    Serial.println(getTimeDateStr());
  }

}

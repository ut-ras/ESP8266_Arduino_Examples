#ifndef CLOCK_H
#define CLOCK_H


/* CLOCK example for ESP8266
 * https://github.com/ut-ras/ESP8266_Arduino_Examples/TimeSyncExample/clock.h
 * Example of how to time sync the ESP8266 and generate strings from the time functions
 * 
 * Time Function References
 * configTime reference: https://github.com/esp8266/Arduino/blob/master/cores/esp8266/time.c
 * time reference: https://www.tutorialspoint.com/c_standard_library/c_function_time.htm
 * localtime reference: https://www.tutorialspoint.com/c_standard_library/c_function_localtime.htm
 * 
 * find your timezone: https://www.timeanddate.com/time/map/
 */

#include <time.h>

//set the timezone and daylight savings
#define TIMEZONE -6
#define DST 0

//set the timeout for turning on Time Sync
#define SYNC_TIMEOUT_S 10




/* Clock Functions */

void setupTime();
boolean isTimeSync();   //only use time functions if this is true
struct tm * getLocalT();    
String getTimeDateStr(); 
   
int getSecond();
int getMinute();
int getHour();  //[0, 23]

String getClock();  //long_form=true, seconds=false
String getClock(bool long_form, bool seconds);
String getDate();   //long_form=true, day_of_wk=true
String getDate(bool long_form, bool day_of_wk);


/* Helper Functions */
String getMonth(int m, boolean long_form);
String getWkday(int wd, boolean long_form);
int hour_24to12(int h24);


//this boolean is true in setupTime if the time sync is set and WiFi is connected
boolean is_sync = false;





/* setupTime
 * Synchronize time - only works if connected to the internet 
 */
void setupTime() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(TIMEZONE * 3600, DST * 3600, "pool.ntp.org", "time.nist.gov");
    //Serial.println("Waiting on time sync...");

    int start_t = millis();
    while (time(nullptr) < 1510644967) {
      delay(10);
      
      //check for timeout
      if ((millis() - start_t) >= (SYNC_TIMEOUT_S * 1000)) {
        Serial.println("Time Sync: Timed out waiting for sync");
        return;
      }
    }
    is_sync = true;
  }
  else {
    Serial.println("Time Sync: Not connected to WiFi, can't time sync");
  }
}




/* isSync
 * Return true if time is synchronized
 */
boolean isTimeSync() {  return is_sync; }

/* getLocalT
 * get the (struct tm *) that represents the local time
 */
struct tm * getLocalT() {
  time_t now;
  time(&now);
  struct tm * timeinfo = localtime(&now); 
  return timeinfo;
}

/* getTimeDateStr
 * asctime Default Format : Wkd Mon DD HH:MM:SS YYYY
 */
String getTimeDateStr() {
  //asctime(timeinfo) is a quick way to get the string
  struct tm * timeinfo = getLocalT();
  return String(asctime(timeinfo));
}




/* getClock
 * long_form:   [HH:MM PM]
 * short_form:  [HH:MM]
 * seconds: add ":SS" to end of time, after "MM"
 */
String getClock(bool long_form, bool seconds) {
  struct tm * timeinfo = getLocalT();

  int hour = timeinfo->tm_hour;
  int min = timeinfo->tm_min;
  int sec = timeinfo->tm_sec;
  
  String ampm = "AM";
  if (hour >= 12) {
    ampm = "PM";
  }

  //convert to 12 hour format
  hour = hour_24to12(timeinfo->tm_hour);
  
  String s = "[";
  s += String(hour) + ":" + String(min);

  if (seconds)   { s += ":" + String(sec); }
  if (long_form) { s += " " + ampm;        }
  
  s += "]";
  return s;
}

String getClock() {
  return getClock(true, false);
}


/* getDate
 * long_form:     Month DD, YYYY
 * short_form:    MM/DD/YYYY
 * day_of_wk:     add "Monday, " to beginning
 */
String getDate(bool long_form, bool day_of_wk) {
  struct tm * timeinfo = getLocalT();

  int month_i = timeinfo->tm_mon;
  int day = timeinfo->tm_mday;
  int year = 1900 + timeinfo->tm_year;           //tm_year returns year since 1900
  int wday_i = timeinfo->tm_wday;

  String month = getMonth(month_i, long_form);

  String dateStr = "";

  if (day_of_wk){
    dateStr += getWkday(wday_i, long_form) + ", ";
  }

  if (long_form) {
    dateStr += month + " " + String(day) + ", " + String(year);
  }
  else {
    dateStr += String(month_i) + "/" + String(day) + "/" + String(year);
    //dateStr += String(day) + " " + month + " " + String(year);    //short form month
  }

  return dateStr;
}

String getDate() {
  return getDate(true, true);
}




/* getMonth
 * Get string for month corresponding to number m [0, 11]
 */
String getMonth(int m, boolean long_form) {
  switch(m) {
    case 0: 
      return long_form?"January":"Jan";break;
    case 1: 
      return long_form?"February":"Feb";break;
    case 2: 
      return long_form?"March":"Mar";break;
    case 3: 
      return long_form?"April":"Apr";break;
    case 4: 
      return long_form?"May":"May";break;
    case 5: 
      return long_form?"June":"Jun";break;
    case 6: 
      return long_form?"July":"Jul";break;
    case 7: 
      return long_form?"August":"Aug";break;
    case 8: 
      return long_form?"September":"Sep";break;
    case 9: 
      return long_form?"October":"Oct";break;
    case 10: 
      return long_form?"November":"Nov";break;
    case 11: 
      return long_form?"December":"Dec";break;
    default:
      return "Error";break;
  }
}

/* getWkday
 * Get string for weekday corresponding to number wd [0, 6]
 */
String getWkday(int wd, boolean long_form) {
  switch(wd) {
    case 0: 
      return long_form?"Sunday":"Sun";break;
    case 1: 
      return long_form?"Monday":"Mon";break;
    case 2: 
      return long_form?"Tuesday":"Tues";break;
    case 3: 
      return long_form?"Wednesday":"Wed";break;
    case 4: 
      return long_form?"Thursday":"Thurs";break;
    case 5: 
      return long_form?"Friday":"Fri";break;
    case 6: 
      return long_form?"Saturday":"Sat";break;
    default:
      return "Error";break;
  }
}

/* hour_24to12
 * Get conversion from 24 hour format [0, 23] to 12 hour format
 */
int hour_24to12(int h24) {
  int h12 = h24;
  if (h12 > 12) {
    h12 -= 12;
  }
  else if (h12 == 0) {
    h12 = 12;
  }
  return h12;
}




/* getSecond [0, 59]
 */
int getSecond() {
  struct tm * timeinfo = getLocalT();
  return timeinfo->tm_sec;
}

/* getMinute [0, 59]
 */
int getMinute() {
  struct tm * timeinfo = getLocalT();
  return timeinfo->tm_min;
}

/* getHour [0, 23]
 */
int getHour() {
  struct tm * timeinfo = getLocalT();
  return timeinfo->tm_hour;
}


#endif


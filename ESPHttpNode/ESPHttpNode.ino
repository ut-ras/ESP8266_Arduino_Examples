/*
 * ESPHttpNode
 * HTTP Node examples adapted from ESP8266HTTPClient examples.
 * Mostly just a tester file for HTTP event-stream/ Server Sent Event listener function (sseListen).
 * HTTP GET and POST examples also added for convenience.
 *
 *
 * HTTP Event Streams / Server-Sent Events
 * Server-Sent events are a connection between a client and server where the session is
 * kept alive. The connection is requested by the client with a standard GET request.
 * The session is kept alive and HTTP "text/event-stream" responses are sent from the server.
 * The sseListen() function is meant to be similar to the Javascript EventSource, which can
 * handle this connection in a Browser. However, sseListen() is not fully standard compliant for EventSource.
 *    https://www.w3schools.com/html/html5_serversentevents.asp
 *    https://www.w3.org/TR/eventsource/
 *    https://developer.mozilla.org/en-US/docs/Web/API/EventSource
 *    https://www.html5rocks.com/en/tutorials/eventsource/basics/
 *    https://hpbn.co/server-sent-events-sse/
 *
 * ESP8266HTTPClient
 *    https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/
 *
 */


#include <ESP8266WiFi.h>

#include "espHttpNode.h"


const char * ssid = "Demobot";
const char * pass = "demobot1234";
IPAddress ip;


void setup() {
  Serial.begin(115200);
  setupWiFi(ssid, pass);
  setupHTTP();
}

void loop() {
  // Check WiFi
  setupWiFi(ssid, pass);

  // HTTP Server Sent Event
  // Remains in this function as long as the server maintains the connection.
  sseListen("http://192.168.4.1:80/sse");

  //String get_resp = httpGet("http://192.168.4.1:80/getsomething");
  //int post_resp = httpPost("http://192.168.4.1:80/postsomething", "something");

  delay(2000);
}


/* Connect to a WiFi network
 */
void setupWiFi(const char * _ssid, const char * _pass) {
  //Connect to a WiFi network
  if (!isWiFiConnected()) {
    Serial.print("WiFi Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _pass);
    while (!isWiFiConnected()) {
      delay(500);
      yield();
      Serial.print(".");
    }
    ip = WiFi.localIP();

    Serial.println("");
    Serial.print("WiFi Connected ssid: ");
    Serial.println(_ssid);
  }
}

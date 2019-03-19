/*
 * ESPHttpNode
 * HTTP Node examples adapted from ESP8266HTTPClient examples.
 * Mostly just a tester file for HTTP event-stream/ Server Sent Event listener function (sseListen).
 * HTTP GET and POST examples also added for convenience.
 *
 * HTTP Event Streams / Server-Sent Events
 * Server-Sent events are a connection between a client and server where the session is
 * kept alive. The connection is requested by the client with a standard GET request.
 * The session is kept alive and HTTP "text/event-stream" responses are sent from the server.
 * The sseListen() function is meant to be similar to the Javascript EventSource, which can
 * handle this connection in a Browser.
 *    https://www.w3schools.com/html/html5_serversentevents.asp
 *    https://developer.mozilla.org/en-US/docs/Web/API/EventSource
 *    https://www.html5rocks.com/en/tutorials/eventsource/basics/
 *    https://hpbn.co/server-sent-events-sse/
 *
 * ESP8266HTTPClient
 *    https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/
 *
 */


#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>


const char * ssid = "Demobot";
const char * pass = "demobot1234";
const char * sse_url = "192.168.4.1/sse";



void setup() {
  setupWiFi(ssid, pass);

}

void loop() {
  sseListen(sse_url);
}



/* Connect to a WiFi network
 */
void setupWiFi(const char * _ssid, const char * _pass) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    yield();
    //Serial.print(".");
  }

}



/* HTTP event-stream
 * This function models behavior of a JS EventSource
 * Sends a GET request to url, session maintained for an event-stream response
 * Server Sent Event loop, waits in function for duration of session
 */
void sseListen(const char * _sse_url) {
  WiFiClient client;
  HTTPClient http;
  http.setReuse(true);                    //keepalive //might need to be in setup
  http.begin(client, String(_sse_url));

  int httpCode = http.GET();              //request event stream
  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    uint8_t buff[128] = { 0 };            //temporary buffer for 128b at a time

    int len = http.getSize();             //(-1 when Server sends no Content-Length header)
    WiFiClient * stream = &client;
    while (http.connected() && (len == -1)) {
      size_t size = stream->available();

      if (size) {
        //buffer overwritten by amount of available bytes
        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
        handleSseData(buff, c);
      }
      delay(1);
    }
    Serial.println();
    Serial.print("[HTTP] connection closed or response has specified Content-Length.\n");
  }
  else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

/* Server Sent Event data received
 */
void handleSseData(uint8_t * buff, int buff_len) {
  //up to 128 bytes in buff at this point
  //not necessarily a full SSE line, need to parse
  //events deliminated by double newlines
  //data in format "event: ", and "data: "
  //if not a full line, might need to store remainder until next time

  //TODO

  Serial.write(buff, buff_len);
}




/* HTTP GET
 * session started, request sent, wait for response, session ended
 * response payload returned as String
 */
String httpGet(String url) {
  WiFiClient client;
  HTTPClient http;
  String payload = "";
  http.begin(client, url);

  //send HTTP request
  //httpCode = http.GET();
  int httpCode = http.sendRequest("GET");
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);

  if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
    //can also get the response in a buffer instead of a String (see ESP8266HTTPClient examples)
    payload = http.getString();     //get response
    Serial.println(payload);
  }
  else {
    payload = "HTTP_" + String(httpCode);
  }

  http.end();
  return payload;
}

/* HTTP POST
 * session started, request sent, session ended
 */
int httpPost(String url, String payload) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);

  //send HTTP request
  int httpCode = http.POST(payload);
  //int httpCode = http.sendRequest("POST", payload, size);     //uint8_t * payload, size_t size

  Serial.printf("[HTTP] POST... code: %d\n", httpCode);
  http.end();
  return httpCode;
}

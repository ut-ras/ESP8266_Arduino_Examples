/*
 * ESPSseServer
 *
 *
 * HTTP Event Streams / Server-Sent Events
 * Server-Sent events are a connection between a client and server where the session is
 * kept alive. The connection is requested by the client with a standard GET request.
 * The session is kept alive and HTTP "text/event-stream" responses are sent from the server.
 * Browsers can listen to event streams in the background with Javascript EventSource,
 * or other devices can listen like in the ESPHttpNode example.
 *    https://www.w3schools.com/html/html5_serversentevents.asp
 *    https://developer.mozilla.org/en-US/docs/Web/API/EventSource
 *    https://www.html5rocks.com/en/tutorials/eventsource/basics/
 *    https://hpbn.co/server-sent-events-sse/
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


//WiFi Settings
//STA = connect to a WiFi network with name ssid
//AP = create a WiFi access point with  name ssid
#define WIFI_MODE "AP"
const char * ssid = "Demobot";
const char * pass = "demobot1234";


//Web Server
int port = 80;
IPAddress ip;
String webServerPath = "http://";

//Web server at port 80
ESP8266WebServer server(port);


//Server Sent Event
boolean sse_on = false;



/* Setup Functions */

/* setupWiFi
 * STA = connect to a WiFi network with name ssid
 * AP = create a WiFi access point with  name ssid
 */
void setupWiFi(String mode, const char * _ssid, const char * _pass) {
  if (mode.equals("AP")) {
    //Turn on Access Point
    WiFi.softAP(_ssid, _pass);
    ip = WiFi.softAPIP();
  }
  else {
    //Connect to a WiFi network
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _pass);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      yield();
      //Serial.print(".");
    }
    ip = WiFi.localIP();
  }
}

void setupWebServer() {
  //Turn on a web server at port 80
  //Map paths to hander functions, can also specify HTTP methods
  //server.on("/", handleRoot);
  server.on("/sse", handleSseRequest);
  server.onNotFound(handleNotFound);    //404 Not Found

  server.begin();

  webServerPath += ip.toString() + ":" + String(port) + "/";
}

void setup() {
  Serial.begin(115200);

  setupWiFi(WIFI_MODE, ssid, pass);   //Access Point or Station
  setupWebServer();                   //Set up the Web Server

  Serial.println("WiFi mode=" + String(WIFI_MODE) + ", ssid = " + String(ssid) + ", pass = " + String(pass));
  Serial.println("Web server at " + webServerPath);
}



/* Main Loop */
void loop() {
  //handle sse updates
  serverSentEventUpdate("eventname", "eventdata");

  //handle web server
  server.handleClient();

  delay(500);
}



/* Server-Sent Event Control */

// server-sent event request  "/sse"
void handleSseRequest() {
    WiFiClient client = server.client();
    if (client) {
      sse_on = true;
      //Respond to the client to say Server Sent Event stream is starting
      serverSentEventHeader(client);
      Serial.println("SSE client connected");
    }
}

// server-sent event stream to "/sse"
void serverSentEventUpdate(String eventName, String data) {
  if (sse_on) {
    WiFiClient client = server.client();
    if (client.connected()) {
      serverSentEvent(client, eventName, data);
      Serial.println("SSE client updated");
    }
    else {
      sse_on = false;
      delay(1);
      client.stop();
      Serial.println("SSE client disconnected");
    }
  }
}


/* Server-Sent Event HTTP */

// server-sent event header: sent once when the stream is initialized
void serverSentEventHeader(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Connection: keep-alive");
  client.println("Content-Type: text/event-stream");
  client.println("Cache-Control: no-cache");
  client.println();
  client.flush();
}

// server-sent event data stream
void serverSentEvent(WiFiClient client, String eventName, String data) {
  if (eventName != NULL && data != NULL) {
    client.println("event: " + eventName);
    client.println("data: " + data);
    client.println();
    client.flush();
  }
}




/* HTML */

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\n";
  message += "Method: " + String((server.method() == HTTP_GET)?"GET":"POST") + "\n";
  message += "Arguments: " + String(server.args()) + "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

/* ESP8266 WiFi Serial Tool
 *
 * Sets up the ESP8266 as a WiFi Access Point or connects to an existing WiFi network
 * Launches a Web Server with a form to output text over Serial
 * The server also outputs a console with live updates from Serial events on the ESP8266,
 *      using Javascript and Server-Sent HTTP event stream
 *
 * WIFI_MODE variable
 * AP Mode: Launches the web server at http://192.168.4.1:80/
 * STA Mode: Connects to an existing WiFi network
 *
 * Set WiFi credentials with "ssid" and "pass" strings
 *
 *
 * Resources
 *
 * Web Server
 * https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/HelloServer
 * https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
 * https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
 * Arduino IDE: Examples > ESP8266WebServer folder
 *
 * Soft Access Point
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
 *
 * Server-Sent Events and Javascript Event Listener
 * https://www.w3schools.com/html/html5_serversentevents.asp
 * https://developer.mozilla.org/en-US/docs/Web/API/EventSource
 * https://www.html5rocks.com/en/tutorials/eventsource/basics/
 * https://hpbn.co/server-sent-events-sse/
 *
 * Javascript DOM update
 * https://www.w3schools.com/js/js_htmldom.asp
 * https://www.w3schools.com/js/js_htmldom_nodes.asp
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


//WiFi Settings
//STA = connect to a WiFi network with name ssid
//AP = create a WiFi access point with  name ssid
#define WIFI_MODE "AP"
const char * ssid = "ESP_Serial_Tool";
const char * pass = "esp";


//Web Server
int port = 80;
IPAddress ip;
String webServerPath = "http://";

//Web server at port 80
ESP8266WebServer server(port);


//Serial Monitor
boolean serial_monitor_on = false;





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

  server.on("/", handleRoot);
  server.on("/serial", HTTP_POST, handleSerial);
  server.on("/serial", HTTP_GET, handleRoot);
  server.on("/serialupdate", handleSerialUpdate);
  server.onNotFound(handleNotFound);    //404 Not Found

  server.begin();

  webServerPath += ip.toString() + ":" + String(port) + "/";
}

void setup() {
  Serial.begin(115200);

  setupWiFi(WIFI_MODE, ssid, pass);   //Access Point or Station
  setupWebServer();                   //Set up the Web Server

  //Serial.println("WiFi mode=" + String(WIFI_MODE) + ", ssid = " + String(ssid) + ", pass = " + String(pass));
  //Serial.println("Web server at " + webServerPath);
}



/* Main Loop */

void loop() {
  //handle serial event updates
  if (serial_monitor_on) {
    handleSerialUpdateEvent();
  }

  //handle web server
  server.handleClient();
}





/* Request Handlers */

//main page   "/"
void handleRoot() {
  server.send(200, "text/html", indexHTML());
}


//serial posts    "/serial"
void handleSerial() {
  //hasArg() checks if the last HTTP request in the server has an argument
  //arg() gets the value of the arg by name

  //turn serial monitor off before outputting
  serial_monitor_on = false;

  //check for serial input form
  if(server.hasArg("serial_input")) {
    String serial_input = server.arg("serial_input");
    Serial.println(serial_input);
  }

  handleRoot();
}


// server-sent event request  "/serialupdate"
void handleSerialUpdate() {
  WiFiClient client = server.client();
  if (client) {
    //Serial.println("New Serial Monitor Request");
    serial_monitor_on = true;

    //Respond to the client to say Server Sent Event stream is starting
    serverSentEventHeader(client);
  }
}

// server-sent event stream to "/serialupdate"
void handleSerialUpdateEvent() {
  WiFiClient client = server.client();
  if (client.connected()) {
    serverSentEvent(client);
    delay(16);
  }
  else {
    serial_monitor_on = false;
    delay(1);
    client.stop();
    //Serial.println("client disconnected");
  }
}



/* Server-Sent Event HTTP*/

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
void serverSentEvent(WiFiClient client) {
  String mssg = Serial.readString();
  if (mssg != "" && mssg != NULL) {
    //Serial.println("Received Serial Input " + mssg);
    client.println("event: serialupdate");
    client.println("data: " + mssg + "");
    client.println();
    client.flush();
  }
}



/* HTML */

String indexHTML() {
  String htmlPage =
            String("<body>") +
              "<div id=\"page_header\" style=\"margin: 0 5% 2em 5%;\">" +
                "<h1>ESP8266 Serial Tool</h1>" +
              "</div>" +

              "<div id=\"serial_out\" style=\"margin: 0 5% 2em 5%;\">" +
                "<h3>Serial Output</h3>" +
                "<form action=\"/serial\" method=\"post\">" +
                  "<input type=\"text\" name=\"serial_input\" placeholder=\"Output to ESP8266 Serial\"><br>" +
                  "<input type=\"submit\" value=\"Output Serial\"><br>" +
                "</form>" +
              "</div>" +

              "<div id=\"serial_in\" style=\"margin: 0 5% 2em 5%;\">" +
                "<h3>Serial Input</h3>" +
                "<div id=\"serial_display\" style=\"padding-left: 5%; border:inset;\">" +
                "</div>" +
              "</div>" +

              getConsoleEventJavascript() +
            "</body>";
  return htmlPage;
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}



/* JAVASCRIPT */

String getConsoleEventJavascript() {
  String s = String("<script>") +
      //function to add an HTML <p> element to the serial_display div with the DOM
      "function addConsoleEvent(mssg) {" +
        "var text = document.createTextNode(mssg);" +
        "var pg = document.createElement('p');" +
        "pg.appendChild(text); " +
        "document.getElementById('serial_display').appendChild(pg);" +
      "}" +

      //Initialize an EventSource to listen for server sent events at /serialupdate
      "var source = new EventSource('/serialupdate');" +
      "if(typeof(EventSource)!=='undefined') {" +
        "console.log('EventSource initialized');" +

        /* // Server sent event listener version 1: this wasn't working for me, not sure why
        "source.onmessage = function(e) {" +
            "console.log('Serial Console Event onmessage' + e.data);" +
        "};" + */

        // Server sent event listener version 2: this works for me
        "source.addEventListener('serialupdate', function(e) {" +
            //"var data = JSON.parse(e.data);" +    //json parsing example
            "console.log('Serial Console Event ' + e.data);" +
            "addConsoleEvent(e.data);" +
        "}, false);"

        "console.log('EventSource Listener Initialized');" +
      "}" +
      "else {" +
        "console.log('EventSource Listener could not be initialized');" +
        "addConsoleEvent('Serial Input Console could not be initialized');" +
        //TODO use polling if EventSource is not supported by browser
      "}" +
  "</script>";
  return s;
}


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
 * Javascript AJAX and DOM
 * https://www.w3schools.com/js/js_htmldom.asp
 * https://www.w3schools.com/js/js_ajax_intro.asp
 * https://www.w3schools.com/js/js_htmldom_nodes.asp
 * https://www.w3schools.com/js/js_ajax_http_send.asp
 * https://www.w3schools.com/howto/howto_js_trigger_button_enter.asp
 * https://www.w3schools.com/jsref/event_onclick.asp
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
  String serial_input = "";
  if(server.hasArg("serial_input")) {
    serial_input = server.arg("serial_input");
    Serial.println(serial_input);
  }

  //handleRoot();     //now the form is handled with JS so there is no need to respond with index html
  server.send(200, "text/plain", serial_input);
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
  if (Serial.available()) {
    WiFiClient client = server.client();
    if (client.connected()) {
      /* minimize serial data is lost, the Serial buffer insn't big
       * we could readStringUntil('\n') 1 string each loop, but the buffer might fill, so we want to empty it each loop
       * Idea 1 (below): loop Serial.available, send all data, will hang the server and prevent client reqs for lots of inputs though
       * Idea 2 (TODO): loop Serial.available, read all data into an extra buffer, send 1 from that buffer each loop on fixed dt
       * Either way, we can still lose data if the Arduino Serial buffer fills up between loops
       * Also, since we are not getting a response from the client, we can't be sure it got there
       */
      String mssg = "";
      while (Serial.available()) {
        if (mssg != "") {delay(16);}            
        mssg = Serial.readStringUntil('\n');
        //Serial.println("Received Serial Input " + mssg);
        serverSentEvent(client, "serialupdate", mssg);
      }
    }
    else {
      serial_monitor_on = false;
      delay(1);
      client.stop();
      //Serial.println("client disconnected");
    }
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
void serverSentEvent(WiFiClient client, String eventName, String data) {
  if (data != NULL) {
    client.println("event: " + eventName);
    client.println("data: " + data);
    client.println();
    client.flush();
  }
}



/* HTML */
//TODO stop using inline style tags and just make some css classes
String indexHTML() {
  String htmlPage =
            String("<body style=\"font-family:'Arial'; background-color:#1d1f21; color:#c5c8c6;\">") +
              "<div id=\"page_header\" style=\"margin: 0 5% 2em 5%; color:#cc6666;\">" +
                "<h1>ESP8266 Serial Tool</h1>" +
              "</div>" +

              "<div id=\"page_serial\" style=\"margin: 0 5% 2em 5%;\">" +
                "<h3 style=\"color:#81a2be;\">Serial Console</h3>" +
                
                "<div id=\"serial_console\" style=\"border:inset;  border-color:#8abeb7; background-color:#282a2e;\">" +             
                  "<div id=\"serial_display\" style=\"padding-left: 1.5em; min-height: 5em; max-height: 10em; overflow: auto; font-size:medium;\">" +
                    //Stuff is added here from JS
                  "</div>" +
  
                  "<div id=\"serial_out\" style=\"border-top:groove; border-color:#8abeb7;\">" +
                    "<button onclick=\"postSerial()\" style=\"float: right;  color:#1d1f21; background-color:#8abeb7;border-color:#5e8d87; font-family:'Arial';font-size:medium;\">Output Serial</button>" +
                    "<div style=\"overflow: hidden;\">" +
                      "<input type=\"text\" id=\"serial_input\" placeholder=\"Output to ESP8266 Serial\" style=\"width: 100%; padding-left: 1.5em; border:none; background-color:#282a2e; color:#c5c8c6; font-family:'Arial';font-size:medium;\">" +
                    "</div>" +
                  "</div>" +
                "</div>" +
              "</div>" +
              
              getConsoleEventJavascript() +
            "</body>";
  return htmlPage;
}

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



/* JAVASCRIPT */

String getConsoleEventJavascript() {
  String s = String("<script>") +
      //function to add an HTML <p> element to the serial_display div with the DOM
      "function addConsoleEvent(mssg) {" +
        "var text = document.createTextNode(mssg);" +
        "var pg = document.createElement('p');" +
        "pg.appendChild(text); " +
        "var display = document.getElementById('serial_display'); " +
        "display.appendChild(pg); " +
        "display.scrollTop = display.scrollHeight; " +
      "}" +
      
      //function to HTTP Post
      "function postSerial() {" +
        "var textBox = document.getElementById('serial_input');" +
        "var mssg = textBox.value;" +

        "var xhttp = new XMLHttpRequest(); " +
        "xhttp.open('POST', '/serial', true);" +
        "xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');" +
        "xhttp.send('serial_input=' + mssg);" +

        "textBox.value = '';" +
        "xhttp.onload = function() { " +
          //Only add the mssg to the console if the ESP8266 received it and sent a response back
          "console.log('Serial Post: ' + xhttp.responseText); " +
          "addConsoleEvent('> ' + xhttp.responseText);" +
        "}" +
      "}" +
      
      // Response when user presses "Enter" in the input text box
      "var textBox = document.getElementById('serial_input');" +
      "textBox.addEventListener('keyup', function(e) {" +
        "e.preventDefault();" +
        "if (e.keyCode === 13) { postSerial(); }" +
      "});" +

      //Initialize an EventSource to listen for server sent events at /serialupdate
      "var source = new EventSource('/serialupdate');" +
      "if(typeof(EventSource)!=='undefined') {" +
        "console.log('EventSource initialized');" +

        // Server sent event listener
        "source.addEventListener('serialupdate', function(e) {" +
            //"var data = JSON.parse(e.data);" +    //json parsing example
            "console.log('Serial Console Event: ' + e.data);" +
            "addConsoleEvent('< ' + e.data);" +
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


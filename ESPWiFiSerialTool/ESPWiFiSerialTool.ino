/* ESP8266 WiFi Serial Tool
 *
 * Sets up the ESP8266 as a WiFi Access Point or connects to an existing WiFi network
 * Launches a Web Server with a form to output text over Serial
 *
 * WIFI_MODE variable
 * AP Mode: Launches the web server at http://192.168.4.1:80/
 * STA Mode: Connects to an existing WiFi network
 *
 * Set WiFi credentials with ssid and pass strings
 *
 * Web Server
 * https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/HelloServer
 * https://links2004.github.io/Arduino/d3/d58/class_e_s_p8266_web_server.html
 * https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WebServer/src/ESP8266WebServer.h
 * Examples > ESP8266WebServer
 *
 * Soft Access Point
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
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



void loop() {
  //handle web server
  server.handleClient();
}






/* Request Handlers */

//main page
void handleRoot() {
  server.send(200, "text/html", indexHTML());
}

//serial posts
void handleSerial() {
  //hasArg() checks if the last HTTP request in the server has an argument
  //arg() gets the value of the arg by name

  if(server.hasArg("serial_input")) {
    String serial_input = server.arg("serial_input");
    Serial.println(serial_input);
  }
  handleRoot();
}





/* HTML */

String indexHTML() {
  String htmlPage =
            String("<body>") +
              "<h1>ESP8266 Serial Tool</h1>" +
              "<form action=\"/serial\" method=\"post\">" +
                "<input type=\"text\" name=\"serial_input\" placeholder=\"Output to ESP8266 Serial\"><br>" +
                "<input type=\"submit\" value=\"Output Serial\"><br>" +
              "</form>" +
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

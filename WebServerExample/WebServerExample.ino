/* ESP Web Server Example
 * Uses the ESP8266WiFi library WiFiServer and WiFiCLient streams
 * In progress
 *
 * TODO
 * Work on creating list of clients and handling basic connections
 *
 *  Sets up the ESP8266 as a WiFi Access Point
 *  Launches a Web Server with a form to output text over Serial
 *  Usually launches the web server at http://192.168.4.1:81/
 *
 *
 * Web Server
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/server-examples.html
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/server-class.html
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/client-examples.html
 * https://playground.arduino.cc/Main/ParsingAnHttpRequest
 *
 * Soft Access Point
 * https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/soft-access-point-class.html
 */


#include <ESP8266WiFi.h>


//Soft AP
const char * ssid = "ESP_AP";
const char * pass = "demobots";

//Web Server
int port = 81;
IPAddress ip;
String webServerPath = "http://";

//Web server at port
WiFiServer server(port);


void setup() {
  Serial.begin(115200);

  //Turn on Access Point
  WiFi.softAP(ssid, pass);
  ip = WiFi.softAPIP();

  //Turn on a web server at port
  //Map paths to hander functions, can also specify HTTP methods
  server.begin();

  //URL of Web Server
  webServerPath += ip.toString() + ":" + String(port) + "/";

  Serial.println("Access Point ssid = " + String(ssid) + ", pass = " + String(pass));
  Serial.println("Web server at " + webServerPath);

}



void loop() {
  handleClients();
}


//handle web server HTTP requests from clients
void handleClients() {
  // Get one of the waiting clients
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  int max_wait = 4000;
  int start_wait = millis();
  while(!client.available()){
    delay(20);
    if (millis() - start_wait > max_wait) { break; }
    yield();
  }

  // read the header - everything until the empty line
  String header = "";
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (line == "\n") {client.readStringUntil('\n'); break;}
    header += line;
  }

  Serial.println("Header: " + header);

  // parse the header for request type and path
  int space_1 = header.indexOf(" ");
  int space_2 = header.indexOf(" ", space_1 + 1);
  String req_type = header.substring(0, space_1);              //req_type: POST, GET, etc
  String req_path = header.substring(space_1 + 1, space_2);    //req_path: action in the form, /something or /

  // parse the header for length of content
  String key = "Content-Length: ";
  int len_i = header.indexOf(key);
  int content_l = 0;
  if (len_i != -1) {
    len_i += key.length();
    int len_end = header.indexOf("\r", len_i);
    String len = header.substring(len_i, len_end);
    content_l = 3000;
    content_l = len.toInt();
  }
  //Serial.println("Content Length: " + content_l);

  // read the content
  String content = "";
  if (req_type.equals("POST") && (content_l > 0)) {
    content = client.readStringUntil('\n');
    //content = content.substring(0, content_l);
  }
  Serial.println("Content: " + content);

  client.flush();

  //route the request to a handler
  route_urls(client, req_type, req_path, content);

  //client disconnected when function exits because client object destroyed
}


//route each url to a handler
void route_urls(WiFiClient client, String req_type, String req_path, String content) {
  //send_http_resp(client, "200 OK", "text/html", html_string);

  if (req_path.equals("/") && req_type.equals("GET")) {
    Serial.println("Get Index");
    handleRoot(client, content);
  }
  else if (req_path.equals("/serial")) {
    Serial.println("Serial Post: " + content);
    handleSerial(client, content);
  }
  else {
    send_http_resp(client, "404 Not Found", "text/plain", "404 Not Found");
  }

}


//main page handler
void handleRoot(WiFiClient client, String content) {
  send_http_resp(client, "200 OK", "text/html", indexHTML());
}

//serial posts handler
void handleSerial(WiFiClient client, String content) {
  String serial_input = parse_content(content, "serial_input");
  if (serial_input != "") {
    Serial.println(serial_input);
  }
  handleRoot(client, "");
}



//main page HTML
String indexHTML() {
  String htmlPage =
            String("<body>") +
              "<h1>ESP8266 Serial Tool</h1>" +
              "<form action=\"/serial\" method=\"post\">" +
                "<input type=\"text\" name=\"serial_input\" value=\"Output to ESP8266 Serial\"><br>" +
                "<input type=\"submit\" value=\"Output Serial\"><br>" +
              "</form>" +
            "</body>";
  return htmlPage;
}



//send an HTTP response to a client
//general usage: send_http_resp(client, "200 OK", "text/html", html_string)
void send_http_resp(WiFiClient client, String status, String content_type, String data) {
  String s =  http_resp_header(status, content_type, "Closed", data.length()) + data;
  client.flush();
  client.print(s);
}


//build an HTTP response header string
//general usage: http_resp_header("200 OK", "text/html", "Closed")
String http_resp_header(String status, String content_type, String connection, int data_len) {
  String h = String("HTTP/1.1 ") + status + " \r\n" +
              "Content-Length: " + String(data_len) + "\r\n" +
              "Connection: " + connection + "\r\n" +
              "Content-Type: " + content_type + "\r\n" +
              "\r\n";
  return h;
}


//Parse the content of an HTTP post (parses by '&' or end of string)
String parse_content(String content, String key) {
  String value = "";
  int data_i = content.indexOf(key);

  if (data_i != -1) {
    data_i += key.length() + 1; //+1 for the =
    int data_end = content.indexOf("&", data_i);
    if (data_end == -1) {
      data_end = content.length();
    }
    value = content.substring(data_i, data_end);
  }
  value.replace("+", " ");
  return value;
}

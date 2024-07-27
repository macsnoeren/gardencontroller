#include <Arduino.h>

#include <WiFi.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>

// Web server instance that processes the web requests
int count = 0;
std::unique_ptr<WebServer> server;
String greatWebInterface = "https://raw.githubusercontent.com/macsnoeren/internet-of-things/master/wemos-web-ui-leaner/demo-leaner.html";

// Setup of the WifiManager
void setupWifiManager () {
  WiFiManager wifiManager;
  //wifiManager.resetSettings(); // When you need to reset the Wifi settings.
  wifiManager.autoConnect();  // Auto generated name ESP + ChipID
  Serial.println("Connected with Wi-Fi!");
}

void setPin (bool on) {
    digitalWrite(GPIO_NUM_15, (on ? HIGH : LOW));
}

void outdoorLight(bool on) {
  digitalWrite(GPIO_NUM_16, (on ? LOW : HIGH));
}

// Callback method for the web server to handle when the resource is not found
void handleNotFound() {
  server->send(404, "text/plain", "File not found");
}

// Setup of the web server ESP8266WebServer
void setupWebServer () {
  server.reset(new WebServer(WiFi.localIP(), 80));

  // Handle the root of the web server => return spin-off website here!
  // Load scripts and css to be used in the header, like bootstrap and jquery.
  // For cyber security purposes, use the integrity checker of the browser.
  // Load the external documents from the external web server by using a ajax call (using demo.html).
  // when the document is loaded. See the script tag.
  //"<script>var ip = \"" + WiFi.localIP().toString() + "\"; $(document).ready(function(){  $(\"#page\").load(\"https://vmacman.jmnl.nl/wemos/demo.html\"); });</script>"
  server->on("/", []() {
    server->send(200, "text/html",
		 "<!DOCTYPE html lang=\"en\">"
		 "<html>"
		 "<head>"
		 "<meta charset=\"utf-8\">"
		 "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, shrink-to-fit=no\">"
		 "<script src=\"http://code.jquery.com/jquery-1.11.1.min.js\"></script>"
     "<script>var ip = \"" + WiFi.localIP().toString() + "\";</script>"
		 "</head>"
		 "<body>\n"
		 "<div id=\"page\">Loading your great web user interface, please wait ...</div>\n"
     "<script>$(document).ready(function(){  $(\"#page\").load(\"" + greatWebInterface + "\"); });</script>"
		 "</body>"
		 "</html>");
    });

    // When the website want to have some Wemos action, it can do this using the /action page
    // Get parameters of the request to perform the required action of the user.
    // This method is an example to do this
    server->on("/action", []() {
        String message = "Create some action here...\n\n";
        message += "URI: ";
        message += server->uri();
        message += "\nMethod: ";
        message += (server->method() == HTTP_GET) ? "GET" : "POST";
        message += "\nArguments: ";
        message += server->args();
        message += "\n";
        int lightId = 0;
        boolean value = 0;
        for (uint8_t i = 0; i < server->args(); i++) {
            message += " " + server->argName(i) + ": " + server->arg(i) + "\n";
            if ( server->argName(i) == "light" ) {
              lightId = server->arg(i).toInt();
            }
            if ( server->argName(i) == "value" ) {
              value = (server->arg(i) == "on" ? true : false);
            }
        }
        server->send(200, "text/plain", message);

        if ( lightId == 1 ) {
          outdoorLight(value);
        }
                
    });

  // Handle resources that are not found.
  server->onNotFound(handleNotFound);
  
  server->begin();
  
  Serial.println("HTTP server started.");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);  
  Serial.print("Zwembad Controller 1.0\n");

  // Onboard blue led
  gpio_set_direction(GPIO_NUM_15, GPIO_MODE_OUTPUT);
  setPin(false);
  delay(2000);
  setPin(true);
  delay(2000);
  setPin(false);

  gpio_set_direction(GPIO_NUM_16, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
  digitalWrite(GPIO_NUM_16, HIGH);
  digitalWrite(GPIO_NUM_17, HIGH);
  digitalWrite(GPIO_NUM_33, HIGH);

  setupWifiManager();
  setupWebServer();
}

void loop() {
  if ( WiFi.status() == WL_CONNECTED ) {
    server->handleClient(); // Handling the web requests

  } else {
    Serial.println("Lost connection with AP");
    delay(2000);
  }

  if ( count++ > 10 ) {
    count = 0;
    Serial.println("Heart beat!");
    Serial.println(WiFi.status());
  }

  delay(100);
}

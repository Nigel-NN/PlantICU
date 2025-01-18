#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Replace with your network credentials
const char* ssid = "Nigel";
const char* password = "ikrni93715";

// Define relay pins
const int relay1 = D1;
const int relay2 = D2;
const int relay3 = D3;
const int relay4 = D4;

// Create an instance of the web server
ESP8266WebServer server(80);

// Set up NTP client to fetch the time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // UTC time

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  // Print the IP address of the ESP8266
  Serial.println("");
  Serial.println("WiFi Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Set relay pins as outputs
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  // Set initial relay states (off)
  digitalWrite(relay1, LOW);
  digitalWrite(relay2, LOW);
  digitalWrite(relay3, LOW);
  digitalWrite(relay4, LOW);

  // Start the NTP client to get the time
  timeClient.begin();
  timeClient.setTimeOffset(4 * 3600);  // GST is UTC+4 (4 hours ahead)

  // Define route handlers for relay control
  server.on("/", HTTP_GET, handleRoot);
  server.on("/relay1/on", HTTP_GET, relay1On);
  server.on("/relay1/off", HTTP_GET, relay1Off);
  server.on("/relay2/on", HTTP_GET, relay2On);
  server.on("/relay2/off", HTTP_GET, relay2Off);
  server.on("/relay3/on", HTTP_GET, relay3On);
  server.on("/relay3/off", HTTP_GET, relay3Off);
  server.on("/relay4/on", HTTP_GET, relay4On);
  server.on("/relay4/off", HTTP_GET, relay4Off);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Update the NTP client to get the current time
  timeClient.update();

  // Handle incoming HTTP requests
  server.handleClient();
}

// Web page handler to show the relay status and clock
void handleRoot() {
  String relay1State = (digitalRead(relay1) == HIGH) ? "OFF" : "ON";
  String relay2State = (digitalRead(relay2) == HIGH) ? "OFF" : "ON";
  String relay3State = (digitalRead(relay3) == HIGH) ? "OFF" : "ON";
  String relay4State = (digitalRead(relay4) == HIGH) ? "OFF" : "ON";

  // Get the current time from the NTP client
  String formattedTime = timeClient.getFormattedTime();

  String html = "<html>\
<head>\
  <title>Relay Control</title>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <script>\
    function refreshClock() {\
      setTimeout(function() {\
        location.reload();\
      }, 1000);\
    }\
  </script>\
  <style>\
    body { font-family: Arial, Helvetica, sans-serif; background-color: #f7f7f7; margin: 0; padding: 0; }\
    h1 { text-align: center; color: #333; margin-top: 50px; }\
    .container { width: 100%; max-width: 600px; margin: 0 auto; padding: 20px; background-color: #fff; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); display: flex; justify-content: space-between; }\
    .left { width: 45%; }\
    .right { width: 45%; }\
    .switch-container { margin-bottom: 20px; display: flex; align-items: center; }\
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; }\
    .switch input { opacity: 0; width: 0; height: 0; }\
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; transition: .4s; border-radius: 34px; }\
    .slider:before { position: absolute; content: ''; height: 26px; width: 26px; border-radius: 50%; left: 4px; bottom: 4px; background-color: white; transition: .4s; }\
    input:checked + .slider { background-color: #4CAF50; }\
    input:checked + .slider:before { transform: translateX(26px); }\
    label { font-size: 18px; margin-left: 10px; }\
    .clock { font-size: 30px; font-weight: bold; text-align: center; margin-bottom: 20px; }\
    .timer-container { margin-bottom: 20px; display: flex; justify-content: space-between; align-items: center; }\
    input[type='number'] { width: 60px; padding: 5px; font-size: 16px; border-radius: 5px; border: 1px solid #ccc; }\
    @media screen and (max-width: 600px) {\
      .switch { width: 50px; height: 30px; }\
      .container { flex-direction: column; align-items: center; }\
      .left, .right { width: 100%; margin-bottom: 20px; }\
    }\
  </style>\
</head>\
<body onload='refreshClock()'>\
  <div class='container'>\
    <div class='left'>\
      <div class='timer-container'>\
        <label for='relay1Timer'>Water Pump Timer:</label>\
        <input type='number' id='relay1Timer' min='0' max='60' value='0' step='1' onchange=\"location.href='/setRelay1Timer/' + this.value\">\
      </div>\
      <div class='timer-container'>\
        <label for='relay2Timer'>Fertilizer Pump Timer:</label>\
        <input type='number' id='relay2Timer' min='0' max='60' value='0' step='1' onchange=\"location.href='/setRelay2Timer/' + this.value\">\
      </div>\
      <div class='timer-container'>\
        <label for='relay3Timer'>Light Timer:</label>\
        <input type='number' id='relay3Timer' min='0' max='60' value='0' step='1' onchange=\"location.href='/setRelay3Timer/' + this.value\">\
      </div>\
      <div class='timer-container'>\
        <label for='relay4Timer'>Relay 4 Timer:</label>\
        <input type='number' id='relay4Timer' min='0' max='60' value='0' step='1' onchange=\"location.href='/setRelay4Timer/' + this.value\">\
      </div>\
    </div>\
    <div class='right'>\
      <div class='clock'>Current Time (GST): " + formattedTime + "</div>\
      <div class='switch-container'>\
        <label for='relay1'>Water Pump</label>\
        <label class='switch'>\
          <input type='checkbox' " + String((relay1State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay1/" + String((relay1State == "ON" ? "off" : "on")) + "'\">\
          <span class='slider'></span>\
        </label>\
      </div>\
      <div class='switch-container'>\
        <label for='relay2'>Fertilizer Pump</label>\
        <label class='switch'>\
          <input type='checkbox' " + String((relay2State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay2/" + String((relay2State == "ON" ? "off" : "on")) + "'\">\
          <span class='slider'></span>\
        </label>\
      </div>\
      <div class='switch-container'>\
        <label for='relay3'>Light</label>\
        <label class='switch'>\
          <input type='checkbox' " + String((relay3State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay3/" + String((relay3State == "ON" ? "off" : "on")) + "'\">\
          <span class='slider'></span>\
        </label>\
      </div>\
      <div class='switch-container'>\
        <label for='relay4'>Relay 4</label>\
        <label class='switch'>\
          <input type='checkbox' " + String((relay4State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay4/" + String((relay4State == "ON" ? "off" : "on")) + "'\">\
          <span class='slider'></span>\
        </label>\
      </div>\
    </div>\
  </div>\
</body>\
</html>";

  server.send(200, "text/html", html);
}

// Relay control functions (same as before)
void relay1On() {
  digitalWrite(relay1, LOW);  // Turn off when the button says "ON"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay1Off() {
  digitalWrite(relay1, HIGH); // Turn on when the button says "OFF"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay2On() {
  digitalWrite(relay2, LOW);  // Turn off when the button says "ON"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay2Off() {
  digitalWrite(relay2, HIGH); // Turn on when the button says "OFF"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay3On() {
  digitalWrite(relay3, LOW);  // Turn off when the button says "ON"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay3Off() {
  digitalWrite(relay3, HIGH); // Turn on when the button says "OFF"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay4On() {
  digitalWrite(relay4, LOW);  // Turn off when the button says "ON"
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay4Off() {
  digitalWrite(relay4, HIGH); // Turn on when the button says "OFF"
  server.sendHeader("Location", "/");
  server.send(303);
}

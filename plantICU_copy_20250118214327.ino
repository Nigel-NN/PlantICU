#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// Replace with your network credentials
const char* ssid = "Nigel";
const char* password = "ikrni93715";

// Define relay pins
const int relay1 = D1;  // Water Pump
const int relay2 = D2;  // Fertilizer Pump
const int relay3 = D3;  // Light
const int relay4 = D4;  // Other relay

// Define water level sensor pin
const int waterSensorPin = A0;  // Change this if necessary

// Create an instance of the web server
ESP8266WebServer server(80);

// Set up NTP client to fetch the time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000);  // UTC time

// Variables for tracking the pump usage
unsigned long startTimeWater = 0;  // Track the start time of the water pump
unsigned long startTimeFertilizer = 0;  // Track the start time of the fertilizer pump
float totalWaterDispensed = 0;      // Track the total amount of water dispensed in ml
float totalFertilizerDispensed = 0; // Track the total amount of fertilizer dispensed in ml

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  pinMode(relay1, OUTPUT);  // Water pump
  pinMode(relay2, OUTPUT);  // Fertilizer pump
  pinMode(relay3, OUTPUT);  // Light
  pinMode(relay4, OUTPUT);  // Other relay

  digitalWrite(relay1, LOW);  // Ensure pumps are off initially
  digitalWrite(relay2, LOW);  
  digitalWrite(relay3, LOW);  
  digitalWrite(relay4, LOW);  

  timeClient.begin();
  timeClient.setTimeOffset(4 * 3600);  // GST is UTC+4

  server.on("/", HTTP_GET, handleRoot);
  server.on("/relay1/on", HTTP_GET, relay1On);
  server.on("/relay1/off", HTTP_GET, relay1Off);
  server.on("/relay2/on", HTTP_GET, relay2On);
  server.on("/relay2/off", HTTP_GET, relay2Off);
  server.on("/relay3/on", HTTP_GET, relay3On);
  server.on("/relay3/off", HTTP_GET, relay3Off);
  server.on("/relay4/on", HTTP_GET, relay4On);
  server.on("/relay4/off", HTTP_GET, relay4Off);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  timeClient.update();
  server.handleClient();
}

void handleRoot() {
  String relay1State = (digitalRead(relay1) == HIGH) ? "OFF" : "ON";
  String relay2State = (digitalRead(relay2) == HIGH) ? "OFF" : "ON";
  String relay3State = (digitalRead(relay3) == HIGH) ? "OFF" : "ON";
  String relay4State = (digitalRead(relay4) == HIGH) ? "OFF" : "ON";

  String formattedTime = timeClient.getFormattedTime();
  
  // Read the water level sensor value
  int waterLevel = analogRead(waterSensorPin);

  // Determine the water level status based on thresholds
  String waterLevelStatus;
  String waterLevelColor;
  if (waterLevel > 700) {
    waterLevelStatus = "High";
    waterLevelColor = "green";
  } else if (waterLevel > 300) {
    waterLevelStatus = "Medium";
    waterLevelColor = "yellow";
  } else {
    waterLevelStatus = "Low";
    waterLevelColor = "red";
  }

  // Map water level to a percentage (0-100%)
  int waterLevelPercentage = map(waterLevel, 0, 1023, 0, 100);

  String html = "<html>\
<head>\
  <title>Plant ICU</title>\
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\
  <meta http-equiv='refresh' content='1'>  <!-- Refresh the page every 1 second -->\
  <style>\
    body { font-family: Arial, sans-serif; background-color: #f7f7f7; margin: 0; padding: 0; }\
    h1 { text-align: center; color: #333; margin-top: 50px; }\
    .container { width: 100%; max-width: 600px; margin: auto; padding: 20px; background: #fff; border-radius: 10px; box-shadow: 0 0 10px rgba(0, 0, 0, 0.1); }\
    .clock { text-align: center; font-size: 24px; margin-bottom: 20px; }\
    .switch-container { margin-bottom: 15px; display: flex; align-items: center; justify-content: space-between; }\
    .switch { position: relative; display: inline-block; width: 60px; height: 34px; }\
    .switch input { opacity: 0; width: 0; height: 0; }\
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px; transition: .4s; }\
    .slider:before { position: absolute; content: ''; height: 26px; width: 26px; left: 4px; bottom: 4px; background: white; border-radius: 50%; transition: .4s; }\
    input:checked + .slider { background-color: #4CAF50; }\
    input:checked + .slider:before { transform: translateX(26px); }\
    label { font-size: 18px; }\
    .progress-bar { width: 100%; background-color: #f3f3f3; border-radius: 5px; height: 25px; margin-bottom: 20px; }\
    .progress-bar div { height: 100%; border-radius: 5px; transition: width 0.5s; }\
  </style>\
</head>\
<body>\
  <div class='container'>\
    <h1>Relay Control</h1>\
    <div class='clock'>Current Time (GST): " + formattedTime + "</div>\
    <div class='switch-container'>\
      <label>Water Level: " + waterLevelStatus + "</label>\
    </div>\
    <div class='progress-bar'>\
      <div style='width:" + String(waterLevelPercentage) + "%; background-color:" + waterLevelColor + ";'></div>\
    </div>\
    <div class='switch-container'>\
      <label>Water Pump</label>\
      <label class='switch'>\
        <input type='checkbox' " + String((relay1State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay1/" + String((relay1State == "ON" ? "off" : "on")) + "'\">\
        <span class='slider'></span>\
      </label>\
    </div>\
    <div class='switch-container'>\
      <label>Fertilizer Pump</label>\
      <label class='switch'>\
        <input type='checkbox' " + String((relay2State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay2/" + String((relay2State == "ON" ? "off" : "on")) + "'\">\
        <span class='slider'></span>\
      </label>\
    </div>\
    <div class='switch-container'>\
      <label>Light</label>\
      <label class='switch'>\
        <input type='checkbox' " + String((relay3State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay3/" + String((relay3State == "ON" ? "off" : "on")) + "'\">\
        <span class='slider'></span>\
      </label>\
    </div>\
    <div class='switch-container'>\
      <label>Relay 4</label>\
      <label class='switch'>\
        <input type='checkbox' " + String((relay4State == "ON" ? "checked" : "")) + " onclick=\"location.href='/relay4/" + String((relay4State == "ON" ? "off" : "on")) + "'\">\
        <span class='slider'></span>\
      </label>\
    </div>\
    <div class='switch-container'>\
      <label>Total Water Dispensed (ml): " + String(totalWaterDispensed) + "</label>\
    </div>\
    <div class='switch-container'>\
      <label>Total Fertilizer Dispensed (ml): " + String(totalFertilizerDispensed) + "</label>\
    </div>\
  </div>\
</body>\
</html>";

  server.send(200, "text/html", html);
}

// Relay control functions for water pump
void relay1On() {
  digitalWrite(relay1, LOW);  // Turn on the water pump
  startTimeWater = millis();  // Store the start time for water pump
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay1Off() {
  digitalWrite(relay1, HIGH);  // Turn off the water pump
  unsigned long elapsedTime = millis() - startTimeWater;  // Time the pump was on
  totalWaterDispensed += (elapsedTime / 1000.0) * 10;  // Add to the water dispensed counter (ml)
  server.sendHeader("Location", "/");
  server.send(303);
}

// Relay control functions for fertilizer pump
void relay2On() {
  digitalWrite(relay2, LOW);  // Turn on the fertilizer pump
  startTimeFertilizer = millis();  // Store the start time for fertilizer pump
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay2Off() {
  digitalWrite(relay2, HIGH);  // Turn off the fertilizer pump
  unsigned long elapsedTime = millis() - startTimeFertilizer;  // Time the pump was on
  totalFertilizerDispensed += (elapsedTime / 1000.0) * 10;  // Add to the fertilizer dispensed counter (ml)
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay3On() {
  digitalWrite(relay3, LOW);  // Turn on light
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay3Off() {
  digitalWrite(relay3, HIGH);  // Turn off light
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay4On() {
  digitalWrite(relay4, LOW);  // Turn on relay 4
  server.sendHeader("Location", "/");
  server.send(303);
}

void relay4Off() {
  digitalWrite(relay4, HIGH);  // Turn off relay 4
  server.sendHeader("Location", "/");
  server.send(303);
}
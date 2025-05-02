#include "thingProperties.h"
#include <HTTPClient.h>

//Raspberry Pi local ip addr: 10.251.74.63
//Raspberry Pi backend server runs on port 3000
const char* serverUrl = "http://10.251.74.63:3000/api/sensor-data";

void setup() {
  // Initialize serial
  Serial.begin(9600);
  delay(1500); 

  // Initialize Arduino IoT Cloud properties and connection
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Attempting connection to Wi-Fi");
  }
  
  Serial.println("Connected to Wi-Fi");
}

void loop() {
  ArduinoCloud.update();
  
  // Send sensor data to MongoDB every 5 seconds (non-blocking)
  static unsigned long lastSend = 0;
  if (millis() - lastSend >= 5000) {
    sendSensorDataToMongoDB();
    lastSend = millis();
  }
}


void sendSensorDataToMongoDB(){
    if (WiFi.status() == WL_CONNECTED) {
      // Example sensor data (replace with actual data from ESP32s)
      float sensorValue = 25.5; // Simulated temperature value (Replace with value like eSP32_1_LSV)
      
      // Creating JSON payload
      String jsonPayload = "{\"sensorType\": \"temperature\", \"value\": " + String(sensorValue) + "}";
      
      // Sending HTTP POST request
      HTTPClient http;
      http.begin(serverUrl);
      http.addHeader("Content-Type", "application/json");
      
      int httpCode = http.POST(jsonPayload);
      
      if (httpCode == HTTP_CODE_CREATED) {
        String response = http.getString();
        Serial.println("Server response: " + response);
      } 
      else {
        Serial.println("HTTP error: " + String(httpCode));
      }
      
      http.end();
  }
}


// Motion Timeout Value Update (if you allow ESP32-1 to change it)
void onESP321MTVChange() {
  Serial.print("Motion Timeout Updated to: ");
  Serial.print(ESP32_1_MTV);
  Serial.println(" minutes");
}


// Remote Power Management: Main controller can manage ESP32_1
void onESP321PWRChange() {
  Serial.print("Power Status Changed for ESP32_1: ");
  Serial.println(ESP32_1_PWR ? "ON" : "OFF");
  
  if (ESP32_1_PWR) {
    Serial.println("ESP32_1 was remotely powered ON.");
    // Optional: You could set some variables or reset values
  } else {
    Serial.println("ESP32_1 was remotely powered OFF.");
    // Optional: Tell other devices to go idle
  }
}
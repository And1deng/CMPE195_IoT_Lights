#include "arduino_secrets.h"
#include "thingProperties.h"
#include <Adafruit_TSL2591.h>
#include <WiFi.h>

#define LSENSOR_PIN 21

Adafruit_TSL2591 Lsensor = Adafruit_TSL2591(2591);

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PASS);
  while (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  Wire.begin();  // Default SDA=21, SCL=22 on most ESP32 boards

  connectToWiFi();

  WiFi.setSleep(true); //Modem sleep mode?

  if (!Lsensor.begin()) {
    Serial.println("Error: TSL2591 sensor not found. Check wiring.");
    while (1);  // Stop execution if sensor is not found
  }

  Lsensor.setGain(TSL2591_GAIN_MED);
  Lsensor.setTiming(TSL2591_INTEGRATIONTIME_300MS);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectToWiFi();
  }

  ArduinoCloud.update();

  if(ESP32_1_PWR){ //Board is enabled
      sensors_event_t light_detection;
      Lsensor.getEvent(&light_detection);
    
      if (light_detection.light == 0 || isnan(light_detection.light)) {
        ESP32_1_LSV = 0.000;
      } else {
        ESP32_1_LSV = light_detection.light;
      }
    
      Serial.print("Light Level (lux): ");
      Serial.println(light_detection.light);

      delay(500); //Check lux every 1 sec
    }
  else{
    Serial.print("ESP32_1_PWR is off.\n");
    delay(500); //Check for PWR every 0.5 sec
  }
}

void onESP321PWRChange() {
  Serial.print("ESP32_1_PWR changed to: ");
  Serial.println(ESP32_1_PWR ? "ON" : "OFF");
}


/*
  Since ESP321LSV is READ_WRITE variable, onESP321LSVChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onESP321LSVChange()  {
  // Add your code here to act upon ESP321LSV change
}
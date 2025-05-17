#include "arduino_secrets.h"
#include "thingProperties.h"
#include <WiFi.h>
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include <time.h>

#define PIR_PIN 21 

bool motion_detected = false;
int motionState = LOW;
unsigned long motion_start_time = 0;
unsigned long motion_timeout_duration = ESP32_2_MTV * 60UL * 1000UL; //timeout in minutes


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

  pinMode(PIR_PIN, INPUT);
  connectToWiFi();

  WiFi.setSleep(true);

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

  // Get current formatted time string
  unsigned long epochTime = ArduinoCloud.getLocalTime();
  time_t rawtime = (time_t)epochTime;
  struct tm* timeinfo = localtime(&rawtime);
  char timeString[25];
  strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", timeinfo);

  if (ESP32_2_PWR) {
    motionState = digitalRead(PIR_PIN);

    if (motionState == HIGH && !motion_detected) {
      Serial.print("[");
      Serial.print(timeString);
      Serial.println("] Motion detected!");

      motion_detected = true;
      ESP32_2_Motion = true;
      motion_start_time = millis();
      ArduinoCloud.update();
    }

    else if (motion_detected) {
      if (motionState == HIGH) {
        motion_start_time = millis(); // reset timer while motion is still active
      }

      if ((millis() - motion_start_time) > motion_timeout_duration) {
        Serial.print("[");
        Serial.print(timeString);
        Serial.println("] Motion timeout expired. No motion now.");

        motion_detected = false;
        ESP32_2_Motion = false;
        ArduinoCloud.update();
      }
    }
    delay(1000); //repeats every 1 sec
  } 
  else {
    Serial.print("ESP32_1_PWR is off.\n");
    motion_detected = false;
    ESP32_2_Motion = false;
    delay(500); //Check for PWR every 0.5 sec
  }
  Serial.println(motionState);
}


void onESP322MTVChange() {
  motion_timeout_duration = ESP32_2_MTV * 60UL * 1000UL; //changes in minutes
  Serial.print("Motion timeout updated to ");
  Serial.print(ESP32_2_MTV);
  Serial.println(" seconds.");
}

void onESP322PWRChange() {
  Serial.print("ESP32_2_PWR changed to: ");
  Serial.println(ESP32_2_PWR ? "ON" : "OFF");
}

#include "arduino_secrets.h"
#include "thingProperties.h"
#include <WiFiS3.h>
#include "RTC.h"
#include <ArduinoHttpClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
#include <time.h>


const char* serverIP = "172.20.10.4"; // Pi's IP
const int serverPort = 3000;
unsigned long lastSensorUpdate = 0;
const long sensorInterval = 10000;
WiFiClient client;

//for lighting
int previous_brightness = -1;

//ifttt
const char* iftttServer = "maker.ifttt.com";
const int iftttPort = 443; // HTTPS
const char* iftttKey = "fyltaMBpLAUirNCOQVxDLXJQdTZrO6uOkP2FgrGV3BC";
const char* eventOn = "turn_on";
const char* eventOff = "turn_off";
const char* event10 = "brightness_10";
const char* event20 = "brightness_20";
const char* event30 = "brightness_30";
const char* event40 = "brightness_40";
const char* event50 = "brightness_50";
const char* event60 = "brightness_60";
const char* event70 = "brightness_70";
const char* event80 = "brightness_80";
const char* event90 = "brightness_90";
const char* event100 = "brightness_100";

//For PI
WiFiSSLClient sslClient;
HttpClient iftttClient(sslClient, iftttServer, iftttPort);

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

void triggerIFTTTWebhook(const char* eventName, const char* key) {
  String path = "/trigger/" + String(eventName) + "/with/key/" + String(key);
  Serial.print("Sending IFTTT request: ");
  Serial.println(path);
  iftttClient.get(path);
  int statusCode = iftttClient.responseStatusCode();
  String response = iftttClient.responseBody();
  Serial.print("IFTTT Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}

void on()      {triggerIFTTTWebhook(eventOn, iftttKey); }
void off()     {triggerIFTTTWebhook(eventOff, iftttKey); }
void ten()     {brightness_Level = 10; triggerIFTTTWebhook(event10, iftttKey); }
void twenty()  {brightness_Level = 20; triggerIFTTTWebhook(event20, iftttKey); }
void thirty()  {brightness_Level = 30; triggerIFTTTWebhook(event30, iftttKey); }
void forty()   {brightness_Level = 40; triggerIFTTTWebhook(event40, iftttKey); }
void fifty()   {brightness_Level = 50; triggerIFTTTWebhook(event50, iftttKey); }
void sixty()   {brightness_Level = 60; triggerIFTTTWebhook(event60, iftttKey); }
void seventy() {brightness_Level = 70; triggerIFTTTWebhook(event70, iftttKey); }
void eighty()  {brightness_Level = 80; triggerIFTTTWebhook(event80, iftttKey); }
void ninety()  {brightness_Level = 90; triggerIFTTTWebhook(event90, iftttKey); }
void hundred() {brightness_Level = 100; triggerIFTTTWebhook(event100, iftttKey); }

String getRTCTimestamp() {
  RTCTime currentTime;
  RTC.getTime(currentTime);
  char buffer[25];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02dT%02d:%02d:%02dZ",
           currentTime.getYear(), currentTime.getMonth(), currentTime.getDayOfMonth(),
           currentTime.getHour(), currentTime.getMinutes(), currentTime.getSeconds());
  return String(buffer);
}

void sendSensorData(String sensorType, String data) {
  if(WiFi.status() != WL_CONNECTED){
    return;
  }
  if (client.connect(serverIP, serverPort)) {
    Serial.println("Sending Sensor Data.");
    String payload = "{\"sensorType\":\"" + sensorType + "\"," +
                     "\"data\":" + data + "," +
                     "\"timestamp\":\"" + getRTCTimestamp() + "\"}";
    client.println("POST /api/sensor-data HTTP/1.1");
    client.println("Host: " + String(serverIP) + ":" + String(serverPort));
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println("Connection: close");
    client.println();
    client.println(payload);
    delay(10);
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
  } else {
    Serial.println("Pi Connection failed!");
  }
}

void updateSensors() {
  float lightValue = module_1_Light_Val;
  if (isnan(lightValue)) lightValue = 0.000;
  sendSensorData("light", "{\"value\":" + String(lightValue) + "}");
  bool motionDetected = module_2_Motion_Val;
  sendSensorData("motion", "{\"detected\":" + String(motionDetected ? "true" : "false") + "}");
}

void adjustBrightness(int set_to) {
  switch (set_to) {
    case 0: off(); return;
    case 10: ten(); break;
    case 20: twenty(); break;
    case 30: thirty(); break;
    case 40: forty(); break;
    case 50: fifty(); break;
    case 60: sixty(); break;
    case 70: seventy(); break;
    case 80: eighty(); break;
    case 90: ninety(); break;
    case 100: hundred(); break;
    default: return;
  }

  delay(1500); // Allow time for change
}

void lightAutoAdjust(int threshold) {
  Serial.print("Target threshold: ");
  Serial.println(threshold);

  float upper_bound = threshold * 1.3;
  int auto_brightness;
  
  if (module_1_Light_Val >= threshold && module_1_Light_Val <= upper_bound) {
    Serial.println("Threshold met.");
    return;
  }

  if (!light_Status) {
    // Light is off — calculate starting brightness
    light_Status = true;
    int needed_lumens = threshold - module_1_Light_Val;
    auto_brightness = ((needed_lumens + 7) / 8);
    auto_brightness = ((auto_brightness + 9) / 10) * 10;

    if (auto_brightness > 100) auto_brightness = 100;
    if (auto_brightness < 0) auto_brightness = 0;

    previous_brightness = auto_brightness;
    adjustBrightness(auto_brightness);
  } else {
    // Light is on — use last known value (rounded)
    auto_brightness = ((previous_brightness + 5) / 10) * 10;
  }

  while (auto_brightness >= 0 && auto_brightness <= 100) {
    ArduinoCloud.update();
    delay(1000);

    Serial.print("Lux reading: ");
    Serial.println(module_1_Light_Val);

    if (module_1_Light_Val >= threshold && module_1_Light_Val <= upper_bound) {
      Serial.println("Threshold met.");
      return;
    }

    if (module_1_Light_Val > upper_bound) {
      auto_brightness -= 10;
      if (auto_brightness < 0) auto_brightness = 00;
      Serial.print("Decreasing brightness to: ");
    } else if (module_1_Light_Val < threshold) {
      auto_brightness += 10;
      if (auto_brightness > 100) auto_brightness = 100;
      Serial.print("Increasing brightness to: ");
    } else {
      break;
    }

    Serial.println(auto_brightness);
    previous_brightness = auto_brightness;
    adjustBrightness(auto_brightness);
  }

  ArduinoCloud.update();
}


void automatic_mode(int hour){
  if(module_2_Motion_Val){
      Serial.println("Motion detected\n");
      ArduinoCloud.update();

      //Check time of day to determine light threshold to use
      if (hour >= 5 && hour < 14) {
        Serial.print("adjusting light to day.\n");
        lightAutoAdjust(day_Light_Threshold);
      } 
      else if (hour >= 14 && hour < 19) {
        Serial.print("adjusting light to afternoon.\n");
        lightAutoAdjust(afternoon_Light_Threshold);
      } 
      else if (hour >= 19 || hour < 5) {
        Serial.print("adjusting light to night.\n");
        lightAutoAdjust(night_Light_Threshold);
      }
      else { //Backup default
        Serial.print("Using default threshold of 500.\n");
        lightAutoAdjust(500);
      }
    }
    else{ //Turn light off if no motion
      Serial.println("No Motion\n");
      if(light_Status){
        light_Status = false;
        previous_brightness = -1;
        brightness_Level = 0;
        off();
      }
    }
}

void manual_mode(){
    int manual_brightness = manual_Light_On.getBrightness();
    brightness_Level = ((manual_brightness + 5) / 10) * 10;
    if (brightness_Level != 0 && (brightness_Level != previous_brightness || !light_Status)) {
      light_Status = true;
      previous_brightness = brightness_Level;
      adjustBrightness(brightness_Level);
    } 
    else if(brightness_Level == 0 || !manual_Light_On.getSwitch()){
      if(light_Status){
        light_Status = false;
        previous_brightness = -1;
        brightness_Level = 0;
        off();
      }
    }
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Reset all values on startup
  module_1_Light_Val = 0.0;
  manual_Light_On.setSwitch(false);
  manual_Light_On.setBrightness(0);
  afternoon_Light_Threshold = 400;
  brightness_Level = 0;
  day_Light_Threshold = 500;
  night_Light_Threshold = 300;
  enable_automatic = true;
  hub_PWR = true;
  Light_PWR = false;
  light_Status = false;
  module_2_Motion_Val = false;
  motion_PWR = false;

  // WiFi + Cloud connection
  connectToWiFi();

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // Wait for connection to Arduino IoT Cloud
  Serial.println("Waiting for Arduino IoT Cloud connection...");
  while (!ArduinoCloud.connected()) {
    ArduinoCloud.update();
    delay(500);
  }
  Serial.println("Connected to Arduino IoT Cloud.");
}

void loop() {
  //Setup WIFI
  if (WiFi.status() != WL_CONNECTED || WiFi.localIP() == IPAddress(0, 0, 0, 0)) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectToWiFi();
  }

  //Setup Time
  unsigned long epochTime = ArduinoCloud.getLocalTime();
  time_t rawtime = (time_t)epochTime;
  struct tm* timeinfo = localtime(&rawtime);
  int hour = timeinfo->tm_hour;
  int day = timeinfo->tm_mday;

  
  ArduinoCloud.update();

  //Pi data timer
  if(millis() - lastSensorUpdate >= sensorInterval) {
    updateSensors();
    lastSensorUpdate = millis();
  }

  //Lighting Loop
  if (manual_Light_On.getSwitch()) {  // 1. Manual Mode gets top priority
    enable_automatic = false;
  
    Serial.print("----MANUAL MODE----\n");
    Serial.print("Current day: ");
    Serial.println(day);
    Serial.print("Current hour: ");
    Serial.println(hour);
    Serial.print("Current light level: ");
    Serial.println(brightness_Level);
  
    manual_mode();
  }
  else if (scheduler.isActive()) {    // 2. Scheduler-controlled auto mode
    Serial.print("----SCHEDULER AUTOMATIC MODE----\n");
    Serial.print("Current day: ");
    Serial.println(day);
    Serial.print("Current hour: ");
    Serial.println(hour);
    Serial.print("Current light level: ");
    Serial.println(brightness_Level);
  
    automatic_mode(hour);
  }
  else if (enable_automatic) {        // 3. Default fallback auto mode
    //turn manual off
    manual_Light_On.setSwitch(false);
    manual_Light_On.setBrightness(0);
  
    Serial.print("----DEFAULT AUTOMATIC MODE----\n");
    Serial.print("Current day: ");
    Serial.println(day);
    Serial.print("Current hour: ");
    Serial.println(hour);
    Serial.print("Current light level: ");
    Serial.println(brightness_Level);
  
    automatic_mode(hour);
  }
  else {  // All modes off
    Serial.print("----ALL MODES OFF----\n");
    Serial.print("Current day: ");
    Serial.println(day);
    Serial.print("Current hour: ");
    Serial.println(hour);
    if(light_Status){
      light_Status = false;
      previous_brightness = -1;
      brightness_Level = 0;
      off();
    }
  }


  delay(500);
}

void onLightPWRChange()  {
  // Add your code here to act upon LightPWR change
}

void onMotionPWRChange()  {
  // Add your code here to act upon MotionPWR change
}

void onLightStatusChange()  {
  // Add your code here to act upon LightStatus change
}

void onNightLightThresholdChange()  {
  // Add your code here to act upon NightLightThreshold change
}

void onDayLightThresholdChange()  {
  // Add your code here to act upon DayLightThreshold change
}

void onAfternoonLightThresholdChange()  {
  // Add your code here to act upon AfternoonLightThreshold change
}

void onManualLightOnChange()  {
  enable_automatic = false;
}

void onModule2MotionValChange()  {
  // Add your code here to act upon Module2MotionVal change
}
/*
  Since BrightnessLevel is READ_WRITE variable, onBrightnessLevelChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onBrightnessLevelChange()  {
  // Add your code here to act upon BrightnessLevel change
}

void onEnableAutomaticChange()  {
  // Add your code here to act upon EnableAutomatic change
}

void onSchedulerChange()  {
  // Add your code here to act upon Scheduler change
}
/*
  Since Module1LightVal is READ_WRITE variable, onModule1LightValChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onModule1LightValChange()  {
  // Add your code here to act upon Module1LightVal change
}
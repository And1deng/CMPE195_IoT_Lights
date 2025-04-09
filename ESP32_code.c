#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "";
const char* pass = "";

//For Server
WebServer server(80); //http server

//For PIR Sensor
const int PIRpin = //ADD HERE; 
bool PIRsignal = false;

void PIR_sensor(){
  PIRsignal = digitalRead(PIRpin);
  String reading = String("{\"PIRreading\": " + (PIRsignal ? "true" : "false") + "}");
  server.send(200, "application/json", response);
  Serial.println("Sent motion state: " + response + "\n");
}

void setup() {
  //Setup wifi
  Wifi.begin(ssid, password);
  Serial.begin(115200);

  while(Wifi.status() != WL_CONNECTED){
    delay(1000);
    Serial.println("Trying to connect to");
    Serial.println(ssid + "\n");
  }

  Serial.println("ESP32 IP: " + Wifi.localIP().toString() + "\n");

  //Setup PIR
  pinMode(PIRpin, INPUT);

  //Initiate Server
  server.on("/sensor", HTTP_GET, PIR_sensor);
  server.begin();
}

void loop() {
  server.handleClient();
}

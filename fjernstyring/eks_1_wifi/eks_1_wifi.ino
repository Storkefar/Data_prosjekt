//library imports
#include <WiFi.h>
 
// SSID & Password for router
const char* ssid = "Snorre's iPhone";  // Enter your SSID here
const char* password = "abcd1234";  //Enter your Password here

void setup() {
  //Begins serial monitor with a baud rate of 9600
  Serial.begin(9600);
  
  Serial.println("Try Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); // Connect to your wi-fi modem
 
  // Check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected successfully");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());  //Show ESP32-IP on serialmonitor

}

void loop() {
  // put your main code here, to run repeatedly:

}

#include <WiFi.h>

// SSID & Password for router
const char* ssid = "Snorre's iPhone";
const char* password = "abcd1234";

void setup() {
  Serial.begin(9600);
  delay(100);

  /*
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();

  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  

  WiFi.begin(ssid, password); // Connect to your wi-fi modem
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Establishing connection to WiFi...");
    delay(1000);
  }
      
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  */
  
}

/*
Example function. When the ESP32 receives a message from the internet connection to drive,
this function tells the car to drive trough serial communication (UART)
*/
void Drive(bool Direction) {
  if(Direction) {
    Serial.print("w"); //Drive forwards
  } else {
    Serial.print("s"); //Drive backwards
  }
}

void loop() {
  Serial.print("w");
}

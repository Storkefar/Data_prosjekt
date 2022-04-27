#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.20.10.2";

#define ledPin 26

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  delay(2000);
  int attempts = 0;
    
  Serial.begin(115200);
  WiFi.begin(ssid, password);
   
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      attempts++;
      if (attempts > 20) {
        ESP.restart();
        }
      Serial.print(".");
  }
   
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP32Client")) {
      Serial.println("connected"); 
 
    } else {
 
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
 
    }
  }
  client.subscribe("fjernstyring/mottaker");

}

void callback(char* topic, byte* message, unsigned int length) {
  /*
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  */
  String movement;

  for (int i = 0; i < length; i++) {
      //Serial.print((char)message[i]);
      movement += (char)message[i];
    }

  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "fjernstyring/mottaker") {
    //Serial.print("starter fjernstyring av Zumo");
    if (movement == "2") {
        Serial.print("w");  
      }
    else if (movement == "5") {
        Serial.print('s');
      }
    else if (movement == "4") {
        Serial.print('a');
      }
    else if (movement == "6") {
        Serial.print('d');
      }
    else if (movement =="*") {
        Serial.print('x');
      }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("fjernstyring/mottaker");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  if(!client.connected()) {
      reconnect();
    }
  client.loop();
  
}

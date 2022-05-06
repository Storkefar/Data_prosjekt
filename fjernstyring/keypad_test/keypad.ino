#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
{'q','w','e'},
{'a','s','d'},
{'7','8','9'},
{'*','0','#'}
};
byte rowPins[ROWS] = {32,33,25,26};//connect to the row pinouts of the keypad
byte colPins[COLS] = {27,14,12,13}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Replace the next variables with your SSID/Password combination
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);

void setup(){
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
  //client.setCallback(callback);

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("fjernstyring/keypad");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop(){
  if(!client.connected()){
      reconnect();
    }
  client.loop();
  char key = keypad.getKey();  //Variable to store pressed key
  if (key != NO_KEY){
    //konverterer char til uint8_t for å kunne publishe
    uint8_t key_send = key;
    Serial.println(key_send); //Prints the key in serial monitor
    //publisher keypress til fjernstyring/keypad
    client.beginPublish("fjernstyring/keypad", 1 ,false);
    client.write(key);
    client.endPublish();
  }

}

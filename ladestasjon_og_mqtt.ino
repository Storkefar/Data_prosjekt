#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino.h>
#include <Zumo32U4.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(-1);
int bankKonto = 100;
uint8_t bilID = 25;
int prisListe[] = {1, 2, 3}; // read from server
Zumo32U4LCD display;
Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
uint8_t battery_level = 100;
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  
  // Clear the buffer.
  display.clearDisplay();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  /*
   * Parameter 1, Full ladning per prosent: 1 nok
   * Parameter 2, Ladning til manuelt stop: 1 nok per prosent
   * parameter 3, Lade et valgt antall prosent: 1 nok per prosent
   */

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
    //prisliste needs to be read from server  
    // bankKonto needs to be read from server
  }
  Serial.println();
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("Ladestasjon")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("ladestasjon");
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
  if (!client.connected()) {
    reconnect();
  }
  /*
  client.loop();
  uint8_t tall = 123;
  client.beginPublish("ladestasjon", 1, false);
  client.write(tall);
  client.endPublish();
  delay(3000);
  */
  TypeLadning();
}

void BestilleLadning(){
  int typeLadning = 0;
  callback();
  if(buttonA.isPressed()){
    typeLadning = 1;
  }
  else if(buttonB.isPressed()){
    typeLadning = 2;
  }
  else if(buttonC.isPressed()){
      typeLadning = 3;
  }
  raporter(typeLadning);
  
  switch(typeLadning){
      case 1: 
      {
          //Fulladning, batteriet lades opp til 100%
          for(uint8_t i = battery_level; i<=100;i++){
            bankKonto -= prisListe[0];
            if(bankKonto < 0){
              break;
            } 
            SkyserverOLED(i);     
            delay(1000);
          }
        }//end if
        break;
      }
      case 2: 
      {
        //Ladning til brukeren stopper
        for(uint8_t i = battery_level; i<=100;i++){
          bankKonto -= prisListe[1];
          if(buttonB.isPressed() || bankKonto < 0){
            break;
          }
          delay(1000);  
          SkyserverOLED(i);  
        }
        break;
      }
      case 3: 
      {
        //Lade et valgt antall prosent
        //Eksempel: battery_level = 50%, LadeTil = 30% => blir ladet til 80%
        while (Serial.available() == 0){}
        int LadeTil = Serial.parseInt();
        LadeTil += battery_level;
        if(LadeTil > 100){
          LadeTil = 100; 
        }
        for(uint8_t i = battery_level; i<= LadeTil; i++){
          bankKonto -= prisListe[2];
          if(bankKonto < 0){
            break;
          }
          delay(1000);
          SkyserverOLED(i);            
        }
        break;
      }
  }
}

void SkyserverOLED(uint8_t i){
  //Vise på skyserveren
  client.beginPublish("ladestasjon", 1, false);
  client.write(i);
  client.endPublish();
  //Vise på Ladestasjonens oled
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(10,10);
  display.println(i);
  display.display();
  delay(3000);  
}
void raporter(int typeLadning){
  client.beginPublish("ladestasjon", 1, false);
  client.write(bilID);
  client.write(typeLadning); 
  client.endPublish();
}

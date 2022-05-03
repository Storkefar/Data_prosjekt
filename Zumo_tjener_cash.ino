int bankKonto = 100;
unsigned long arbeidsTid = 0;
unsigned long sluttTid = 0;

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino.h>
#include <Zumo32U4.h>
#include <SPI.h>


Adafruit_SSD1306 display(-1);
int bankKonto;
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);

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
  if (String(topic) == "bankUt") {
      bankKonto =
    }
  if (String(topic) == "ladestasjon") {
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
      client.subscribe("bankUt");
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
  tjeneCash();

}

void tjeneCash(){
  arbeidsTid = millis();
  
  //Vise på skyserveren
  client.beginPublish("ladestasjon", 1, false);
  client.write("Utfører oppgaven...");
  client.write("Trykk på knapp A for å stoppe");
  client.endPublish();
  
  
  //Venter til knapp A er trykket for å stoppe arbeidet. Derreter tjene penger ut ifra hvor lenge bilen har vært i jobb
  while(buttonA.waitForButton()){}
  sluttTid = arbeidsTid;
  uint8_t inntekt = sluttTid / 1000;
  bankKonto += inntekt;
  
  //Vise resultater på skyserveren
  client.beginPublish("", 1, false);
  client.write("Arbeidet stoppet");
  client.write("Penger tjente: ",inntekt);
  client.write("Bilen brukte: ", sluttTid, "sekunder");
  client.endPublish();
  
  
}

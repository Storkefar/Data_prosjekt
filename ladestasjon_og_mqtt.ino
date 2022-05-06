#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(-1);
//Deklarerer ulike variabler
uint8_t bankKonto;
uint8_t typeLadning = 0;
const int knappA = 2;
const int knappB = 4;
const int knappC = 18;
uint8_t bilID = 25;
uint8_t battery_level = 35;
//Kobler på wifi og skyserveren
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);
//Denne funksjonen sender data til skyserveren og oled skjermen til ladestasjonen
void statusDisplay(uint8_t i) {
  //Vise på skyserveren
  client.beginPublish("ladestasjon", 1, false);
  client.write(i);
  client.endPublish();
  //Vise på Ladestasjonens oled
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(i);
  display.display();
}
//Denne funksjonen sender bil-id og hvilken ladning den vil ha til skyserveren
void raporter(uint8_t typeLadning) {
  client.beginPublish("ladestasjon", 1, false);
  client.write(bilID);
  client.write(typeLadning);
  client.endPublish();
}

//en wifi setup funksjon, som lar oss koble til på wifi
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



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // initialize with the I2C addr 0x3C
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // Clear the buffer.
  display.clearDisplay();

  //declare buttons
  pinMode(knappA, INPUT);
  pinMode(knappB, INPUT);
  pinMode(knappC, INPUT);
}

//Callback funksjonen lar meg sende og motta data til skyserveren. Når jeg, for eksempel,
//bruker data om bank kontoen som ligger på skyserveren, kan jeg hente den, oppdatere den og sende tilbake
void callback(char* topic, byte* message, unsigned int length) {
  String msg;

  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }


  if (String(topic) == "bankUt") {
    bankKonto = msg.toInt();
    Serial.println(bankKonto);
    delay(5000);
    //bankKonto  = atoi (msg.substring(1, 3).c_str ());
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
      client.subscribe("bankUt");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(500);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
 // client.loop();
  BestilleLadning();
}
//"Hoved" funksjonen til ladestasjon modulen. Forkortet kan brukeren velge type ladning de vil ha, som lader batteriet på forskjellige måter og sender data til skyserveren.
void BestilleLadning() {
  //leser av knapper
  int read_knappA = digitalRead(knappA);
  int read_knappB = digitalRead(knappB);
  int read_knappC = digitalRead(knappC);
  
//En rekke if-tester som sjekker hvilken knapp er trykket inn for å bestemme type ladning brukeren vil ha
  if (read_knappA == HIGH) {
    typeLadning = 1;
    client.publish("bankIn", "fetch bank"); //henter data om banken fra skyserberen
    raporter(typeLadning); //raporterer bil-id
    client.loop();
  }
  else if (read_knappB == HIGH) {
    typeLadning = 2;
    client.publish("bankIn", "fetch bank");
    raporter(typeLadning);
    client.loop();
  }
  else if (read_knappC == HIGH) {
    typeLadning = 3;
    client.publish("bankIn", "fetch bank");
    raporter(typeLadning);
    client.loop();
  }
  //hoved swtich case til modulen. Ut ifra hvilken knapp/ladning modus er valgt, lades batteriet opp, penger blir trukket fra skyserveren, alt blir vist på skyserveren og oled
  switch (typeLadning) {
    case 1:
      {
        //Fulladning, batteriet lades opp til 100%
        for (uint8_t i = battery_level; i <= 100; i++) {
          bankKonto -= 1;
          if (bankKonto < 0) {
            break;
          }
          //viser på skyserver og oled
          statusDisplay(i);
          delay(2000);
          //oppdaterer penger i kontoen
          char kontoBuffer[10];
          String melding = String(-bankKonto);
          Serial.println(melding);
          melding.toCharArray(kontoBuffer, 10);
          client.publish("bankIn", kontoBuffer);
        }
      }//end if
      break;

    case 2:
      {
        //Ladning til brukeren stopper/ trykker på knappen igjen
        for (uint8_t i = battery_level; i <= 100; i++) {
          bankKonto -= 1;
          if ( read_knappB == HIGH || bankKonto < 0) {
            break;
          }
          delay(2000);
          //viser på skyserveren og oled
          statusDisplay(i);
          //oppdaterer bank kontoen på skyserveren
          char kontoBuffer[10];
          String melding = String(-bankKonto);
          melding.toCharArray(kontoBuffer, 10);
          client.publish("bankIn", kontoBuffer);
        }
        break;
      }
    case 3:
      {
        //Lade et valgt antall prosent
        //Eksempel: battery_level = 50%, LadeTil = 30% => blir ladet til 80%
        while (Serial.available() == 0) {}
        int LadeTil = Serial.parseInt();
        LadeTil += battery_level;
        if (LadeTil > 100) {
          LadeTil = 100;
        }
        for (uint8_t i = battery_level; i <= LadeTil; i++) {
          bankKonto -= 1;
          if (bankKonto < 0) {
            break;
          }
          delay(2000);
          // vises på skyserveren og oled
          statusDisplay(i);
          char kontoBuffer[10];
          //oppdaterer bank kontoen på skyserveren
          String melding = String(-bankKonto);
          melding.toCharArray(kontoBuffer, 10);
          client.publish("bankIn", kontoBuffer);
        }
        break;
      }

  }
  typeLadning = 0;
}

//inkluderer bibliotek
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// definerer ssid og passord
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";

// definerer mqtt server adresse
const char* mqtt_server = "172.20.10.2";

//lager en client som kan kobles til et spesifisert innternett IP adresse
WiFiClient espClient;
//lager en delvis initialisert klient forekomst
PubSubClient client(espClient);

void setup() {  
  //setter baud raten på serial til 9600 bits per sekund  
  Serial.begin(9600);
  //kjører funksjon for kobling til WiFi
  setup_WiFi();

  //konfigurer server detaljene
  client.setServer(mqtt_server, 1883);
  //setter en callback funksjon som kjøres når en melding mottas
  client.setCallback(callback);
  // funksjon for kobling til mqtt megler
  reconnect();     
  delay(500);
  //fetcher batterinivå fra fil på rpi
  // kan på denne måten lagre SW batteriet ladning eksternt når Zumo er avlsått
  client.publish("batteri", "fetch");
}

void setup_WiFi() {
  // definerer en tellevariabel
  int attempts = 0;
  //initialiserer WiFi bibliotekets nettverk innstillinger og gir nåverende status
  WiFi.begin(ssid, password);
  //while løkke som kjøres mens enheten ikke er koblet til WiFi
  while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      attempts++;
      //restarter esp dersom den bruker for lang tid på å koble til nettverket
      if (attempts > 20) {
        ESP.restart();
        }
      Serial.print(".");
  }
  // serial printer at enheten er koblet til nettverket og printer IP adressen
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());  
}
void reconnect() {
  // while løkke som kjøres til klienten er tilkoblet
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // prøver tilkobling
    if (client.connect("esp_fjernstyring")) {
      Serial.println("connected");
      // publisher til node-red dashbord at enheten er tilkoblet
      client.publish("connected", "connected");
      // abonnerer til kanalene enheten ønsker å lytte på
      client.subscribe("fj/kontroll");
      client.subscribe("fj/hastighet");
      client.subscribe("batteri/status");
      client.subscribe("dataIn");
      client.subscribe("stopFlow");
      client.subscribe("soppelstate");
    } else {
      // serial printer klienten sin status når den feilet tilkobling
      Serial.print("failed with rc=");
      Serial.print(client.state());
      delay (500);
    }
  }
}
// funksjon for lesing fra MQTT
void callback(char* topic, byte* message, unsigned int length) {
  String msg;

  //for løkke for å konstruere meldings string
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];  
  }
  String strTopic = String(topic);
  // sender topic og melding til zumo 
  if(strTopic == "fj/kontroll" || String(topic) == "dataIn" || String(topic) == "fj/hastighet" || strTopic == "stopFlow" || strTopic == "soppelstate" || strTopic == "batteri/status") {
    Serial.println(strTopic + "!" + msg); 
  } 
}
void loop() {
  // kjører setup_wifi dersom enheten kobles fra nettverket
  if (WiFi.status() != WL_CONNECTED) {
    setup_WiFi();
  }
  // kjører reconnect() dersom klienten mister forbindelse til MQTT serveren
  if (!client.connected()) {
    Serial.println("disconnected");
    reconnect();
  }

  // kjøres regelmessig for å tillate klienten å prossesere inkommende meldinger og vedlikeholde tilkobling til serveren
  client.loop();

  // lesning fra Zumo32U4--------------------------------------------------------------------
  //variabler for sending til Node-red
  String header = "";
  String msg = "";

  // topic og msg buffer for sending til Node-red
  char topicBuffer[40];
  char msgBuffer[40];

  // while løkke som kjøres når zumo sender data
  while (Serial.available() > 0) {
    //definerer en statisk meldingsbuffer
    static char melding[80];
    // variabel for indeks i melding
    static unsigned int melding_pos = 0; 

    // leser første byte i serial som en char
    char inByte = Serial.read();

    //så lenge meldingen ikke avlsuttes vil meldingsbufferen bli oppdatert med dataen fra serial
    if(inByte != '\n' && melding_pos < 39) {
        melding[melding_pos] = inByte;
        melding_pos++;
    } 
    //kjører når meldingen er ferdig konstruert
    else {
      // definerer boolean for å kunne splitte header og msg fra melding
      bool headerBool = true;
      //for løkke som går igjennom meldingsindeksene
      for (int i = 0; i < melding_pos -1; i++) {
        //dersom split statement "!" blir påtrykt vet koden at header er ferdig konstruert
        if (melding[i] == '!') {
          //setter header bool til false
          headerBool = false;
          //kjører continue for å ikke legge til "!" i msg buffer
          continue; 
        } 
        //legger til melding med rett indeksering i header- og msg string
        if (headerBool) {
          header += melding[i];
        } else {
          msg += melding[i];  
        }
      }
      //setter melding indeks posisjon til null
      melding_pos = 0;
    }
    //kopierer header og msg string til respektive char buffere
    header.toCharArray(topicBuffer, 40);
    msg.toCharArray(msgBuffer, 40);
    // publiserer til node red dersom
    if (header == "stats/hastighet") {
      client.publish(topicBuffer, msgBuffer);
    } else if (header == "stats/avstand") {
      client.publish(topicBuffer, msgBuffer);  
    } else if (header == "stats/hastighet_gj" || header == "stats/maks" || header == "stats/70aM") {
      client.publish(topicBuffer, msgBuffer);  
    } else if (header == "soppel") {
      client.publish(topicBuffer, msgBuffer);  
    } else if (header == "bankIn") {
      client.publish(topicBuffer, msgBuffer);
    } else if (header == "batteri") {
      client.publish(topicBuffer, msgBuffer);
    }
  }      
}

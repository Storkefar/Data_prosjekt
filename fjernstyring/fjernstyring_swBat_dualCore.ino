#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "Trym sin iPhone";
const char* password = "12345678";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "172.20.10.2";

WiFiClient espClient;
PubSubClient client(espClient);

TaskHandle_t Fjernstyring;

String header;

void setup() {    
  Serial.begin(9600);

  setup_WiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
  reconnect();     
  
  xTaskCreatePinnedToCore(
      fjernstyring_func,   // Task function. 
      "Fjernstyring",     // name of task. 
      10000,       // Stack size of task 
      NULL,        // parameter of the task 
      5,           // priority of the task 
      &Fjernstyring,      // Task handle to keep track of created task 
      0);          // pin task to core 0 
     
  delay(500);

  //fetcher batterinivå fra fil på rpi
  // kan på denne måten lagre SW batteriet ladning eksternt når Zumo er avlsått
  client.publish("stats/batteri", "fetch");

}

void setup_WiFi() {
  int attempts = 0;
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
}

void callback(char* topic, byte* message, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  //Serial.println();
  if (String(topic) == "fjernstyring/mottaker") {
    Serial.println(msg);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp_fjernstyring")) {
      Serial.println("connected");
      // Subscribe
      client.publish("connected", "connected");
      client.subscribe("fjernstyring/mottaker");
      client.subscribe("stats/hastighet");
      client.subscribe("batteri/status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void fjernstyring_func(void * parameter) {
  for(;;) {
    if(!client.connected()) {
      reconnect();
    }
    client.loop();
    vTaskDelay(10);    
  }
}


void loop() {
  //runs on core 1
  if (Serial.available() > 0) {
    static char melding[12];
    static unsigned int melding_pos = 0; 
    uint8_t number;

    char inByte = Serial.read();
    //Serial.print(inByte);
    
    if(inByte != '\n' && melding_pos < 11) {
        melding[melding_pos] = inByte;
        melding_pos++;
    } else {
        melding[melding_pos] ='\0';
  
        if (atoi(melding) > 0) {
          number = atoi(melding);  
        } else {
          header = String(melding);
        }
        melding_pos = 0;
    }

    if (String(melding) == "cmS") {
      client.publish("stats/hastighet", "cmS tallverdi");  
    }
    if (String(melding) == "avstand") {
      client.publish("stats/avstand", "avstand");
 
    }

  }
  
  //vTaskDelay(10);

  //--------------------------- sw battery
  
     
}

#include <Wire.h> //wire.h biblioteket
#include <Zumo32U4.h>//zumo biblioteket
const int ENA = 9; //Motor 1
const int IN1 = 15; //Wheel direction 1
const int ENB = 10; //Motor 2
const int IN2 = 16; //Wheel direction 2

bool mode=true; //Variabel som bestemmer om bilen skal kjøre fort eller sakte
int motorL=0; //Variabel som bestemmer farten til det venstre hjulet
int motorR=0; //Variabelen som bestemmer farten til det høyre hjulet
char kommando;

Zumo32U4Encoders encoders;//Gir encoders en class
Zumo32U4Motors motors; //Gir motors en class
Zumo32U4LineSensors lineSensors; //Gir lineSensors en class

//globale variabler til dishcarge funksjonen
int hiddenChargeTotalTime = 0;
int emergencyChargeStatus = 0; 
//globale variabler til regularCharge funksjonen

//globale variabler flere funksjoner er avhengig av

float batteryCapacityAndPercentage[2] = {2200*60*60*1000,100};
bool discharge = true;
unsigned long chargingFirstTime = 0;
int timeInterval = 100;
unsigned long dischargeFirstTime = 0;
int batteryHealth = 100;
int batteryHealthLevel = 4;

//globale variabler til batteryWarning funksjonen
unsigned long warningFirstTime = 0;

#define NUM_SENSORS 5 //Definerer 5 til NUM_SENSORS
unsigned int lineSensorValues[NUM_SENSORS]; //Lager et array for de fem Sensor verdiene 

unsigned long firstTime; // variabler fart og distanse utregning
float speedAndDistanceArray[2];

float averageSpeedMaxSpeedTime [3]; // variabler gjennomsnittsutregning og div
float averageSpeedFirstDistance;
float maxSpeed;
float maxSpeedTime;
unsigned long lastTime;
char sendData; 
bool readData =false;
int counter;

int fart = 200; //Definerer fart veriablen som senere vil bli brukt til å styre farten på motorene 

int soppelKasse=-1;
char soppelstate[4];

void setup() {
   lineSensors.initFiveSensors(); // Skrur på alle 5 IR-sensorene
  Serial1.begin(9600); //Serial 1 er den motatte komunikasjonen/UART fra ESP32
  Serial.begin(9600); //Serial er den vanlig serial kommunikasjonen.

  motors.setSpeeds(0,0); //Sets motorpower
}

void loop() {
  String topic="";
  String msg="";//Variabel som brukes til å velge hvilken switch case meldingen skal trigge
  
  while (Serial1.available() > 0) {
    
    static char melding[30]; //Lager et satic char array for innkommende meldinger
    static unsigned int melding_pos = 0;//Lager en int for å leses en og en karakter i meldingen
    
    char inByte = Serial1.read(); //Leser den første lesbare karakteren i Serial
    
    if(inByte != '\n' && melding_pos < 19) { // For hver itterasjon i while-loopen sjekker denne meldingen om det neste i Serial er et linje skift og at meldingen ikke har oversteget 11 karakterer. 
        melding[melding_pos] = inByte;
        melding_pos++;
    } 
    else {
      bool topicBool = true;
      for (int i = 0; i < melding_pos - 1; i++) {
        if(melding[i] == '!') {
          topicBool = false;
          continue;  
        }
      if(topicBool) {
        topic += melding[i];  
      } else {
        msg += melding[i];
      }
      
      }
      melding_pos = 0;
      //Serial.println(topic);
      //Serial.println(kommando); 
    }
    //Serial.println(topic);
    if(topic == "fj/kontroll") {
      kommando = msg[0];
     }
    if(topic == "fj/hastighet") {
      fart = msg.toInt();
    }
    if(topic=="dataIn"){
      readData=true; 
      sendData = msg[0];
      //Serial.print(sendData); 
    }
    if(topic=="stopFlow"){
      readData=false;
      
      }
    if (topic == "soppelstate") {
      for (int i = 0; i < 4; i++) {
        soppelstate[i] = msg[i];
        Serial.println(soppelstate[i]);  
      }
      if (soppelstate[soppelKasse] == '0') {
          motors.setSpeeds(200, 200);
          delay(250);
          kommando = 'l';
        }
      else if (soppelstate[soppelKasse] == '1') {
        soppelstate[soppelKasse] == '0';
        soppelHent();
        kommando='l';
        }
    }

    //kommando flow -------------------------------------------------------------------------
    //Serial.println(kommando);
    switch(kommando) {
      case 'w': //case for at bilen skal kjøre fremover
        motorR=fart;
        motorL=fart;
      break;

      case 'a': //case for at bilen skal svinge til venstre
        motorR=fart;
        motorL=fart/2;
      break;

      case 's': //case for at bilen skal rygge
        motorR=-fart;
        motorL=-fart;
      break; 

      case 'd': // case for at bilen skal svinge mot høyre
        motorR=fart/2;
        motorL=fart;
      break;

      case '*'://case for at bilen skal stoppe
        motorR=0;
        motorL=0;
      break;
      
      case 'c': //case for å kallibrere linjefølgnings sensorene
        kommando=' ';
        calibrateSensors();
      break;
      

      case 'q':// case for at bilen skal snu seg på samme sted mot venstre
        motorR=fart;
        motorL=-fart;
      break;
      
      case 'e'://case for at bilen skal snu seg på stedet mot høyre
       motorR=-fart;
       motorL=fart;
      break;
      
      case'h':
      sendData='h';
      break;

      case'k':
      sendData='k';
      break;
      
      default:
      break;  
    
    }
    motors.setSpeeds(motorL,motorR); // Setter motor farten til det den ble definert som i casene.
    //-----------------------------------------------------------------------------------------------------------------------------
  }
   if(kommando == 'l') {
   linjefolg(kommando);
   }
   if(kommando=='o'){
    Serial1.println("soppel!fetch soppel");
    kommando=' '; 
    motorR = 0;
    motorL = 0;
   }
   motors.setSpeeds(motorL,motorR); // Setter motor farten til det den ble definert som i casene.
   hastighet(sendData, readData);
}
  
void hastighet(char sendData, bool readData){ // Funksjon som oppdaterer arrayen med hastighet og distanse i tillegg regner ut visse verdier etter 60sek


  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //regner ut fart og distanse hvis gitt tidsintervall har passert
    float countRight = encoders.getCountsAndResetRight();
    float countLeft = encoders.getCountsAndResetLeft();
    float averageCount = (countRight + countLeft)/2;
    float revolutions = averageCount/909.7;
    float distanceInCm = 12.25 * revolutions;

    speedAndDistanceArray[0] = distanceInCm / (0.1); //lagrer farten i arrayen
    speedAndDistanceArray[1] += abs(distanceInCm); //lagrer distansen i arrayen

    if (abs(speedAndDistanceArray[0]) > maxSpeed){ //sjeker om vi har en ny maksfart
      maxSpeed = speedAndDistanceArray[0];
    }

    if ((abs(speedAndDistanceArray[0])) > (0.7*69)){ //regner ut om hastigheten er større en 70% av maks og legger til hvor mange sekunder vi har hatt den hastigheten
      maxSpeedTime += timeInterval;
    }
   
    if(readData==true) {
     
      if(sendData=='h'){
        
        String hastighet=String(speedAndDistanceArray[0],2);
          Serial1.println("stats/hastighet!" + hastighet);
        }
        else if(sendData=='k'){
        
        String avstand= String(speedAndDistanceArray[1],2);
          Serial1.println("stats/avstand!" + avstand);
          
        }
      }
    firstTime = millis(); //Starter ny tidtaking
    
    }



    if (counter % (60*1000)==0){
      averageSpeedMaxSpeedTime[0] = (speedAndDistanceArray[1] - averageSpeedFirstDistance)/60; //regner ut gjennomsnittshastighet i cm/s ila siste 60 sek og lagrer
      averageSpeedMaxSpeedTime[1] = maxSpeed; //lagrer max hastighet
      averageSpeedMaxSpeedTime[2] = maxSpeedTime; //lagrer hvor lenge bilen har kjørt mer enn 70% av maks hastighet
      if ((averageSpeedMaxSpeedTime[0] >= 40) || (abs(averageSpeedMaxSpeedTime[1]) >= 65) || (averageSpeedMaxSpeedTime[2] >= 30)){ //oppdaterer battery health etter krav
        batteryHealth--;
      }
      if(readData=true){
        if(sendData=='p'){
          String gjennomCms= String(averageSpeedMaxSpeedTime[0],2);
          Serial.println("stats/hastighet_gj!"+gjennomCms); //sender gjennomsnittshastighet i cm/s    
          String maxsHastighet= String(averageSpeedMaxSpeedTime[1],2);
          Serial.println("stats/maks!"+maxsHastighet); //sender makshastighet
          String fartOver70= String(averageSpeedMaxSpeedTime[2],2);  
          Serial.println("stats/70aM!"+fartOver70); //sender tid over 70% av makshastighet  
        }
      } 
     

      averageSpeedFirstDistance = speedAndDistanceArray[1]; //noterer ned nåværende kjørt distanse slik at vi kan beregne hastighet neste gang
      maxSpeed = 0; //resetter variablene
      maxSpeedTime = 0;
    }
  }

void calibrateSensors(){
  /*Kalliberingen av IR-sensorene gjøres slik at sensorene vet forskjellen på hva som er vanlig bakke og hva som er definert som linje. 
  Det er derfor viktig at bilen er plassert over en linje før man starter denne delen av programmet.  */
  delay(1000);//Kunstpause
  for(uint16_t i = 0; i < 122; i++){//For-loop for at bilen skal snurre mot høyre og venstre på samme sted
    if (i > 30 && i <= 90){
      motors.setSpeeds(-200, 200);//Først mot venstre
    }
    else{
      motors.setSpeeds(200, -200);//Deretter mot høyre
    }
    lineSensors.calibrate(); // Her bruker vi kalliberings funksjonen som scanner bakken en gang og henter inn dataen som referanse
  }
  motors.setSpeeds(0, 0);// Tilslutt setter vi motorene til null slik at bilen stopper og er klar til å kjøres.
}
void linjefolg(char kommando){
  if (kommando == 'l') {
    int position = lineSensors.readLine(lineSensorValues); 
    position=position-2000;
    Serial.println(lineSensorValues[4]);
    if(position<0){
    motorR=-200*exp(-0.0025*abs(position))+400;
    motorL=200*exp(-0.0025*abs(position));
    motors.setSpeeds(motorL,motorR);
    }
    else{
    motorL=-200*exp(-0.0025*abs(position))+400;
    motorR=200*exp(-0.0025*abs(position));
    motors.setSpeeds(motorL,motorR);
    }   
  }
} 
void soppelHent(){
  int teller = 0;
motors.setSpeeds(-200,200);
  delay(420);
  motors.setSpeeds(200,200);
  delay(200);
  int position = lineSensors.readLine(lineSensorValues);
  while(lineSensorValues[0] < 700) {
    position = lineSensors.readLine(lineSensorValues); 
    position=position-2000;
    //Serial.println(lineSensorValues[4]);
    if(position<0){
    motorR=-200*exp(-0.0025*abs(position))+400;
    motorL=200*exp(-0.0025*abs(position));
    motors.setSpeeds(motorL,motorR);
    }
    else{
    motorL=-200*exp(-0.0025*abs(position))+400;
    motorR=200*exp(-0.0025*abs(position));
    motors.setSpeeds(motorL,motorR);
    }
    if(teller == 0) {
      if (position == -2000 || position == 2000) {
        motors.setSpeeds(200, -200); 
        delay(880);
        teller++;
      } 
    }
    else if (teller == 1) {
      if (position == -2000 || position == 2000) {
        motors.setSpeeds(200,200);
        delay(300);
        break;
      }
    }
  }
  motors.setSpeeds(-200,200);
  delay(440);
  motors.setSpeeds(200,200);
  delay(250);
}

#include <Wire.h> //wire.h biblioteket
#include <Zumo32U4.h>//zumo biblioteket
const int ENA = 9; //Motor 1
const int IN1 = 15; //Wheel direction 1
const int ENB = 10; //Motor 2
const int IN2 = 16; //Wheel direction 2

bool mode=true; //Variabel som bestemmer om bilen skal kjøre fort eller sakte
int motorL=0; //Variabel som bestemmer farten til det venstre hjulet
int motorR=0; //Variabelen som bestemmer farten til det høyre hjulet
char data[3]; //Initialized variable to store recieved data

Zumo32U4Encoders encoders;
Zumo32U4Motors motors; //Gir motors en class
Zumo32U4LineSensors lineSensors; //
#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];

unsigned long firstTime; // variabler fart og distanse utregning
float speedAndDistanceArray[2];

float averageSpeedMaxSpeedTime [3]; // variabler gjennomsnittsutregning og div
float averageSpeedFirstDistance;
float maxSpeed;
float maxSpeedTime;
unsigned long counter;
unsigned long lastTime; 
int fart = 200;
int lastmill;
void setup() {
   lineSensors.initFiveSensors(); // Skrur på alle 5 IR-sensorene
  Serial1.begin(9600); //Serial 1 er den motatte komunikasjonen/UART fra ESP32
  Serial.begin(9600); //Serial er den vanlig serial kommunikasjonen.

  motors.setSpeeds(0,0); //Sets motorpower
}

//Enable or disable the motors
void motor(int motorPower){
  analogWrite(ENA, motorPower);
  analogWrite(ENB, motorPower);

  Serial.println("Motors setup");
}

void loop() {
  //Serial1.print('h');
 
  //motors.setSpeeds(100,100);
  
  while (Serial1.available() > 0) {
     
    static char melding[12];
    static unsigned int melding_pos = 0;
    char kommando;
  

    char inByte = Serial1.read();
    if(inByte != '\n' && melding_pos < 11) {
        melding[melding_pos] = inByte;
        melding_pos++;
    } else {
        melding[melding_pos] ='\0';
        
        if (melding_pos <= 2 && atoi(melding)==0) {
          kommando = melding[0];          
        }
        if (atoi(melding) > 0) {
          fart = atoi(melding);  
        } 
      
        melding_pos = 0;
    }
    
    switch(kommando) {
      case 'w':
        motorR=fart;
        motorL=fart;
      break;

      case 'd':
        motorR=fart;
        motorL=fart/2;
      break;

      case 's':
        motorR=-fart;
        motorL=-fart;
      break; 

      case 'a':
        motorR=fart/2;
        motorL=fart;
      break;

      case '*':
        motorR=0;
        motorL=0;
       break;
      
      case '#':
        mode= !mode;
      break;
      
      case 'c':
        //calibrateSensors();
      break;
      
      case 'l':
        linjefolg();
      break;
      
      default:
      break;  
    }
   if(mode){
      motors.setSpeeds(motorL,motorR);
   }
   else{
      motors.setSpeeds(motorL*2,motorR*2);  
   }  
   
    
  }
 hastighet();
    
    }
  
    

void calibrateSensors(){
  /*Kalliberingen av IR-sensorene gjøres slik at sensorene vet forskjellen på hva som er vanlig bakke og hva som er definert som linje. 
  Det er derfor viktig at bilen er plassert over en linje før man starter denne delen av programmet.  */
  delay(1000);
  for(uint16_t i = 0; i < 120; i++){
    if (i > 30 && i <= 90){
      motors.setSpeeds(-200, 200);
    }
    else{
      motors.setSpeeds(200, -200);//Deretter mot høyre
    }
    lineSensors.calibrate(); // Her bruker vi kalliberings funksjonen som scanner bakken en gang og henter inn dataen som referanse
  }
  motors.setSpeeds(0, 0);// Tilslutt setter vi motorene til null slik at bilen stopper og er klar til å kjøres.
}
void linjefolg(){
  while (Serial1.read()==-1){   
           int16_t position = lineSensors.readLine(lineSensorValues); 
           position=position-2000;
           if (position<0){
           motorL=(2000-abs(position))^2/3500+200;
           motorR=-(2000-abs(position))^2/3500+200;
           motors.setSpeeds(motorL,motorR);
            
           }
           else{
           motorR=(2000-abs(position))^2/3500+200;
           motorL=-(2000-abs(position))^2/3500+200;
           motors.setSpeeds(motorL,motorR);
           }
  } 
}
void hastighet(){ // Funksjon som oppdaterer arrayen med hastighet og distanse i tillegg regner ut visse verdier etter 60sek
  float timeInterval = 1000; //setter tidsintervallet

  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //regner ut fart og distanse hvis gitt tidsintervall har passert
    float countRight = encoders.getCountsAndResetRight();
    float countLeft = encoders.getCountsAndResetLeft();
    float averageCount = (countRight + countLeft)/2;
    float revolutions = averageCount/909.7;
    float distanceInCm = 12.25 * revolutions;

    speedAndDistanceArray[0] = distanceInCm / (timeInterval/1000); //lagrer farten i arrayen
    speedAndDistanceArray[1] += abs(distanceInCm); //lagrer distansen i arrayen

    if (abs(speedAndDistanceArray[0]) > maxSpeed){ //sjeker om vi har en ny maksfart
      maxSpeed = speedAndDistanceArray[0];
    }

    if ((abs(speedAndDistanceArray[0])) > (0.7*69)){ //regner ut om hastigheten er større en 70% av maks og legger til hvor mange sekunder vi har hatt den hastigheten
      maxSpeedTime += timeInterval;
    }
    Serial1.println("cmS");
    //Serial.print("cmS");
    Serial1.println(speedAndDistanceArray[0], 0); //sender farten i cm/s
    Serial1.println("avstand");
    Serial1.println(speedAndDistanceArray[1], 0); //sender distansen i cm

    counter += timeInterval/1000; //oppdaterer telleren som kan fortelle oss når det har gått 60 sekunder

    firstTime = millis(); //Starter ny tidtaking
    }



    if ((counter % 60) == 0){
      averageSpeedMaxSpeedTime[0] = (speedAndDistanceArray[1] - averageSpeedFirstDistance)/60; //regner ut gjennomsnittshastighet i cm/s ila siste 60 sek og lagrer
      averageSpeedMaxSpeedTime[1] = maxSpeed; //lagrer max hastighet
      averageSpeedMaxSpeedTime[2] = maxSpeedTime; //lagrer hvor lenge bilen har kjørt mer enn 70% av maks hastighet
      
      //Serial1.println(averageSpeedMaxSpeedTime[0]); //sender gjennomsnittshastighet i cm/s
      //Serial1.println(averageSpeedMaxSpeedTime[1]); //sender makshastighet
      //Serial1.println(averageSpeedMaxSpeedTime[2]); //sender tid over 70% av makshastighet

      averageSpeedFirstDistance = speedAndDistanceArray[1]; //noterer ned nåværende kjørt distanse slik at vi kan beregne hastighet neste gang
      maxSpeed = 0; //resetter variablene
      maxSpeedTime = 0;
    }
  }

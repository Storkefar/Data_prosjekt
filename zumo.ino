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
  Serial1.print('h');
 while(Serial1.available()) {
    //Serial1.readBytes(data, 1); //Read the serial data from the ESP32 and store in the data variable
    //delay(1);
    char received = Serial1.read(); //Reads the serial data as a char to use with switch case
    Serial.println(received); //Print data to Serial Monitor 
    hastighet();
    
      char mottat = received;
      switch(mottat) {
        case '2':
          motorR=200;
          motorL=200;
        break;
  
        case '6':
          motorR=200;
          motorL=100;
        break;
  
        case '5':
          motorR=-200;
          motorL=-200;
        break; 
  
        case '4':
          motorR=100;
          motorL=200;
        break;
  
        case '*':
          motorR=0;
          motorL=0;
         break;
        
        case '#':
          mode= !mode;
        break;
        
        case '8':
          calibrateSensors();
        break;
        
        case '9':
          linjefolg();
        break;
        
        default:
        break;  
      }
     if(mode){
        motors.setSpeeds(motorR,motorL);
     }
     else{
        motors.setSpeeds(motorR*2,motorL*2);  
     }
     
    
  }
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
void hastighet(){

 //Serial1.println("522");
  //Serial.print("qqq");*/
  float timeInterval = 2000; //setter tidsintervallet

  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //regner ut fart og distanse hvis gitt tidsintervall har passert
    float countRight = encoders.getCountsAndResetRight();
    float countLeft = encoders.getCountsAndResetLeft();
    float averageCount = (countRight + countLeft)/2;//
    float revolutions = averageCount/909.7;
    int distanceInCm = 12.25 * revolutions;
    Serial1.print("!");
    Serial1.print(distanceInCm);
  }
    timeInterval=timeInterval+2000; //oppdaterer telleren som kan fortelle oss når det har gått 60 sekunder

    firstTime = millis()-firstTime; //Starter ny tidtaking
  
  }

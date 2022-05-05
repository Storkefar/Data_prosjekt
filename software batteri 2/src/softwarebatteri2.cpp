#include <Arduino.h>
#include <Zumo32U4.h>
#include <Wire.h>

Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4Buzzer buzzer;

//speedometer variabler
//tidsvariabler
float timeInterval = 100; 
unsigned long firstTime; 
//variablen med fart og distanse
float speedAndDistanceArray[2]; 
//gjennomsnittsmålings variabler
float averageSpeedMaxSpeedTime [3]; // variabler gjennomsnittsutregning og div.
float averageSpeedFirstDistance; //distanse brukt til gjennomsnittsfart
float maxSpeed; //maxhastighet gjennomsnittsfart
float maxSpeedTime; //tid til max hastighet utregning

//batteri variabler
//tidsvariabler knyttet til batteriet
float hiddenChargeTotalTime = 0;
unsigned long warningFirstTime = 0;
unsigned long chargingFirstTime = 0;
unsigned long dischargeFirstTime = 0;
//statusvariabler knyttet til batteriet
int emergencyChargeStatus = 0; 
bool discharge = true;
//variabler knyttet til batterinivå
//long batteryCapacityAndPercentage[2] = {2200*3600,100};
float batteryCapacity = 7920000;
float batteryPercentage = 100;
float batteryHealth = 100;
float batteryHealthLevel = 4;











//speedometer funksjon
void speedAndDistance(){ // Funksjon som oppdaterer arrayen med hastighet og distanse i tillegg regner ut visse verdier etter 60sek
  
  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //regner ut fart og distanse hvis gitt tidsintervall har passert
    float countRight = encoders.getCountsAndResetRight();
    float countLeft = encoders.getCountsAndResetLeft();
    float averageCount = (countRight + countLeft)/2;
    float revolutions = averageCount/909.7;
    float distanceInCm = 12.25 * revolutions;

    speedAndDistanceArray[0] = distanceInCm / (0.1); //lagrer farten i arrayen NB 0.1 er time interval/1000
    speedAndDistanceArray[1] += abs(distanceInCm); //lagrer distansen i arrayen

    if (abs(speedAndDistanceArray[0]) > maxSpeed){ //sjeker om vi har en ny maksfart
      maxSpeed = speedAndDistanceArray[0];
    }

    if ((abs(speedAndDistanceArray[0])) > (0.7*69)){ //regner ut om hastigheten er større en 70% av maks og legger til hvor mange sekunder vi har hatt den hastigheten
      maxSpeedTime += (timeInterval/1000);
    }

    firstTime = millis(); //Starter ny tidtaking
    
    /* uncomment for å teste kode
    Serial.println(speedAndDistanceArray[0]);
    Serial.println(speedAndDistanceArray[1]);
    Serial.println("--------");
    */
    }


 
    if (lastTime % (60*1000) == 0){ //sjekker om det har gått 60 sekunder
      averageSpeedMaxSpeedTime[0] = (speedAndDistanceArray[1] - averageSpeedFirstDistance)/60; //regner ut gjennomsnittshastighet i cm/s ila siste 60 sek og lagrer
      averageSpeedMaxSpeedTime[1] = maxSpeed; //lagrer max hastighet
      averageSpeedMaxSpeedTime[2] = maxSpeedTime; //lagrer hvor lenge bilen har kjørt mer enn 70% av maks hastighet

      if ((averageSpeedMaxSpeedTime[0] >= 40) || (abs(averageSpeedMaxSpeedTime[1]) >= 65) || (averageSpeedMaxSpeedTime[2] >= 30)){ //oppdaterer battery health etter krav
        batteryHealth--;
      }
      /* uncomment for å sjekke kode
      Serial.println(averageSpeedMaxSpeedTime[0]); //sender gjennomsnittshastighet i cm/s
      Serial.println(averageSpeedMaxSpeedTime[1]); //sender makshastighet
      Serial.println(averageSpeedMaxSpeedTime[2]); //sender tid over 70% av makshastighet
      Serial.println("-------");
      Serial.println(batteryHealth);
      Serial.println("------");
      */

      averageSpeedFirstDistance = speedAndDistanceArray[1]; //noterer ned nåværende kjørt distanse slik at vi kan beregne hastighet neste gang
      maxSpeed = 0; //resetter variablene
      maxSpeedTime = 0;
    } 
  } 
  //funksjon slutt





void batteryDischarge(){ 

  speedAndDistance();

  
    if (discharge == true){

        unsigned long dischargeLastTime = millis();

        if ((buttonC.getSingleDebouncedPress()) && (emergencyChargeStatus == 0)){ //skrur på nødladingsmodus hvis knapp c trykkes *kan kun brukes en gang
          emergencyChargeStatus = 1;
          }

        if ((dischargeLastTime - dischargeFirstTime) >= timeInterval){ //sjekker om det har gått 100ms     
          //nødlading
          if ((emergencyChargeStatus == 1) && (speedAndDistanceArray[0] < 0)){ //foretar nødlading hvis valgt
            float instantConsumption = ((abs(speedAndDistanceArray[0])*2)+10); //regner ut lading
            batteryCapacity += (instantConsumption*(timeInterval/1000)); //oppdaterer kapasiteten
            batteryPercentage = (batteryCapacity/(7920000))*(100); //oppdaterer batteri prosenten            
            if (batteryPercentage >= 20){ //slår av nødlading når 20% er nådd
              emergencyChargeStatus = 2;
            }
          }
          //nødlading slutt
          
          //hidden lading
          else if ((speedAndDistanceArray[0] < -30)&&(hiddenChargeTotalTime < 120)){ //hidden lading initiert ved fart mer enn 30 cm/s bakover. Kan maks brukes 2 min
            float instantConsumption = (abs(speedAndDistanceArray[0])*2)+10; //regner ut lading
            batteryCapacity += (instantConsumption*(timeInterval/1000)); //oppdaterer kapasiteten
            batteryPercentage = (batteryCapacity/(7920000))*100; //oppdaterer batteri prosenten
            hiddenChargeTotalTime += (timeInterval/1000); //renger ut hvor lenge hidden charge har blitt gjort totalt         
          }
          //hidden lading slutt

          //vanlig utlading 
          else{ 
          float instantConsumption = (abs((speedAndDistanceArray[0])*2))+10; //regner ut forbruk
          float firstPercentageSample = batteryPercentage; //sjekker om batteriet er 5% eller mer 
          batteryCapacity -= (instantConsumption*(timeInterval/1000))*1000; //oppdaterer kapasiteten
          batteryPercentage = (batteryCapacity/(7920000))*(100); //oppdaterer batteri prosenten
          float secondPercentageSample = batteryPercentage; //sjekker om batteriet har gått under 5%
          if ((firstPercentageSample > 10) && (secondPercentageSample <= 10)){
             buzzer.playFrequency(800, 1000, 15); //gir varsel hvis batteriet har gått under 10% 
          }
          else if  ((firstPercentageSample >= 5) && (secondPercentageSample < 5)){
            batteryHealth--;
          }         
          }
          //vanlig utlading slutt 
          dischargeFirstTime = millis();

          if (batteryPercentage > 100){
            batteryPercentage = 100;
            batteryCapacity = 7920000;
          }
          else if (batteryPercentage < 0){
            batteryPercentage = 0;
            batteryCapacity = 0;
          }
Serial.println(batteryPercentage);
Serial.println(emergencyChargeStatus);
Serial.println("--------");                  
        }
  }
  else {
    dischargeFirstTime = millis(); //hvis lading == false så noteres tiden slik at det blir korrekt når discharge tidene skal sammenlignes
  }  
}



















void setup() {
  Serial.begin(9600);
}

void loop() {
  motors.setSpeeds(200,200);
  batteryDischarge();
  
 }
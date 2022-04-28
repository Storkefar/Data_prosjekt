#include <Arduino.h>
#include <Zumo32U4.h>
#include <Wire.h>

Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4LCD display;

unsigned long firstTime; // variabler fart og distanse utregning
float speedAndDistanceArray[2];

float averageSpeedMaxSpeedTime [3]; // variabler gjennomsnittsutregning og div
float averageSpeedFirstDistance;
float maxSpeed;
float maxSpeedTime;
unsigned long counter;



void speedAndDistance(){ // Funksjon som oppdaterer arrayen med hastighet og distanse i tillegg regner ut visse verdier etter 60sek
  float timeInterval = 100; //setter tidsintervallet

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

    Serial1.println(speedAndDistanceArray[0]); //sender farten i cm/s
    Serial1.println(speedAndDistanceArray[1]); //sender distansen i cm

    counter += timeInterval/1000; //oppdaterer telleren som kan fortelle oss når det har gått 60 sekunder

    firstTime = millis(); //Starter ny tidtaking
    }



    if ((counter % 60) == 0){
      averageSpeedMaxSpeedTime[0] = (speedAndDistanceArray[1] - averageSpeedFirstDistance)/60; //regner ut gjennomsnittshastighet i cm/s ila siste 60 sek og lagrer
      averageSpeedMaxSpeedTime[1] = maxSpeed; //lagrer max hastighet
      averageSpeedMaxSpeedTime[2] = maxSpeedTime; //lagrer hvor lenge bilen har kjørt mer enn 70% av maks hastighet

      Serial1.println(averageSpeedMaxSpeedTime[0]); //sender gjennomsnittshastighet i cm/s
      Serial1.println(averageSpeedMaxSpeedTime[1]); //sender makshastighet
      Serial1.println(averageSpeedMaxSpeedTime[2]); //sender tid over 70% av makshastighet

      averageSpeedFirstDistance = speedAndDistanceArray[1]; //noterer ned nåværende kjørt distanse slik at vi kan beregne hastighet neste gang
      maxSpeed = 0; //resetter variablene
      maxSpeedTime = 0;
    }
  } //funksjon slutt


 






void setup() {

}

void loop() {
 
}
#include <Arduino.h>
#include <Zumo32U4.h>
#include <Wire.h>

Zumo32U4Encoders encoders;
Zumo32U4Motors motors;

// fartsmålings variabler
float timeInterval = 100; //tidsintervall fartsmåling
unsigned long firstTime; // variabler fart og distanse utregning
float speedAndDistanceArray[2]; //hastighet og fart

//gjennomsnittsmålings variabler
float averageSpeedMaxSpeedTime [3]; // variabler gjennomsnittsutregning og div.
float averageSpeedFirstDistance; //distanse brukt til gjennomsnittsfart
float maxSpeed; //maxhastighet gjennomsnittsfart
float maxSpeedTime; //tid til max hastighet utregning

//battery health som blir oppdatert hvis gjennomsnittsmålinger tilfredstiller kravene
float batteryHealth = 100;



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

    if ((abs(speedAndDistanceArray[0])) > (0.01*69)){ //regner ut om hastigheten er større en 70% av maks og legger til hvor mange sekunder vi har hatt den hastigheten
      maxSpeedTime += (timeInterval/1000);
    }

    firstTime = millis(); //Starter ny tidtaking
    
    /* uncomment for å teste kode
    Serial.println(speedAndDistanceArray[0]);
    Serial.println(speedAndDistanceArray[1]);
    Serial.println("--------");
    */
    }


 
    if (lastTime % (60*1000) == 0){ //sjekker om det har gått 5 sekunder
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


 






void setup() {
 

}

void loop() {


 
}
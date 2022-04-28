#include <Arduino.h>
#include <Zumo32U4.h>
#include <Wire.h>

Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4LCD display;

unsigned long firstTime; //deklarerer variabler
float speedAndDistanceArray[2];



void speedAndDistance(){ // Funksjon som oppdaterer arrayen med hastighet og distanse
  float timeInterval = 100; //setter tidsintervallet

  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //regner ut fart og distanse hvis gitt tidsintervall har passert
    float countRight = encoders.getCountsAndResetRight();
    float countLeft = encoders.getCountsAndResetLeft();
    float averageCount = (countRight + countLeft)/2;
    float revolutions = averageCount/909.7;
    float distanceInCm = 12.25 * revolutions;

    speedAndDistanceArray[0] = distanceInCm / (timeInterval/1000);
    speedAndDistanceArray[1] += abs(distanceInCm); 

    Serial1.println(speedAndDistanceArray[0]); //sender farten i cm/s
    Serial1.println(speedAndDistanceArray[1]); //sender distansen i cm

    firstTime = millis(); //Starter ny tidtaking
    }
  } //funksjon slutt
 






void setup() {

}

void loop() {
 
}
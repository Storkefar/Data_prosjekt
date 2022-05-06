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
unsigned long firstTime = 0;  
//variablen med fart og distanse
float speedAndDistanceArray[2]; 
//gjennomsnittsmålings variabler
float averageSpeedMaxSpeedTime [3]; 
float averageSpeedFirstDistance; 
float maxSpeed; 
float maxSpeedTime; 
float averageArrayCount = 0;

//batteri variabler
//tidsvariabler knyttet til batteriet
float hiddenChargeTotalTime = 0;
unsigned long dischargeFirstTime = 0;
float batteryWarningTime = 0;
unsigned long chargingFirstTime = 0;
//statusvariabler knyttet til batteriet
int emergencyChargeStatus = 0; 
bool discharge = true;
bool serviceNeeded = false;
bool changeNeeded = false;
//variabler knyttet til batterinivå
float batteryCapacity = 7920000;
float batteryPercentage = 100;
float batteryHealth = 100;
float batteryHealthLevel = 4;






void speedAndDistance(){ 
  
  unsigned long lastTime = millis(); //måler nåværende tid

  if ((lastTime - firstTime) >= timeInterval){ //sjekker om tidsintervallet har passert
    float countRight = encoders.getCountsAndResetRight(); //henter encoder counts og resetter count høyre
    float countLeft = encoders.getCountsAndResetLeft(); //samme som over, men på venstre side
    float averageCount = (countRight + countLeft)/2; //regner ut gjennomsnittlig count
    float revolutions = averageCount/909.7; //regner ut omdreininger
    float distanceInCm = 12.25 * revolutions; //regner ut distansen i cm

    speedAndDistanceArray[0] = distanceInCm / (0.1); // regner ut farten. 0.1 er tidsintervallet i sekunder
    speedAndDistanceArray[1] += abs(distanceInCm); //inkrementerer total distanse kjørt

    if (abs(speedAndDistanceArray[0]) > maxSpeed){ //sjekker om vi har fått en større maksfart enn den som er lagret. Blir annulert hvert 60. sek
      maxSpeed = speedAndDistanceArray[0]; //lagrer ny maksfart
    }

    if ((abs(speedAndDistanceArray[0])) > (0.7*69)){ //regner ut om hastigheten er større en 70% av maks og legger til hvor mange sekunder vi har hatt den hastigheten
      maxSpeedTime += (0.1); //NB 0.1 er timeInterval/1000
    }
    averageArrayCount += 0.1; //inkrementerer en count som hjelper oss å fortelle når 60 sekunder har gått
    firstTime = millis(); //Starter ny tidtaking for så vi vet når neste gang vi skal lagre en ny fart   
    } 

    if (averageArrayCount >= 60){ //hvis det har gått 60 sekunder, blir visse verdier lagret

      averageSpeedMaxSpeedTime[0] = (speedAndDistanceArray[1] - averageSpeedFirstDistance)/60; //regner ut gjennomsnittshastighet i cm/s ila siste 60 sek og lagrer i en array
      averageSpeedMaxSpeedTime[1] = maxSpeed; //lagrer max hastigheten ila siste 60 sek
      averageSpeedMaxSpeedTime[2] = maxSpeedTime; //lagrer hvor lenge bilen har kjørt mer enn 70% av maks hastighet ila siste 60 sek

      if ((averageSpeedMaxSpeedTime[0] >= 40) || (abs(averageSpeedMaxSpeedTime[1]) >= 65) || (averageSpeedMaxSpeedTime[2] >= 30)){ //dekrementerer battery health etter visse krav
        batteryHealth --;
      }
 
      averageSpeedFirstDistance = speedAndDistanceArray[1]; //noterer ned nåværende kjørt distanse slik at vi kan beregne gjennomsnitts hastighet neste gaang 60 sek har passert
      maxSpeed = 0; //nullstiller variablene slik at vi får alltid får nye verider ila de neste 60 sekundene
      maxSpeedTime = 0;
      averageArrayCount = 0;
      } 
  } 
  





void batteryDischarge(){ 
  
    if (discharge == true){ //sjekker om funksjonen skal kjøres

        unsigned long dischargeLastTime = millis(); //måler nåværende tid

        if ((buttonC.getSingleDebouncedPress()) && (emergencyChargeStatus == 0)){ //skrur på nødlading c trykkes
          emergencyChargeStatus = 1;
          }

        if ((dischargeLastTime - dischargeFirstTime) >= timeInterval){ //sjekker om det har gått 100ms     
          if ((emergencyChargeStatus == 1) && (speedAndDistanceArray[0] < 0)){ //foretar nødlading hvis valgt
            float instantConsumption = ((abs(speedAndDistanceArray[0])*2)+10); //regner ut lading
            batteryCapacity += (instantConsumption*0.1)*10; //oppdaterer kapasiteten NB 0.1 er timeInterval/1000
            batteryPercentage = (batteryCapacity/(7920000))*(100); //oppdaterer batteri prosenten            
            if (batteryPercentage >= 20){ //slår av nødlading når 20% er nådd
              emergencyChargeStatus = 2;
            }
          }
          
          else if ((speedAndDistanceArray[0] < -30)&&(hiddenChargeTotalTime < 120)){ //sjekker om hidden lading skal gjøres
            float instantConsumption = (abs(speedAndDistanceArray[0])*2)+10; //regner ut lading
            batteryCapacity += (instantConsumption*0.1); //oppdaterer kapasiteten
            batteryPercentage = (batteryCapacity/(7920000))*100; //oppdaterer batteri prosenten
            hiddenChargeTotalTime += 0.1; //oppdaterer hvor lenge hidden lading har blitt kjørt   
          }

          else{ //kjører vanlig utlading
          float instantConsumption = (abs((speedAndDistanceArray[0])*2))+10; //regner ut forbruk
          float firstPercentageSample = batteryPercentage; //sjekker om batteriet er 5% eller mer 
          batteryCapacity -= (instantConsumption*0.1); //oppdaterer kapasiteten NB 0.1 er timeInterval/1000
          batteryPercentage = (batteryCapacity/(7920000))*(100); //oppdaterer batteri prosenten
          float secondPercentageSample = batteryPercentage; //sjekker om batteriet har gått under 5%
          if ((firstPercentageSample > 10) && (secondPercentageSample <= 10)){
             buzzer.playFrequency(800, 1000, 15); //gir varsel hvis batteriet har gått under 10% 
          }
          else if  ((firstPercentageSample >= 5) && (secondPercentageSample < 5)){
            batteryHealth--; //dekrementerer battery health
          }         
          }
          //vanlig utlading slutt 
          dischargeFirstTime = millis(); //oppdaterer tiden

          if (batteryPercentage > 100){ //sjekker om batteri prosenten er større enn 100
            batteryPercentage = 100; //setter nivåer til maks
            batteryCapacity = 7920000;
          }
          else if (batteryPercentage < 0){ //sjekker om prosenten er under 0
            batteryPercentage = 0; //setter nivåer til 0
            batteryCapacity = 0;
          }
           if ((batteryPercentage <= 5) && (batteryPercentage > 0)){ //sjekker om den skal gi 5% varsel
              batteryWarningTime += 0.1; //inkrementerer varsel tids counter
              if (batteryWarningTime >= 15){ //gir varsel hvis 15 sek har gått
                  motors.setSpeeds(0,0); //stopper bilen
                  buzzer.playFrequency(800, 250, 15); //spiller av buzzeren
                  while(buzzer.isPlaying()){ };
                  delay(250); //venter
                  buzzer.playFrequency(800, 250, 15); //spiller av buzzeren igjen
                  while(buzzer.isPlaying()){ };
                  batteryWarningTime = 0; //resetter varsel tids counteren
              }
            } 
        } 
  }
  else {
    dischargeFirstTime = millis(); //hvis lading == false så noteres tiden slik at det blir korrekt når discharge tidene skal sammenlignes
  }  
}





void batteryHealthAndLevel(){ 
  int randomNumber1 = random(1000); //genererer to random nummere
  int randomNumber2 = random(1000);
      
  if (randomNumber1 == randomNumber2){ //trekker 50 fra battery health hvis de to randome nummerene er like
    batteryHealth -= 50;
  }

  if (batteryHealth <=25){ //sjekker battery health nivå
    if (batteryHealth <= 0){ //hvis mindre eller lik 0, sett til minimum nivå
      batteryHealth = 0;
      batteryHealthLevel = 0;
      batteryCapacity = 0;
      batteryPercentage = 0;
      changeNeeded = true;
      serviceNeeded = false;
    }
    else{
    batteryHealthLevel = 1;
    changeNeeded = true;
    serviceNeeded = false;
    }
  }
  else if ((batteryHealth > 25) && (batteryHealth <= 50)){
    batteryHealthLevel = 2;
    serviceNeeded = true;
    changeNeeded = false;
  }
  else if ((batteryHealth > 50) && (batteryHealth <= 75)){
    batteryHealthLevel = 3;
    serviceNeeded = false;
    changeNeeded = false;
  }
  else {
    if (batteryHealth > 100){
      batteryHealth = 100;
    }
    batteryHealthLevel = 4;
    serviceNeeded = false;
    changeNeeded = false;
  }

  if(buttonB.getSingleDebouncedPress()){ //utfører service eller bytte på batteriet utifra behov
    if (serviceNeeded == true){
      batteryHealth = 60;
      batteryHealthLevel = 3;
      batteryPercentage = 60;
      batteryCapacity = 7920000*0.6;
      serviceNeeded = false; 
    }
    else if (changeNeeded == true){
      batteryHealth = 100;
      batteryHealthLevel = 4;
      changeNeeded = false;
      serviceNeeded = false;
      batteryPercentage = 100;
      batteryCapacity = 7920000;
    }
  }
}





void charging(){ 
  unsigned long chargingLastTime = millis(); //måler tiden

  if ((chargingLastTime - chargingFirstTime) >= 1000){ //sjekker om det har gått 1 sekund med lading
    batteryCapacity += 440; //laderen har tilført batteriet strøm i 1 sekund og oppdaterer kapasitet deretter
    batteryPercentage = (batteryCapacity/7920000)*100; //oppdaterer batteri prosenten
    chargingFirstTime = millis(); //noterer tiden
    if (batteryPercentage == 100){
      batteryHealth --; //dekrementerer batteryHealth grunnet 1 ladesykluse gjennomført
    }
    else if (batteryPercentage > 100){ // sørger for at batteriet ikke kan være mer enn 100%
      batteryCapacity = 7920000;  //justerer nivåene
      batteryPercentage = (batteryCapacity/7920000)*100;
    }
    }
}





void regularCharge(){  

  if (discharge == true){ //sjekker om bilen utlades

    int buttonAPresses = 0; //nullstiller knappetrykkene
    int buttonBPresses = 0;

    if (buttonA.getSingleDebouncedPress()){ //sjekker om det blir trykket A for å starte lading
        buttonAPresses = 1;
        discharge = false; //slår av utlading hvis A trykkes
      }

    if (buttonAPresses == 1){ //blir bare kjørt hvis A blir trykket
      while (buttonAPresses == 1){ //denne løkken lar oss velge ønsket batteri %
        if (buttonB.getSingleDebouncedPress()){
          buttonBPresses += 10; // plusser på 10+ for hvert trykk på B
        }
        if ((buttonBPresses == 100)||buttonA.getSingleDebouncedPress()){
          buttonAPresses = 2; //hvis det blir plusset til 100%, eller A trykkes, avsluttes valget.
        }
      }
      chargingFirstTime = millis(); //oppdaterer charging tiden før lading initieres
      while((buttonAPresses == 2) && (batteryPercentage < buttonBPresses)){ //lader mens batteriet er under 100% eller valgt %
        motors.setSpeeds(0,0); //sørger for at bil står stille under lading
        charging(); //lader med 440 mA 
        if (buttonA.getSingleDebouncedPress()){
          buttonAPresses = 0; //lading kan avsluttes ved å trykke på A
        }
      }
      discharge = true; // initierer utladings modus
      dischargeFirstTime = millis(); //oppdaterer discharge first time
    }
  }
}


void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
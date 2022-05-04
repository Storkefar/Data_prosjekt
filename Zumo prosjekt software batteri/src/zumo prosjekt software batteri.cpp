#include <Zumo32U4.h>
#include <Arduino.h>
#include <Wire.h>

Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4Encoders encoders;
Zumo32u4Motors motors;
Zumo32U4Buzzer buzzer;

//globale variabler til dishcarge funksjonen
int hiddenChargeTotalTime = 0;
int emergencyChargeStatus = 0; 
  
//globale variabler til regularCharge funksjonen

//globale variabler flere funksjoner er avhengig av
float batteryCapacityAndPercentage[2] = {2200*60*60*1000,100}
float speedAndDistanceArray[2];
bool discharge = true;
unsigned long chargingFirstTime = 0;
int timeInterval = 100;
unsigned long dischargeFirstTime = 0;
float averageSpeedMaxSpeedTime [3];
int batteryHealth = 100;
int batteryHealthLevel = 4;

//globale variabler til batteryWarning funksjonen
unsigned long warningFirstTime = 0;





/*regner om prosenten motatt, til kapasitet i maMs og lagrer prosenten 
endrer: batteryCapacityAndPercentage
avhengig av: sentPercentage*/
void percentageToCapacity(sentPercentage){ 
    batteryCapacityAndPercentage[0] *= (sentPercentage/100); //gjør om mottat prosent til kapasitet og oppdaterer
    batteryCapacityAndPercentage[1] = sentPercentage; //lagrer mottat prosent
}



/* denne funksjonen utlader batteriprosenten og kapasiteten, kan aktivere hidden lading og nødlading. Den endrer også battery health
endrer : batteryCapacityAndPercentage[], dischargeFirstTime, dischargeLastTime, emergency charge status, batteryHealth og hiddenChargeTotalTime
avhengig av: speedAndDistanceArray[0], discharge, timeInterval*/
void batteryDischarge(){ 
    if (discharge == true){

        unsigned long dischargeLastTime = millis();

        if ((buttonC.getSingleDebouncedPress()) && (emergencyChargeStatus == 0)){ //skrur på nødladingsmodus hvis knapp c trykkes *kan kun brukes en gang
          emergencyChargeStatus = 1;
          }

        if ((dischargeLastTime - dischargeFirstTime) >= timeInterval){ //sjekker om det har gått 100ms

          if ((emergencyChargeStatus == 1) && (speedAndDistanceArray[0] < 0)){ //foretar nødlading hvis valgt
            float instantConsumption = ((abs(speedAndDistanceArray[0])*2)+10)*10; //regner ut lading
            batteryCapacityAndPercentage[0] += (instantConsumption*timeInterval); //oppdaterer kapasiteten
            batteryCapacityAndPercentage[1] = (batteryCapacityAndPercentage[0]/(2200*60*60*1000))*100 //oppdaterer batteri prosenten
            dischargeFirstTime = millis(); //oppdaterer tiden
            if (batteryCapacityAndPercentage[1] >= 20){ //slår av nødlading når 20% er nådd
              emergencyChargeStatus = 2;
            }
          }
          
          else if ((speedAndDistanceArray[0] < -30)&&(hiddenChargeTotalTime < 120)){ //hidden lading initiert ved fart mer enn 30 cm/s bakover. Kan maks brukes 2 min
            float instantConsumption = (abs(speedAndDistanceArray[0])*2)+10; //regner ut lading
            batteryCapacityAndPercentage[0] += (instantConsumption*timeInterval); //oppdaterer kapasiteten
            batteryCapacityAndPercentage[1] = (batteryCapacityAndPercentage[0]/(2200*60*60*1000))*100 //oppdaterer batteri prosenten
            dischargeFirstTime = millis();
            hiddenChargeTotalTime += (timeInterval/1000); //renger ut hvor lenge hidden charge har blitt gjort totalt
          }
          else{ //vanlig utlading
          float instantConsumption = (abs(speedAndDistanceArray[0])*2)+10; //regner ut forbruk
          float firstPercentageSample = batteryCapacityAndPercentage[1]; //sjekker om batteriet er 5% eller mer
          batteryCapacityAndPercentage[0] -= (instantConsumption*timeInterval); //oppdaterer kapasiteten
          batteryCapacityAndPercentage[1] = (batteryCapacityAndPercentage[0]/(2200*60*60*1000))*100 //oppdaterer batteri prosenten
          float secondPercentageSample = batteryCapacityAndPercentage[1]; //sjekker om batteriet har gått under 5%
          switch (true){
            case (firstPercentageSample > 10) && (secondPercentageSample <= 10):
              buzzer.playFrequency(800, 1000, 15); //gir varsel hvis batteriet har gått under 10%
              break;

            case (firstPercentageSample >= 5) && (secondPercentageSample < 5):
              batteryHealth--; //dekrementerer battery health hvis batterinivået går under 5%
              break;
          }

          switch (true){ //sørger for at batteriet ikke kan få negativ prosent, og at det ikke er mulig å kjøre bilen hvis batteriet er tomt
            case (batteryCapacityAndPercentage[1] <= 0):
              motors.setSpeeds(0,0); //slår av motoren
              batteryCapacityAndPercentage[0] = 0; //oppdaterer kapasitet
              batteryCapacityAndPercentage[1] = 0; //oppdaterer prosent
              break;
          }
          }
        }
  }
  else {
    dischargeFirstTime = millis(); //hvis lading == false så noteres tiden slik at det blir korrekt når discharge tidene skal sammenlignes
  }
}
          
           


              
            
/*lader opp batteriet med korrekt strøm
endrer: batteryCapacityAndPercentage[], chargingFirstTime, chargingLastTime
avhengig av: chargingCurrent */
void charging(chargingCurrent){ 
  unsigned long chargingLastTime = millis();

  if ((chargingLastTime - chargingFirstTime) >= 1000){ //sjekker om det har gått 1000ms med lading
    batteryCapacityAndPercentage[0] += ((chargingCurrent)*1000) //laderen har tilført batteriet strøm i 1000 ms og oppdaterer kapasitet deretter
    batteryCapacityAndPercentage[1] = (batteryCapacityAndPercentage[0]/(2200*60*60*1000))*100 //oppdaterer batteri prosenten
    chargingFirstTime = millis(); 
    if (batteryCapacityAndPercentage[1] > 100){
      batteryHealth--; //dekrementerer battery health variablen
      batteryCapacityAndPercentage[0] = 2200*60*60*1000 // sørger for at batteriet ikke kan ha mer enn 100%
      batteryCapacityAndPercentage[1] = (batteryCapacityAndPercentage[0]/(2200*60*60*1000))*100 //oppdaterer batteri prosenten
    }
    }
}




/*Sjekker om bilen skal lades via knappetrykk og til hvilken prosent
Avhengig av: discharge og charging() 
Endrer: batteryCapacityAndPercentage[], discharge, chargingFirstTime, dischargeFirstTime*/
void regularCharge(){  

  if (discharge == true){ //kan kun initieres hvis bilen utlades

    int buttonAPresses = 0;
    int buttonBPresses = 0;

    if (buttonA.getSingleDebouncedPress()){ //sjekker om det blir trykket A for å starte lading
        buttonAPresses = 1;
        discharge = false;
      }

    if (buttonAPresses == 1){ //blir bare kjørt hvis A blir trykket
      while (buttonApresses == 1){ //denne løkken lar oss velge ønsket batteri %
        if (buttonB.getSingleDebouncedPress()){
          buttonBPresses += 10; // plusser på 10+ for hvert trykk
        }
        if ((buttonBPresses == 100)||(buttonA.getSingleDebouncedPress() == true)){
          buttonAPresses = 2; //hvis det blir plusset til 100%, eller A trykkes, avsluttes valget.
        }
      }
      chargingFirstTime = millis(); //oppdaterer charging tiden før lading initieres
      while((buttonAPresses == 2) && ((batteryCapacityAndPercentage[1] < 100) || (batteryCapacityAndPercentage[1] < buttonBPresses))){ //lader mens batteriet er under 100% eller valgt %
        motors.setSpeeds(0,0) //sørger for at bil står stille når lader vanlig
        charging(440); //lader med 440 mA 
        if (buttonA.getSingleDebouncedPress()){
          buttonApresses = 0; //lading kan avsluttes ved å trykke på A
        }
      }
      discarge = true; // initierer utladings modus
      dischargeFirstTime = millis(); //oppdaterer discharge first time
    }
  }
}
      

/* Utfører sevice på batteriet og tar hånd om battery health nivået
Endrer: batteryCapacityAndPercentage[], batteryHealth, batteryHealthLevel
*/
void batteryHealth(){ //
  int randomNumber1 = random(1000); //genererer random nummer
  int randomNumber2 = random(1000);
  bool serviceNeeded = false;
  bool changeNeeded = false;
  
  if (randomNumber1 == randomNumber2){ //trekker 50 fra battery health hvis de to randome nummerene er like
    batteryHealth -= 50;
  }

  switch (true){ //setter battery health level til riktig nivå og sjekker om service eller bytte trengs
    case (batteryHealth <= 0):
      motors.setSpeeds(0,0);
      batteryHealth = 0;
      batteryHealthLevel = 0;
      changeNeeded = true;
      ServiceNeeded = false;
      break;

    case ((batteryHealth > 0) && (batteryHealth <= 25)):
      batteryHealthLevel = 1;
      serviceNeeded = true;
      changeNeeded = false;
      break;

    case ((batteryHealth > 25) && (batteryHealth <= 50)):
      batteryHealthLevel = 2;
      changeNeeded = false;
      serviceNeeded = false;
      break;

    case ((batteryHealth > 50) && (batteryHealth <= 75)):
      batteryHealthLevel = 3;
      changeNeeded = false;
      serviceNeeded = false;
      break;

    case ((batteryHealth > 75) && (batteryHealth <= 100)):
      batteryHealth = 100;
      batteryHealthLevel = 4;
      changeNeeded = false;
      serviceNeeded = false;
      break;
  }

  if(buttonB.getSingleDebouncedPress()){ //utfører service eller bytte på batteriet utifra behov
    switch (true){
      case changeNeeded:
        batteryCapacityAndPercentage[2] = {2200*60*60*1000,100} //fyller opp batteri variablene helt
        batteryHealth = 100;
        batteryHealthLevel = 4;
        break;
      case serviceNeeded:
        batteryCapacityAndPercentage[2] = {(2200*60*60*1000)/2,50} //fyller opp batteri variablene halvveis
        batteryHealth = 50;
        batteryHealthLevel = 2;
        break;
    }
}
}

/*Sjekker om batteriet er under 5% og varsler isåfall med en buzzer
Avhengig av: batteryCapacityAndPercentage[]
Endrer: warningFirstTime og warningLastTime
*/
void batteryWarnings(){
  if ((batteryCapacityAndPercentage[1] <= 5) && (discharge == true)){ //hvis batteriet utlades og er under % så skal det gi regelmessige varsler
    unsigned long warningLastTime = millis();
    if (((warningLastTime - warningFirstTime)/1000) >= 15){ //sjekker om det har gått 15 sekunder
      motors.setSpeeds(0,0); //stopper bilen
      buzzer.playFrequency(800, 250, 15); //spiller to korte bip
      while(buzzer.isPlaying()){ };
      delay(250);
      buzzer.playFrequency(800, 250, 15);
      while(buzzer.isPlaying()){ };
      warningFirstTime = millis();
    }
  }
  else {
    warningFirstTime = millis(); //oppdaterer tidsvariabelen slik at differansen ikke blir for stor neste gang den skal brukes
  }
}




    


 
  







  
  

          

          


                        

        




      
      


      






  






void setup() {
  
}

void loop() {

}
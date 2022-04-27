#include <Arduino.h>
#include <Zumo32U4.h>
//#include <wire.h>

Zumo32U4LCD display;
Zumo32U4Encoders encoders;
Zumo32U4Motors motors;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
int totalBatteryPercentage = 100;

void setup() {
  Serial.begin(9600);
  buttonA.waitForButton();
  buttonB.waitForButton();
  buttonC.waitForButton();
}

void loop() {
  // put your main code here, to run repeatedly:

}
void TypeLadning(){
   while (Serial.available() < 0) { }
    int inByte = Serial.read();
    
   switch(inByte){
    case 'a':
    
      //Fulladning, batteriet lades opp til 100%
      for(int i = totalBatteryPercentage; i<=100;i++){
        i = i + 5;
        //Vise på node-red
      }
   }
   break;
   
   case 'b': 
    //Ladning til brukeren stopper
    
    for(int i = totalBatteryPercentage; i<=100;i++){
      if(buttonB.isPressed()){
        break;
      }
        i = i + 5;  
        //Vise på node-red
      
    }
    break;
    
    case 'c': 
    //Lade opp en gitt mengde %
    while (Serial.available() == 0){ 
    }
    int LadeTil = Serial.read();
    for(int i = totalVatteryPercentage; i<= LadeTil; i++){
      i = i + 5
    }
    break;

}

void Prisliste(){
  int prisarray[] = {1,2,3}
  int ladningA = prisarray[1]
  int ladningB = prisarray[2]
  int ladningC = prisarray[3]
}

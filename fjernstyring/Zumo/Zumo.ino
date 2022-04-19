const int ENA = 9; //Motor 1
const int IN1 = 15; //Wheel direction 1
const int ENB = 10; //Motor 2
const int IN2 = 16; //Wheel direction 2

char data[3]; //Initialized variable to store recieved data

void setup() {
  // Begin the Serial on 9600 Baud
  Serial1.begin(9600); //Serial 1 is the received communication/UART from the ESP32
  Serial.begin(9600); //Serial is the normal serial monitor communication

  motors(0); //Sets motorpower
}

//Enable or disable the motors
void motors(int motorPower){
  analogWrite(ENA, motorPower);
  analogWrite(ENB, motorPower);

  Serial.println("Motors setup");
}

//Drive the car forwards or backwards
void drive(bool Direction){
  //Direction
  digitalWrite(IN1, !Direction);
  digitalWrite(IN2, !Direction);
}

//Turn the car left or right (turns with the frontwheels)
void softTurn(bool Direction) {
  if(Direction) {
    //Motors
    analogWrite(ENA, 60);
    analogWrite(ENB, 150);
    
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
  else {
    //Motors
    analogWrite(ENA, 150);
    analogWrite(ENB, 60);
    
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }
}

//Turn the car left or right (turns like a tank)
void hardTurn(bool Direction) {
  //Motors
  digitalWrite(IN1, Direction);
  digitalWrite(IN2, !Direction);
}

void loop() {
  while(Serial1.available()) {
    Serial1.readBytes(data, 1); //Read the serial data from the ESP32 and store in the data variable
    delay(1);
    char received = (char)Serial1.read(); //Reads the serial data as a char to use with switch case
    Serial.println(received); //Print data to Serial Monitor 

    switch(received) {
      case 'w':
        motors(225);
        drive(true);
        break;

      case 'a':
        hardTurn(false);
        break;

      case 's':
        motors(225);
        drive(false);
        break; 

      case 'd':
        hardTurn(true);
        break;

      case 'x':
        motors(0);
        break;

      default:

        break;  
    }
    
  }
}

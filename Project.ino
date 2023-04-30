#include <Wire.h>
#include "MCP23017.h"
#include <Time.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

#define LOADCELL_DOUT_PIN  3
#define LOADCELL_SCK_PIN  2
#define humidSensorPin A4

MCP23017 mcp;

const int pingPin = 7; // Trigger Pin of Ultrasonic Sensor 1
const int echoPin = 6; // Echo Pin of Ultrasonic Sensor 1

const int ping2Pin = 12; // Trigger Pin of Ultrasonic Sensor 2
const int echo2Pin= 11; // Echo Pin of Ultrasonic Sensor 2

// these are the photoresistors
int front;
int left;
int back;
int right;

//lcd display
LiquidCrystal_I2C lcd(0x27,20,4);

// humidity sensor
float humidSensorValue = 0; 

//load cell
HX711 scale;
float calibration_factor = - 19219;


boolean shutdo = false;

int timeSpentWandering;
int timeStartedWandering;

//1 means there is light 0 means no light
int checkLight(int pin) {
  if(mcp.digitalRead(pin) == 0) {
    return 1;
  } else {
    return 0;
  }
}


boolean enoughLight() {
  int light = checkLight(0) + checkLight(1) + checkLight(2) + checkLight(3);
  if ( light >2) {
    return true;
  } else {
    return false;
  }
}

boolean noLight(){
  int light = checkLight(0) + checkLight(1) + checkLight(2) + checkLight(3);
  if ( light == 0) {
    return true;
  } else {
    return false;
  }
}

boolean needToTurnLeft() {
  int light = checkLight(0) + checkLight(1) + checkLight(2) + checkLight(3);
  if ( light == 1 && checkLight(left) == 1) {
    return true;
  }
  else if (light == 2 && checkLight(left) == 1 && checkLight(front) == 1) {
    return true;
  }
  else if (light == 2 && checkLight(left) == 1 && checkLight(back) == 1) {
    return true;     
  } else {
    return false;
  }    
}


boolean needToTurn(){
  int light = checkLight(0) + checkLight(1) + checkLight(2) + checkLight(3);
  if ( light == 1 && checkLight(front) == 1) {
    return false;
  }
  else if (light == 3 && checkLight(back) == 0) {
    return false;
  }
  else if (light == 4) {
    return false;   
  } else {
    return true;
  }    
}


void startTurnLeft() { // why is turning a boolean
  // motor a 
  digitalWrite(12, HIGH);
  digitalWrite(9, LOW);
  analogWrite(3, 225);  
  // motor b  
  digitalWrite(13, HIGH);
  digitalWrite(8, LOW);
  analogWrite(11, 225);

  digitalWrite(9, HIGH);
  digitalWrite(8, HIGH);
  //delay(1000); 
}



void startTurnRight() {
  // motor a 
  digitalWrite(12, LOW);
  digitalWrite(9, LOW);
  analogWrite(3, 225);  
  // motor b  
  digitalWrite(13, LOW);
  digitalWrite(8, LOW);
  analogWrite(11, 225);

  digitalWrite(9, HIGH);
  digitalWrite(8, HIGH);
  //delay(1000); // stop the motors
}

void stopTurning(){
  digitalWrite(9, LOW);
  digitalWrite(8, LOW);
}


//move forward
//
void startMoveForward(){

  // motor a
  digitalWrite(12, HIGH);
  digitalWrite(9, LOW);
  analogWrite(3, 225);

  // motor b  
  digitalWrite(13, LOW);
  digitalWrite(8, LOW);
  analogWrite(11, 225);

  digitalWrite(9, HIGH);
  digitalWrite(8, HIGH);  
}


//stop moving forward
void stopMoveForward(){
  //turn off the motors here
  // motor a
  digitalWrite(12, LOW);
  digitalWrite(9, HIGH);
  analogWrite(3, 225);

  // motor b
  digitalWrite(13, LOW);
  digitalWrite(8, HIGH);
  analogWrite(11, 225);

  digitalWrite(9, HIGH);
  digitalWrite(8, HIGH);
}


boolean canMoveForward(){

   long duration, inches, cm;
   pinMode(pingPin, OUTPUT);
   digitalWrite(pingPin, LOW);
   delayMicroseconds(2);
   digitalWrite(pingPin, HIGH);
   delayMicroseconds(10);
   digitalWrite(pingPin, LOW);
   pinMode(echoPin, INPUT);
   duration = pulseIn(echoPin, HIGH);

   long duration2, inches2, cm2;
   pinMode(ping2Pin, OUTPUT);
   digitalWrite(ping2Pin, LOW);
   delayMicroseconds(2);
   digitalWrite(ping2Pin, HIGH);
   delayMicroseconds(10);
   digitalWrite(ping2Pin, LOW);
   pinMode(echo2Pin, INPUT);
   duration2 = pulseIn(echo2Pin, HIGH);
/*
   long microsecondsToCentimeters(long microseconds) {
    return microseconds / 29 / 2;
   }
*/
   cm = microsecondsToCentimeters(duration);
   cm2 = microsecondsToCentimeters(duration2);

   if (cm2 > 5 && cm < 20) {
    return true;
   } else {
    return false;
   }
}

//figures out if you're facing a spot of light
boolean facingLight(){
  int light = checkLight(0) + checkLight(1) + checkLight(2) + checkLight(3);
  //check to see if the forweard sensor has light on it and nothing else or left and right both have light on them
  if ( light == 1 && front == 0){
    return true;
  } else {
   return false;  
  } 
}




void setup() {
   Serial.begin(9600); // Starting Serial Terminal

   mcp.begin(7);
    
   mcp.pinMode(0, INPUT); //Configue Port PA0 Output
   mcp.pinMode(1,INPUT);
   mcp.pinMode(2, INPUT);
   mcp.pinMode(3, INPUT);
    
   //turn on a 100K pullup internally
   mcp.pullUp(0, HIGH);  
   mcp.pullUp(1, HIGH);
   mcp.pullUp(2, HIGH);
   mcp.pullUp(3,HIGH);

   //use the PA1 LED as debugging
   //mcp.pinMode(1, OUTPUT);

   front = 0;
   left = 1;
   back = 2;
   right = 3;

    
   // channel a
   pinMode(12, OUTPUT);
   pinMode(9, OUTPUT);

   // channel b
   pinMode(13, OUTPUT);
   pinMode(8, OUTPUT);

   lcd.init();
   lcd.backlight();

   scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
   scale.set_scale();
   scale.tare(); //do i need this?
   /**
   long zero_factor = scale.read_average(); 
   Serial.print("Zero factor: ");
   Serial.println(zero_factor);
   */
}

void turn(){

  if(needToTurnLeft()){
    startTurnLeft();
  }else {
    startTurnRight;
  }

  while(!facingLight()){
    //turn until you're facing light
    Serial.print("turning to light");
  }

  stopTurning();  
}

String milliToTime(int time){
  int hours;
  int minutes;
  hours = time/3600000; //12345 is the number of milliseconds in an hour

  minutes = (time % (hours * 3600000)) / 60000 ;  // where 12345 is the number of milliseconds in an hour and 56789 is the number of milliseconds in a minute;
  return hours + " hours and " + minutes + " minutes";
}

void loop() {  
  if(shutdo)) { 
    return;
  }
  if (!enoughLight()) {
    //random walk feature
    if(noLight()){
      //turn to a random direction
      // motor a 
      digitalWrite(12, HIGH);
      digitalWrite(9, LOW);
      analogWrite(3, 225);  
      // motor b  
      digitalWrite(13, HIGH);
      digitalWrite(8, LOW);
      analogWrite(11, 225);

      delay(random(250, 750)); //figure out how long the delay needs ot be to turn x degrees
      digitalWrite(9, HIGH);
      digitalWrite(8, HIGH);
      //delay(1000); // stop the motors

      if (canMoveForward()) {
        startMoveForward()
        timeStartedWandering = millis();

        //move until it can't or for a certain amount of time

        timeSpentWandering += millis() - timeStartedWandering;
        
        if(timeSpentWandering > 30000){ //someTime is how long in milliseconds you want your robot to wander for
          shutdo = true;
        }
      }
      stopMoveForward();

                 
      } else { // we know where the light is so go to it
      
        while(!enoughLight()){
          t
        
          //figure out which way to turn
          if(needToTurn(){
            t
            turn();        
          }
      
          startMoveForward();
          while(canMoveForward() && !enoughLight()) {
            Serial.print("Moving Forward");
            if(needToTurn()) {     
              turn();
            }
          }
          stopMoveForward();
          //move forward until you're in the light
         }
      }
   } else {
      timeSpentWandering = 0;
   }


   //implement update screen logic here

   string line0 = "Weight:";
   
   string line1 = "Been in the light for: ";
   string line2 = "Last Watered: "

   float weight = 0.454*scale.get_units(), 3;
   string timeInLight = getTimeInLight() + " ago";
   string lastWatered = getLastWatered()

   scale.set_scale(calibration_factor); 

   //update screen with new lines
   lcd.setCursor(0,0);
   lcd.print(line0 + weight + " kg");
   lcd.setCursor(0,1);
   lcd.print(line1 + timeInLight);
   lcd.setCursor(0,2);
   lcd.print(line2);
   lcd.setCursor(0,3);
   lcd.print(line2);
   
   delay(100);   
}


long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2;
}

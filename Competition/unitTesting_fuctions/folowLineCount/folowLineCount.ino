//Include all the correct libraries 
#include <Servo.h>
#include "QSerial.h"

// Define constants for pins and other variables/objects 
#define GRIPSERVO 10
#define PANSERVO 8
#define TILTSERVO 9
#define GRIPSENSOR 12
#define M1 7
#define E1 6
#define M2 4
#define E2 5
#define RBUMPER 3
#define LBUMPER 2
#define RLINE A1
#define CLINE A2
#define LLINE A0
#define IRRRX 11
#define panStart 105
#define tiltStart 180
#define gripStart 0
#define LIGHTTHRESHOLD 600
#define LEFTLOW 150
#define RIGHTLOW 150

int ballCount = 0;
int POS = 0;

class Button {
    private:
        bool _state;
        uint8_t _pin;

    public:
        Button(uint8_t pin) : _pin(pin) {}

        void begin() {
            pinMode(_pin, INPUT_PULLUP);
            _state = digitalRead(_pin);
        }

        bool isReleased() {
            bool v = digitalRead(_pin);
            if (v != _state) {
                _state = v;
                if (_state) {
                    return true;
                }
            }
            return false;
        }
        bool isPressed(){
          return !digitalRead(_pin);
        }
};

QSerial IRR;
Servo panServo, tiltServo, gripServo;
Button rightBumper(RBUMPER);
Button leftBumper(LBUMPER);

void setDrive(int rightPow, int leftPow){
  if (rightPow >= 0){
    digitalWrite(M1, HIGH);
  }else {
    digitalWrite(M1, LOW);
    rightPow*=-1;
  }
  if (leftPow >= 0){
    digitalWrite(M2, HIGH);
  }else {
    digitalWrite(M2, LOW);
    leftPow*=-1;
  }
  analogWrite(E1, rightPow);
  analogWrite(E2, leftPow);
}

void followLine(int speed){
//    //Follows the line and sets speed based off of error
//    
//    //Minimum speed
//    int LF_MIN = 10;
//
//    //Gets the error in the left and right
//    int left_error = analogRead(LLINE) - LEFTLOW;
//    int right_error = analogRead(RLINE) - RIGHTLOW;
//    int error = left_error - right_error;
//
//    //calculates the left and right speed
//    int left_pow = max(speed - (error * kP / 1000), LF_MIN);
//    int right_pow = max((speed + (error * kP / 1000))*0.92, LF_MIN);
//
//    //Write the value to the motor
//    setDrive(right_pow,left_pow);

    int LF_MIN = -30;
    int left = analogRead(LLINE);
    int right = analogRead(RLINE);
    if (left > LIGHTTHRESHOLD && right < LIGHTTHRESHOLD)
        setDrive(speed, LF_MIN);   
    else if (left < LIGHTTHRESHOLD && right > LIGHTTHRESHOLD)
        setDrive(LF_MIN, speed);
    else
        setDrive(speed*0.92, speed);

}

void followLinecount(int speed, int numInter){ 
  //intialize the intersection count to zero
  int interCount, count, temp, runningSpeed;
  //int whiteCounter;
  interCount = count = temp = 0;
  runningSpeed = speed;

  //While the interCount is less than the number of intersections that want to be passed  
   
  while(interCount < numInter){
    count = 0;
    //whiteCounter = 0
    Serial.println("Start");
    
    while (count < 5) {
      followLine(runningSpeed);
      //if ((analogRead(LLINE) < LIGHTTHRESHOLD) || (analogRead(RLINE) < LIGHTTHRESHOLD))whiteCounter++;
      //if (whiteCounter%50 == 49)runningSpeed-=(0.05*runningSpeed);
      Serial.println(String(count)+" black cycle");
      //wait until an intersection is detected based off the IR line detector 
      //Intersection can be found if all three bottom sensors (Left, Right, and Center) sense black
      if ((analogRead(LLINE)>LIGHTTHRESHOLD) && (analogRead(RLINE)>LIGHTTHRESHOLD))count++;
      else count = 0;
      delay(1);
    }
    runningSpeed = speed;
    temp = interCount;
    //if (interCount == numInter-1) runningSpeed*=0.60;
    while (interCount < temp+1){
      //When an intersection is detected, loop until it leaves it to to then stop the robot then updated InterCount
      count = 0;
      while (count < 5) {
        followLine(runningSpeed);
        Serial.println(String(count)+" white cycle");
        if ((analogRead(LLINE) < LIGHTTHRESHOLD) || (analogRead(RLINE) < LIGHTTHRESHOLD))count++;
        else count = 0;
        delay(1);
      }
      interCount++;
      Serial.println("UPDATED COUNT");   
    }
  }
  //Stop when done intersections
  setDrive(0,0);
} 

void setup() {
  //Standard Initialization steps 
  Serial.begin(19200);

  //Initialize all the pins for the motors and sensors
  gripServo.attach(GRIPSERVO);
  panServo.attach(PANSERVO);
  tiltServo.attach(TILTSERVO);
  pinMode(GRIPSENSOR, INPUT);
  pinMode(M1, OUTPUT);
  pinMode(E1, OUTPUT);
  pinMode(M2, OUTPUT);
  pinMode(E2, OUTPUT);
  rightBumper.begin();
  leftBumper.begin();
  pinMode(RLINE, INPUT);
  pinMode(CLINE, INPUT);
  pinMode(LLINE, INPUT);
  IRR.attach(IRRRX,-1);

  //Initialize gripper servo to upright, centre, and open grippers 
  panServo.write(panStart);
  tiltServo.write(tiltStart);
  gripServo.write(gripStart);
}

void loop() {
  followLinecount(200, 2);
  delay(3000);
}

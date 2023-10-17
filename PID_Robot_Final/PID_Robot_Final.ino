#include <QTRSensors.h>
#define enB 5
#define enA 6
#define in1 10
#define in2 9
#define in3 8
#define in4 7



QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];
boolean mode = LOW;  //low for motor, high for sensor
int sensorFlag = LOW; //low for sensor calibration, high for not calibration 
const byte interruptPin = 2;
long unsigned int lastPress;
int debounceTime = 40;
boolean ledstate = LOW;
unsigned int ledtime = 0;
int positioner = 0;
//pid
const double KP = 1.4;
const double KD = 0.0;
double lastError = 0;
const int GOAL = 3500;
const unsigned char MAX_SPEED =110;

void SensorCal(){
  //sensors configuration
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){A0, A1, A2, A3, A4, A5, A6, A7}, SensorCount);
  qtr.setEmitterPin(3);

  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH); //turn on LED of Arduino when in calibration mode

  //analogRead() takes about 0.1 ms on an AVR.
  //0.1 ms per sensor = 4 samples per sensor read (default) = 6 sensors
  // * 10 reads per calibrate() call =  -24 ms per calibrate() call
  // Call calibrate() calibration () 400 times to make calibration take about 10 seconds
  for(uint16_t i = 0; i < 400; i++){
    qtr.calibrate();
    }
    digitalWrite(LED_BUILTIN, LOW); // turn off LED when calibration mode exited

    // print the calibration minimum values measured when emitter were on
    Serial.begin(9600);
    for(uint8_t i = 0; i < SensorCount; i++){
      Serial.print(qtr.calibrationOn.minimum[i]);
      Serial.print(' ');
    }
    Serial.println();  

    //print the calibration maximum values measured when emitters were on
    for(uint8_t i = 0; i < SensorCount; i++){
      Serial.print(qtr.calibrationOn.maximum[i]);
      Serial.print(' ');
    }
    Serial.println();
    Serial.println();
    delay(1000);
  }


void SensorData(){
  //read calibrated sensor values and obtain a measure of the line position
  //from 0 to 5000 (for a while line, use readLineWhite() instead)
 
  uint16_t position = qtr.readLineBlack(sensorValues);

  //print the sensor values as numbers from 0 to 1000, where 0 means maximum
  //reflectance and 1000 means minimum reflectance, followed by the line position
  for(uint8_t i = 0; i < SensorCount; i++){
    Serial.print(sensorValues[i]);
    Serial.print('\t');
    }
    Serial.println(position);
    positioner = position;
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(250);
    if(ledtime > 6){
      ledstate =! ledstate;
      ledtime = 0;
      }
    digitalWrite(LED_BUILTIN, ledstate);
    ledtime++;


  }

int PID(){
    uint16_t position = qtr.readLineBlack(sensorValues);
    int error = GOAL - position;
    int adjustment = KP*error + KD*(error - lastError);
    lastError = error;
    Serial.print(adjustment);         
    int x=(constrain(MAX_SPEED + adjustment, 0, MAX_SPEED));
    //int x=(constrain(MAX_SPEED +adjustment, 0, MAX_SPEED));
    return x;
    Serial.println(x);
  }
void forward(){
  analogWrite(enA, 125);
  analogWrite(enB, 125);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW); 
  }

void forward2(int x){
  analogWrite(enA, x);
  analogWrite(enB, x);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(500); 
  }
void right(){
  analogWrite(enA, 125);
  analogWrite(enB, 125);
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW); 
  }

void left2(int x){
  analogWrite(enA, x);
  analogWrite(enB, x);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  delay(200); 
  }

void left(){
  analogWrite(enA, 125);
  analogWrite(enB, 125);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW); 
  }
void right2(int x){
  analogWrite(enA, x);
  analogWrite(enB, x);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  delay(200); 
  }

void back(){
  analogWrite(enA, 125);
  analogWrite(enB, 125);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH); 
  }
  
void off(){
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW); 
  }

  
void motorTest(){
  forward();
  Serial.println("F");
  delay(150);
  back();
  Serial.println("B");
  delay(150);
  right();
  Serial.println("R");
  delay(150);
  left();
  Serial.println("L");
  delay(150);
  off();
  Serial.println("stop");
  delay(150);
  }

  
void setup(){
  // setup to run once
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  Serial.begin(9600);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), changeMode, FALLING);
  }


void loop(){
  if(mode == LOW){
    if(Serial.available() > 0){
      int inbyte = Serial.read();
  
      switch(inbyte){
      case '0':
      Serial.println("F");
      forward();
      delay(150);
      break;
      case '1':
      Serial.println("L");
      left();
      //delay(150);
      break;
      case '2':
      Serial.println("R");
      right();
      //delay(150);
      break;
      case '3':
      Serial.println("B");
      back();
      //delay(150);
      break;
      case '4':
      Serial.println("S");
      off();
      //delay(150);
      break;
      default:
      break;
      }
    }
   }
   else if(mode == HIGH){
   digitalWrite(LED_BUILTIN, LOW);
    if(sensorFlag == LOW){
      Serial.println("IR Test");
      SensorCal();
      sensorFlag = HIGH;
     }else{
       SensorData();
      if(sensorValues[0]==1000 && sensorValues[1]== 1000 && sensorValues[2]==1000 && sensorValues[3]==1000 && sensorValues[4]==1000 && sensorValues[5]==1000 && sensorValues[6]==1000 && sensorValues[7]==1000) 
        {
          Serial.println("OFF");
          off();
          }
       else if(( sensorValues[4] == 1000 || sensorValues[3] == 1000 || sensorValues[2] == 1000 || sensorValues[5] == 1000 )&&(sensorValues[7]<1000 && sensorValues[6]< 1000 && sensorValues[1]< 1000 && sensorValues[0]< 1000 ))
      {
        Serial.println("Forward");
          int x=PID();
          forward2(x);
        }
        else if ((sensorValues[7]==1000 || sensorValues[6]== 1000 ) && (sensorValues[0]!=1000 && sensorValues[1]!= 1000 && sensorValues[2]!=1000 && sensorValues[3]!=1000 && sensorValues[4]!=1000 && sensorValues[5]!=1000))
       { 
        Serial.println("RIGHT");
          int x=PID();
          right2(x);
        }
        else if ((sensorValues[1]== 1000 || sensorValues[0]== 1000) && (sensorValues[6]!=1000 && sensorValues[7]!= 1000 && sensorValues[2]!=1000 && sensorValues[3]!=1000 && sensorValues[4]!=1000 && sensorValues[5]!=1000))
       {
        Serial.println("LEFT");
          int x=PID();
          left2(x);
        }
        else{
          off();
          }
        
     }
    }
  }


void changeMode(){
  if((millis() - lastPress) > debounceTime)
  {
    lastPress = millis();
    if(digitalRead(interruptPin) == 0)
    {
      mode =! mode;
      }
    else if(digitalRead(interruptPin)==1)
    {
      mode=mode;
      }
    }
  }

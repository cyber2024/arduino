#define pinVX A7
#define pinVY A6
//#define stepperClock 11
//#define stepperInterrupt 2
//bool rising = tue;
//const int stepperFreq = 100;
int x,y;

int pinsL[4] {3,4,5,6};
int pinsR[4] {7,8,9, 10};
bool dirL = true;
bool dirR = true;
int _stepL = 0;
int _stepR = 0;

char gyroOut[64];
/**
 * Accelerometer
 */

//#include <Adafruit_MPU6050.h>
//#include <Adafruit_Sensor.h>
#include <Wire.h>
const int MPU = 0x68;
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;    
int AcXoff,AcYoff,AcZoff,GyXoff,GyYoff,GyZoff;
int temp,toff;
double pitch,roll, smoothPitch, smoothRoll;

//Adafruit_MPU6050 mpu;
const short AVG_OVER = 10;
float readings[2][AVG_OVER];
//double averagePitch[AVG_OVER];
int idx = 0;

const float calibA[3] = {0.14f, -0.51f, 8.78f-10.0f};
const float calibG[3] = {0.14f, -0.51f, 8.78f-10.0f};

int interruptCount = 0;

void setup() {
//  pinMode(stepperClock, OUTPUT);
//  analogWrite(stepperClock,125); 
//  attachInterrupt(digitalPinToInterrupt(stepperInterrupt),test, RISING);
//  attachInterrupt(digitalPinToInterrupt(stepperInterrupt),test, FALLING);
  // put your setup code here, to run once:
  Serial.begin(115200);
  //speed up I2C bus
  Wire.begin();
  Wire.setClock(500000L);
  Wire.beginTransmission(MPU);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);
  
  for(int i = 0; i < 4; i++){
    pinMode(pinsL[i], OUTPUT);
    pinMode(pinsR[i], OUTPUT);
  }
  x = 0;
  y = 0;

  
  for(int i = 0; i < 2; i++){
    for(int j = 0; j < AVG_OVER; j++){
      readings[i][j] = 0.0f;
      }
  }

  //Acceleration data correction
  AcXoff = -452;//-950;
  AcYoff = 2218;//-300;
  AcZoff = 3374;
  
  //Temperature correction
  toff = 0;//-1600;
  
  //Gyro correction
  GyXoff = 177;//480;
  GyYoff = -61;//170;
  GyZoff = -1;//210;
  //fill the reading array so that we can average
  for(int i = 0; i < AVG_OVER; i++){
    getGyroData();
    delay(5);
  }
  delay(100);
}


long m = 0;
long fps = 0;
int absX = 0, absY = 0;


void loop() {
  fps = micros();
  updateSteppers();
  getGyroData();
//  char str[64];
//  sprintf(str, "%lf,%lf - %lf,%lf",pitch,smoothPitch, roll, smoothRoll);

  long d = 1000-(micros() - fps);
  if(d > 0){
    delayMicroseconds(d);
  }
  Serial.println(d);
}
void updateSteppers(){
  stepperBalance();
}
void stepperBalance(){
  Serial.print("pitch: ");
  Serial.println(smoothPitch);
  if(smoothPitch > 0.01){
    stepForward();
  } else if(smoothPitch < -0.01){
    stepBackward();
  }
}
void stepperControll(){
  x = analogRead(pinVX)-517;
  y = analogRead(pinVY)-501;
  absX = abs(x);
  absY = abs(y);
  if(absY < 10 && absX < 10){
    //do nothing
  } else if(y > 10 && absX < 10){
    stepForward();
  } else if(y < -10 && absX < 10){
    stepBackward();
  } else if(x < -10 && absY < 10){
    dirL = true;
    dirR = true;
    step(_stepL, dirL, pinsL);
    step(_stepR, dirR, pinsR);
  } else if(x > 10 && absY < 10){
    dirL = false;
    dirR = false;
    step(_stepL, dirL, pinsL);
    step(_stepR, dirR, pinsR);
  }
}
void stepForward(){
    dirL = false;
    dirR = true;
    step(_stepL, dirL, pinsL);
    step(_stepR, dirR, pinsR);
}
void stepBackward(){
    dirL = true;
    dirR = false;
    step(_stepL, dirL, pinsL);
    step(_stepR, dirR, pinsR);
}
void getGyroData(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);
  double t,tx,tf;
  //read accel data
  AcX=(Wire.read()<<8|Wire.read()) + AcXoff;
  AcY=(Wire.read()<<8|Wire.read()) + AcYoff;
  AcZ=(Wire.read()<<8|Wire.read()) + AcZoff;
  
  //read temperature data
  temp=(Wire.read()<<8|Wire.read()) + toff;
  tx=temp;
  t = tx/340 + 36.53;
  tf = (t * 9/5) + 32;
  
  //read gyro data
  GyX=(Wire.read()<<8|Wire.read()) + GyXoff;
  GyY=(Wire.read()<<8|Wire.read()) + GyYoff;
  GyZ=(Wire.read()<<8|Wire.read()) + GyZoff;
  //get pitch/roll
  getAngle(AcX,AcY,AcZ);
}
void step(int &index, bool &dir, int *pins){
  switch(index){
    case 0:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], HIGH);
    break;
    case 1:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], HIGH);
      digitalWrite(pins[3], HIGH);
    break;
    case 2:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], HIGH);
      digitalWrite(pins[3], LOW);
    break;
    case 3:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], HIGH);
      digitalWrite(pins[2], HIGH);
      digitalWrite(pins[3], LOW);
    break;
    case 4:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], HIGH);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], LOW);
    break;
    case 5:
      digitalWrite(pins[0], HIGH);
      digitalWrite(pins[1], HIGH);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], LOW);
    break;
    case 6:
      digitalWrite(pins[0], HIGH);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], LOW);
    break;
    case 7:
      digitalWrite(pins[0], HIGH);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], HIGH);
    break;
    default:
      digitalWrite(pins[0], LOW);
      digitalWrite(pins[1], LOW);
      digitalWrite(pins[2], LOW);
      digitalWrite(pins[3], LOW);
    
  }
  if(dir){
    index++;
  } else{
    index--;
  }
  if(index > 7){
    index = 0;
  } else if(index < 0){
    index = 7;
  }
}


float getAvg(float * arr){
  float result = 0.0f;
  for(int i = 0; i < AVG_OVER; i++){
     result += *(arr+i);
  }
  return result / AVG_OVER;
}

void getAngle(int Vx,int Vy,int Vz) {
  double x = Vx;
  double y = Vy;
  double z = Vz;
  double PITCH_ADJUST = -3.6;
  pitch = atan(x/sqrt((y*y) + (z*z)));
  roll = atan(y/sqrt((x*x) + (z*z)));
  //convert radians into degrees
  pitch = pitch * (180.0/3.14) + PITCH_ADJUST;
  roll = roll * (180.0/3.14) ;
  readings[0][idx] = pitch;
  smoothPitch = getAvg(readings[0]);
  readings[1][idx] = roll;
  smoothRoll = getAvg(readings[1]);
  idx = (idx+1) % AVG_OVER;
}

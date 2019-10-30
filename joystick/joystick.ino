#define pinVX A7
#define pinVY A6

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
const short AVG_OVER = 5;
float readings[6][AVG_OVER];
float gx[3] = {0.0f,0.0f,0.0f};
float ax[3] = {0.0f,0.0f,0.0f};//gx,gy,gz,ax,ay,az;
int idx = 0;
int x,y;
float threshold = 0.10f;

const float calibrateAx = 0.14f;
const float calibrateAy = -0.51f;
const float calibrateAz = 8.78f-10.0f;
const float calibrateGx = 0.14f;
const float calibrateGy = -0.51f;
const float calibrateGz = 8.78f-10.0f;
void setup() {
  Serial.begin(9600); 

  for(int i = 0; i < 6; i++){
    for(int j = 0; j < AVG_OVER; j++){
      readings[i][j] = 0.0f;
      }
  }

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("");
  delay(100);
  
   x = 0;
   y = 0;

 
}

void loop() {
  x = analogRead(pinVX);
  y = analogRead(pinVY);
//  char str[10];
//  sprintf(str,"(%d, %d)",x,y);
//  Serial.println(str);
  
  mpu.read();
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  readings[0][idx] = a.acceleration.x - calibrateAx;
  readings[1][idx] = a.acceleration.y - calibrateAy;
  readings[2][idx] = a.acceleration.z - calibrateAz;
  readings[3][idx] = a.gyro.x - calibrateGx;
  readings[4][idx] = a.gyro.y - calibrateGy;
  readings[5][idx] = a.gyro.z - calibrateGz;
  idx++;
  idx = idx%AVG_OVER;
    /* Print out the values */
//  if(idx == 9){
//    gx[0] = getAvg(readings[0]);
//    gx[1] = getAvg(readings[1]);
//    gx[2] = getAvg(readings[2]);
//    ax[0] = getAvg(readings[3]);
//    ax[1] = getAvg(readings[4]);
//    ax[2] = getAvg(readings[5]);
//    char str[40];
//    sprintf(str,"%02.02f,%02.02f,%02.02f    %02.02f,%02.02f,%02.02f",gx[0],gx[1],gx[2],ax[0],ax[1],ax[2]);
//    Serial.println(str);
//  }

  delay(10);
}

float getAvg(float * arr){
  float result = 0.0f;
  for(int i = 0; i < AVG_OVER; i++){
     result += *(arr+i);
  }
  return result / AVG_OVER;
}

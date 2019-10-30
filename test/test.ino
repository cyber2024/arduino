#include <math.h>
#define PIN 7

unsigned long int prevTime, currTime, startTime, d;
bool high;

void setLights(bool on);

void setup() {

  pinMode(PIN, OUTPUT);
  Serial.begin(19200);
  currTime = 0;
  prevTime = 0;
  startTime = millis();
  high = true;
  setDelayHours(12);
  setLights(high);
}

void loop() {
//  Serial.print(d);
//  Serial.print(", \t");
//  Serial.print(currTime);
//  Serial.print(", \t");
//  Serial.print(prevTime);
//  Serial.print("\n");

//  // put your main code here, to run repeatedly:
  currTime = millis();
  if(currTime > prevTime + d){
    high = !high;
    setLights(high);
    prevTime = currTime;
  }
}

void setLights(bool on){
      digitalWrite(PIN, on ? HIGH : LOW);
      float t = (currTime - startTime)/1000.0;
      Serial.write("Switching 12V lights ");
      Serial.print(on ? "on" : "off");
      Serial.print(" at ");
      Serial.print(t);
      Serial.write("s\n");
}
void setDelayHours(float h){
  d = (unsigned long int)(h*1000*60*60);
  Serial.print("\nDelay set to ");
  Serial.print(h);
  Serial.print("hrs.\n");
 }

#include <math.h>
#define PIN 7

long prevTime, currTime, startTime, onTime, offTime;
long dayLength = 24L*60L*60L*1000L;
bool high, lightsOn;
void setLights(bool on);
void setLightsOnSeconds(long secs);

void setup() {
  pinMode(PIN, OUTPUT);
  Serial.begin(9600);
  currTime = 0;
  prevTime = 0;
  startTime = millis();
  lightsOn = true;
  setLightsOnSeconds(16L*60L*60L);
  setLights(lightsOn);
}

void loop() {
  currTime = millis();
  if(currTime > (prevTime + (lightsOn ? onTime : offTime))){
    lightsOn = !lightsOn;
    setLights(lightsOn);
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
void setLightsOnSeconds(long secs){
  onTime = secs*1000L;
  offTime = dayLength - onTime;
  Serial.println(onTime);
  Serial.println(dayLength);
  Serial.println(offTime);
  Serial.print("\nLights on for ");
  Serial.print(((float)onTime)/(1000.0*60.0*60.0));
  Serial.print("hrs and off for ");
  Serial.print(((float)offTime)/(1000.0*60.0*60.0));
  Serial.print("hrs.\n");
 }

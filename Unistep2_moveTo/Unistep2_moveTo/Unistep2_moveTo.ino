/*
 * Unistep2 example: moveTo
 *
 * Example of stepper enumeration and movement via moveTo(position).
 *
 * Gets user input via Serial monitor.
 *
 * Moves the stepper to the absolute position indicated. Position is a number
 * between 0 and the number of steps per rev (generally, 4096).
 *
 * 30Jan18
 * R Sanchez
 *
 * This code is released to the public domain
 *
 */

// include the library
#include <Unistep2.h>

int sensorPin = A0;
int sensorValue = 0;
int prevStepPos = 0;
int stepThreshold = 5;
// Define some steppers and the pins they will use
// pins for IN1, IN2, IN3, IN4, steps per rev, step delay(in micros)
Unistep2 stepper(2, 3, 4, 5, 4096, 1000);
const int numReadings = 10;
int readings[10];
int readIndex = 0;
int total = 0;
int average = 0;

void setup()
{
  // Your setup code here
  // The library initializes the pins for you

  Serial.begin(9600);
  Serial.println("Initializing...");
  for(int i = 0; i < numReadings; i++){
    readings[i] = 0;
    Serial.print(readings[i]);
    Serial.print(",");
  }
    Serial.println("");
}

void loop()
{
  total -= readings[readIndex];
  readings[readIndex] = analogRead(sensorPin);
  total += readings[readIndex++];
  readIndex = readIndex == numReadings ? 0 : readIndex;
  average = total/numReadings;
  
  // We need to call run() frequently, so we place it in the loop()
  stepper.run();

  // Get user input via Serial monitor
  //if ( stepper.stepsToGo() == 0 ) {

      int stepPos = map(average,0,823,0,4096);
      if(abs(stepPos-prevStepPos)>stepThreshold){
        stepper.moveTo(stepPos);
        prevStepPos = stepPos;
      }
//    if (Serial.read() == '\n') {
//      Serial.print("Movement command: ");
//      // currentPosition() gives us stepper position
//      Serial.print(stepper.currentPosition());
//      Serial.print(" --> ");
//      Serial.println(stepPos);
//
//      // moveTo() chooses the shortest path between the two positions.
//      
//    }
  //}
  // Other code

}

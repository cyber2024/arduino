#include <FastLED.h>

#define PIN 5
#define NUM_LEDS 35
#define DATA_PIN 6

 CRGB leds[NUM_LEDS];
 
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(PIN,OUTPUT);
  Serial.begin(19200);
  FastLED.addLeds<NEOPIXEL
}

// the loop function runs over and over again forever
int increment = 100;
int high = 1000, curr = 100;
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  digitalWrite(PIN,HIGH);
  delay(curr);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(PIN,LOW);
  delay(curr);                       // wait for a second
  Serial.print("Which Pin? ");
  Serial.print(PIN);
  Serial.print(".\n");
  curr += increment;
  if(curr % high == 0)
    increment *= -1;
  
}

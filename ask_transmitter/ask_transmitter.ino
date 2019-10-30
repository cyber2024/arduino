// ask_transmitter.pde
// -*- mode: C++ -*-
// Simple example of how to use RadioHead to transmit messages
// with a simple ASK transmitter in a very simple way.
// Implements a simplex (one-way) transmitter with an TX-C1 module

#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#define bell 5

RH_ASK driver;

const char *msg = "308";
int val = 0;
void setup()
{
    Serial.begin(9600);	  // Debugging only
    if (!driver.init())
         Serial.println("init failed");
    pinMode(bell, INPUT);
}

void loop()
{
    val = digitalRead(bell);
    if(val == 1){
      Serial.println("Ring");    
      driver.send((uint8_t *)msg, strlen(msg));
      driver.waitPacketSent();
      delay(200);
    }

    delay(25);
}

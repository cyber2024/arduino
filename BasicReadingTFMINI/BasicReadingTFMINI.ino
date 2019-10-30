/*
Example code for Benewake TFMini time-of-flight distance sensor. 
by Peter Jansen (December 11/2017)
This example code is in the public domain.

This example communicates to the TFMini using a SoftwareSerial port at 115200, 
while communicating the distance results through the default Arduino hardware
Serial debug port. 

SoftwareSerial for some boards can be unreliable at high speeds (such as 115200). 
The driver includes some limited error detection and automatic retries, that
means it can generally work with SoftwareSerial on (for example) an UNO without
the end-user noticing many communications glitches, as long as a constant refresh
rate is not required. 

The (UNO) circuit:
 * Uno RX is digital pin 10 (connect to TX of TF Mini)
 * Uno TX is digital pin 11 (connect to RX of TF Mini)

THIS SOFTWARE IS PROVIDED ''AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

#include <SoftwareSerial.h>
#include "TFMini.h"
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <LiquidCrystal_I2C.h>

// Setup software serial port 
SoftwareSerial mySerial(10, 11);      // Uno RX (TFMINI TX), Uno TX (TFMINI RX)
TFMini tfmini;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

long curr, dPrev, dDelay, tPrev, tDelay, sPrev, sDelay;
uint16_t dist = 0, strength = 0;
double temp = 0;

//Setup LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  // Step 1: Initialize hardware serial port (serial debug port)
  Serial.begin(115200);
  // wait for serial port to connect. Needed for native USB port only
  while (!Serial);
  Serial.println ("Initializing LIDAR");
  // Step 2: Initialize the data rate for the SoftwareSerial port
  mySerial.begin(TFMINI_BAUDRATE);
  // Step 3: Initialize the TF Mini sensor
  tfmini.begin(&mySerial); 

  Serial.println("Initializing IR Thermometer...");
  mlx.begin();
  
  tPrev = millis();
  dPrev = tPrev;
  sPrev = tPrev;
  curr = dPrev;
  dDelay = 100;  
  tDelay = 50;
  sDelay = 100;

  Serial.println("Initializing display...");
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Initializing...");
  lcd.setCursor(0,0);
  delay(500);
  lcd.print("Sustainability Cloud");
  lcd.setCursor(1,1); lcd.print("Distance: "); 
  lcd.setCursor(1,2); lcd.print("Temp: "); 
}


void loop() {
  curr = millis();

  //Measure distance
  if(isTime(dPrev, curr, dDelay)){
    dPrev = curr;
    // Take one TF Mini distance measurement
    dist = tfmini.getDistance();
    strength = tfmini.getRecentSignalStrength();
    // Display the measurement
    Serial.print(dist);
    Serial.print("cm\t");
    
    //Serial.println(strength);
    Serial.print(temp); Serial.print("*C\n");
  }

  if(isTime(tPrev,curr,tDelay)){
    tPrev = curr;
    temp = mlx.readObjectTempC();
  }
  if(isTime(sPrev,curr, sDelay)){
    sPrev = curr;
    //lcd.clear();
    //Print title

    
    //print distance
    
    lcd.setCursor(12,1); 
    if(dist<100) lcd.print("   ");
    else if(dist<1000) lcd.print("  ");
    else if(dist<10000) lcd.print(" ");
    lcd.print(dist); lcd.print("cm");
    

    //print temp
    lcd.setCursor(11,2); 
    if(temp<10.00) lcd.print("  ");
    else if(temp<100.00) lcd.print(" ");
    lcd.print(temp); lcd.write(223); lcd.print("C");

    
  }
  
}

bool isTime(long &prev, long &curr, long &del){
  return (curr - prev) >= del;

}

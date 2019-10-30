#include <HardwareSerial.h>
#include <Sim800lHttp.h>

HardwareSerial hwSerial(1);
#define tempPin 34
#define pinPostButton 12

bool postInProgress = false;
Sim800lHttp *sim;

//therm variables
long thermistorValue = 0;
const long tempOutputDelay = 5000;
long nextThermistorPrint = 0;
const int TEMPLEN= 10;
long temps[TEMPLEN];
int tempIdx = 0;

//cardReader setup
#include <SPI.h>
#include <SD.h>
#define cardCS 5
File root;

void setup() {
  Serial.begin(57600);
  while(!Serial);
  
  //read setup from card
  Serial.println("\nInitialising SD card reader...");
  if(!SD.begin(cardCS)){
    Serial.println("Initialization failed.");
  } else {
    Serial.println("SD card reader online. \nChecking card...");
    root = SD.open("/");
    printDirectory(root, 0);
    Serial.println("SD card ok.");
    
  }
  
  
  hwSerial.begin(57600, SERIAL_8N1, 16, 17);
  pinMode(pinPostButton,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinPostButton),post, FALLING);
  sim = new Sim800lHttp(10, Serial, hwSerial);
  
  //setup temp
  pinMode(tempPin, INPUT);
  for(int i = 0; i < TEMPLEN; i++){
    temps[i]=0;
  }
}

void loop() {
  sim->update();
  bool *s = sim->status();
  if(postInProgress && !*s){
    postInProgress = false;
  }
  if(millis() > nextThermistorPrint){
    Serial.print("Therm Value: ");
    Serial.println(getTemp());
    nextThermistorPrint = millis() + tempOutputDelay;
  }
  readThermistor();
}
void post(){
  if(!postInProgress){
    postInProgress = true;
    Serial.println("Posting");
    sim->post("cloudcycle.co/api/v1/sensor", "_ABCDE_");
  }
}

void readThermistor(){
  temps[tempIdx++] = analogRead(tempPin);
  tempIdx %= TEMPLEN;
  thermistorValue = 0;
  for(int i = 0; i < TEMPLEN; i++){
    thermistorValue += temps[i];
  }
  thermistorValue /= TEMPLEN;
}
long getTemp(){
  return thermistorValue;
}

void printDirectory(File dir, uint8_t numTabs){
  while(true){
    File entry = dir.openNextFile();
    if(!entry){
      //no more files
      break;
    }
    for(uint8_t i = 0; i < numTabs; i++){
      Serial.print('\t');      
    }
    Serial.print(entry.name());
    if(entry.isDirectory()){
      Serial.println("/");
      printDirectory(entry, numTabs +1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(),DEC);
    }
    entry.close();
  }
}

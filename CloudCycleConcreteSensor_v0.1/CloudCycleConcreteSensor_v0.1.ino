#include <HardwareSerial.h>
#include <Sim800lHttp.h>

/**
 * SIM800L Setup
 */
HardwareSerial hwSerial(1);
#define tempPin 34
#define pinPostButton 27
#define pinSimReset 13
#define pinSimTx 12
#define pinSimRx 14
bool postInProgress = false;
Sim800lHttp *sim;

//therm variables
long thermistorValue = 0;
const long tempOutputDelay = 5000;
long nextThermistorPrint = 0;
const int TEMPLEN= 10;
long temps[TEMPLEN];
int tempIdx = 0;



/**
 *  GPS and Compass Setup
 */
 #define compassSDA 23
 #define compassSCL 2
 #define gpsRx 22
 #define gpsTx 4
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
#include <Adafruit_GPS.h>
#define GPSECHO false
HardwareSerial hwSerial2(2);
#define GPSSerial hwSerial2
// Connect to the GPS on the hardware port
Adafruit_GPS GPS(&GPSSerial);
char gpsIsoDateTime[32];

/**
 * ESP32 SPI PINS
 * CS 5
 * MOSI 23
 * MISO 19
 * SCK 18
 */
//cardReader setup
#include <SPI.h>
#include <SD.h> // SD.h (esp32)
#include <FS.h>
SPIClass spi(HSPI);

File root;
const char COLUMN_NAMES[] = "DATE,INTERNAL_TEMP,GPS_LOCATION,GPS_ANGLE,GPS_ALTITUDE,GPS_SATELLITES,GPS_FIX,GPS_QUALITY,GPS_SPEED\n";
const char LOG_FILENAME[] = "/log.csv";
File logFile;



/**
 * Timers
 */
//How frequently to log data
const uint32_t logDelay = 1000;
uint32_t nextLog = 0;
char logEntry[512];
//How frequently to post data
const uint32_t postDelay = 10000;
uint32_t nextPost = 0;
//GPS logging
uint32_t nextGPS = 0;
const uint32_t gpsDelay= 2000;


//unit number to be deterined upon registration with server
char unitNumber[] = "CC00001";

void setup() {
  Serial.begin(115200);
  while(!Serial);
  
  /**
  * GPS Setup
  */  
  //start compass
  Wire.begin(compassSDA,compassSCL);
  //start GPS serial port
  hwSerial2.begin(9600, SERIAL_8N1, gpsRx, gpsTx);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate
  delay(1000);
  //firmware version
  GPSSerial.println(PMTK_Q_RELEASE);

  /**
   * SD Card Setup
   */
  spi.begin(18,21,19,5);
  //read setup from card
  Serial.println("\nInitialising SD card reader...");
  if(!SD.begin(5, spi)){
    Serial.println("SD Card failed to mount failed.");
  } else {
    Serial.println("SD mounted online. \nChecking card...");
    uint8_t cardType = SD.cardType();
    if(!cardType){
      Serial.println("No SD card detected.");
    } else {
      Serial.print("SD Card Type: ");
      if(cardType == CARD_MMC){
          Serial.println("MMC");
      } else if(cardType == CARD_SD){
          Serial.println("SDSC");
      } else if(cardType == CARD_SDHC){
          Serial.println("SDHC");
      } else {
          Serial.println("UNKNOWN");
      }      

      uint64_t cardSize = SD.cardSize() / (1024 * 1024);
      Serial.printf("SD Card Size: %lluMB\n", cardSize);
      
    }
    //Check card
    root = SD.open("/");
    printDirectory(root, 0);
//    root.close();
    Serial.println("SD card ok.");
  }
  if(!SD.exists(LOG_FILENAME)){
    Serial.print(LOG_FILENAME);
    Serial.println(" - File does not exist, creating now...");
    logFile = SD.open(LOG_FILENAME, FILE_WRITE);
    logFile.print(COLUMN_NAMES);
    logFile.close();
    if(!SD.exists(LOG_FILENAME)){
      Serial.print("Error creating ");
      Serial.println(LOG_FILENAME);
    } else {
      Serial.print("Created new file ");
      Serial.println(LOG_FILENAME);
    }
  }

  
  hwSerial.begin(115200, SERIAL_8N1, 14, 12);
  pinMode(pinSimReset,OUTPUT);
  digitalWrite(pinSimReset, HIGH);
  pinMode(pinPostButton,INPUT_PULLUP);
//  attachInterrupt(digitalPinToInterrupt(pinPostButton),post, FALLING);
  sim = new Sim800lHttp(pinSimReset, Serial, hwSerial);
  
  //setup temp
  pinMode(tempPin, INPUT);
  for(int i = 0; i < TEMPLEN; i++){
    temps[i]=0;
  }


  //set up next post
  nextPost = millis() + postDelay;
}

void loop() {
  sim->update();
  bool *s = sim->status();
  if(postInProgress && !*s){
    postInProgress = false;
  }
  
  readThermistor();
  char gpsChar = GPS.read();
  if (GPS.newNMEAreceived()) {
     if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
        return; // we can fail to parse a sentence in which case we should just wait for another
   }
   if(checkTimer(&nextGPS, &gpsDelay)){
      Serial.print("DateTime: ");
      Serial.println(getDateTime());
   }
  if(checkTimer(&nextLog, &logDelay)){
    logData();
  }
  if(checkTimer(&nextPost, &postDelay)){
    post();
  }
}

void logData(){
  logFile = SD.open(LOG_FILENAME, FILE_APPEND);
  if(!logFile){
    Serial.print("error opening ");
    Serial.println(LOG_FILENAME);
  } else {
    //"DATE,INTERNAL_TEMP,GPS_LOCATION,GPS_ANGLE,GPS_ALTITUDE,GPS_SATELLITES,GPS_FIX,GPS_QUALITY,GPS_SPEED\n";
    sprintf(logEntry, "date=%s&unit=%s&temp=%ld&lat=%4.4f%c&lon=%4.4f%c&angle=%f&altitude=%f&satellites=%d&fix=%d&qual=%d&speed=%f\n",getDateTime(), unitNumber, getTemp(),GPS.latitude,GPS.lat,GPS.longitude,GPS.lon,GPS.angle,GPS.altitude,GPS.satellites,GPS.fix,GPS.fixquality,GPS.speed);
    if(logFile.print(logEntry)){
    } else {
      Serial.println("Unable to saved data.");
    }
    logFile.close();
    Serial.print("Log: ");
    Serial.println(logEntry);
  }
}
char* getDateTime(){
    sprintf(gpsIsoDateTime, "%04d-%02d-%02dT%02d:%02d:%02dZ", GPS.year+2000, GPS.month, GPS.day, GPS.hour, GPS.minute, GPS.seconds);
    return gpsIsoDateTime;
}

void post(){
  if(!postInProgress){
    postInProgress = true;
    Serial.println("Posting");
    sim->post("cloudcycle.co/sensor/logdata", logEntry);
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

bool checkTimer(uint32_t *timeMillis, const uint32_t *delayMillis){
  //incase millis wraps around
  if(*timeMillis > millis()){
    *timeMillis = millis();
  }
  if(*timeMillis + *delayMillis > millis())
    return false;
  *timeMillis = millis();
  return true;
  
}

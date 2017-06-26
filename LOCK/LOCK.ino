//#include <SPI.h>
#include <MFRC522.h>

#include <SoftwareSerial.h>
#define TX_PIN 7
#define RX_PIN 6
SoftwareSerial serialESP(RX_PIN, TX_PIN);


#define RST_PIN 9
#define SUCCESS_PIN 8
#define SS_PIN 10
#define LOCK_PIN 5
#define BUTTON_PIN 4

const int MAX_UIDS = 10;
String uids[MAX_UIDS] = {"80 F9 8C 75","01 02 03 04","","","","","","","",""};
MFRC522 mfrc522(SS_PIN, RST_PIN);

unsigned long int elapsed = 0;
unsigned long int sTime = 0;
bool high = false;
String pin5State = "LOW";


//Variables for reading serial data
const byte numChars = 32;
char receivedCharsSerial[numChars];
boolean newDataFromSerial = false;
char receivedCharsESP[numChars];
boolean newDataFromESP = false;




void setup() {

  Serial.begin(115200);
  Serial.println("Restarting...");

  
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  
//  SPI.begin();
  pinMode(LOCK_PIN,OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LOCK_PIN, LOW);
  pinMode(SUCCESS_PIN, OUTPUT);
  digitalWrite(SUCCESS_PIN,LOW);
  mfrc522.PCD_Init();
  Serial.println("Recognised UIDs are...");
  for(int i = 0; i < MAX_UIDS; i++){
    Serial.print(i); Serial.print(": "); Serial.print(uids[i]); Serial.print("\n");
  }
  Serial.println("Swipe card...\n");

  Serial.println("Initializing Wifi Module...");
  serialESP.begin(9600);
  delay(2500);
  serialESP.write("AT\r\n");
  
}

//unsigned long prevTime = millis();
//int hz = 0;
void loop() {
//  hz++;
//    if(millis() - prevTime > 2000){
//      Serial.println(hz/2);
//      hz=0;
//      prevTime = millis();
//    } 
     
  
  //Read serial from Serial Port
//  serialRX();
//  newSerialData();

//MFRC Reader Module
//  if( ! mfrc522.PICC_IsNewCardPresent()){
//    return;
//  }
//  if( ! mfrc522.PICC_ReadCardSerial()){
//    return;
//  }
//
//  Serial.print("UID tag: ");
//  String content = "";
//  byte letter;
//  for(byte i = 0; i < mfrc522.uid.size; i++){
//     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
//     Serial.print(mfrc522.uid.uidByte[i], HEX);
//     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
//     content.concat(String(mfrc522.uid.uidByte[i], HEX));
//  }
//  Serial.println();
//  Serial.print("Message : ");
//  content.toUpperCase();
//  String uid = content.substring(1);
//  bool uidRecognised = findUidInArray(uid); 
//  accessGranted(uidRecognised);

//Access button to open door from inside
if(digitalRead(BUTTON_PIN)){
    accessGranted(true);    
 }
}

void accessGranted(bool granted){
  if(granted){
    Serial.print(" Access GRANTED.\n");
    digitalWrite(LOCK_PIN, HIGH);
    Serial.println("Door UNLOCKED...");
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(50);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(50);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(50);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(5000);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(100);
    digitalWrite(SUCCESS_PIN,LOW);
    digitalWrite(LOCK_PIN, LOW);
    Serial.println("Door LOCKED.");
    Serial.println();
    
  } else {
    
    Serial.print(" Access DENIED.\n");
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(100);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(50);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(100);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(50);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(200);
    digitalWrite(SUCCESS_PIN,LOW);
    Serial.println();
  }
}

//char receivedCharsSerial[numChars];
//boolean newDataFromSerial = false;
//char receivedCharsESP[numChars];
//boolean newDataFromESP = false;

//void serialRX(){
//  static boolean recieveInProgress = false;
//  static byte idx = 0;
//  char rc;
  
//  while(Serial.available() > 0 && newData = false){
//    rc = Serial.read();
//
//      receivedCharsSerial[idx] = rc;
//      idx++;
//      if(idx >= numChars){
//        idx = numChars -1;
//      }
//      receivedCharsSerial[idx] = '\0';
//      recieveInProgress = false;
//      idx = 0;
//      newDataSerial = true;
//    
//  }
//  if(serialESP.available()>0){
//      receivedCharESP = Serial.read();
//      newDataFromESP = true;
//      Serial.write(serialESP.read());
//  }
//}

//void newSerialData(){
//  if(newDataFromSerial){
//    serialESP.write(receivedCharSerial);
//    newDataFromSerial = false;
//  }
//  if(newDataFromESP){
//    Serial.write(receivedCharESP);
//    newDataFromESP = false;
//  }
//
//}
bool findUidInArray(String uid){
  Serial.print("UID: "+ uid + " detected, checking authority...");
  for(int i = 0; i < MAX_UIDS; i++){
    if(uids[i] == uid)
      return true;
  }
  return false;
}

void getWebSite(String address){
  
}


#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define TX_PIN 3
#define RX_PIN 2

#define SS_PIN 10
#define RST_PIN 9

#define SUCCESS_PIN 3
#define LOCK_PIN 8
#define BUTTON_PIN 6

const int MAX_UIDS = 10;
String uids[MAX_UIDS] = {"80 F9 8C 75","01 02 03 04","","","","","","","",""};


SoftwareSerial serialESP(RX_PIN, TX_PIN);
MFRC522 mfrc522(SS_PIN, RST_PIN);


bool high = false;
String pin5State = "LOW";


//Variables for reading serial data
const byte numChars = 32;
char receivedCharsSerial[numChars];
boolean newDataFromSerial = false;
char receivedCharsESP[numChars];
boolean newDataFromESP = false;
boolean newDataForESP = false;




void setup() {

  Serial.begin(115200);
  Serial.println("Restarting...");

  
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);


  pinMode(LOCK_PIN,OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LOCK_PIN, LOW);
  pinMode(SUCCESS_PIN, OUTPUT);
  digitalWrite(SUCCESS_PIN,LOW);
    
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("Recognised UIDs are...");
  for(int i = 0; i < MAX_UIDS; i++){
    Serial.print(i); Serial.print(": "); Serial.print(uids[i]); Serial.print("\n");
  }
  Serial.println("Swipe card...\n");

  Serial.println("Initializing Wifi Module...");
  serialESP.begin(9600);
  Serial.println("AT Commands Are");
  Serial.println("AT - Test AT startup");
  Serial.println("AT+RST - Restart module");
  Serial.println("AT+GMR - View version info");
  Serial.println("AT+GSLP - Enter deep-sleep mode");
  Serial.println("ATE - Enable / Disable echo");
  Serial.println("AT+CWMODE - WIFI mode（station, AP, station + AP）");
  Serial.println("AT+CWJAP - Connect to AP");
  Serial.println("AT+CWLAP - Lists available APs");
  Serial.println("AT+CIFSR - Get IP Address");
  Serial.println("AT+CIPAP=\"192.168.5.1\" - set ip address");
  Serial.println("AT+CIPSTATUS- Information about connection");
  Serial.println("AT+CIPSTART - Establish TCP connection or register UDP port and start a connection");
  Serial.println("AT+CIPSEND - Send data");
  Serial.println("AT+CIPCLOSE - Close TCP or UDP connection");
  Serial.println("AT+CIPMUX - Enable multiple connections or not");
  Serial.println("AT+CIPSERVER - Configure as server");
  Serial.println("AT+CIPMODE - Set transfer mode");
  Serial.println("AT+CIPSTO - Set server timeout");
  Serial.println("AT+CIUPDATE - update through network\n!!! Don’t run this unless you know what you’re doing !!!\n!!! It will likely brick your device !!!");
  Serial.println("+IPD - Receive network data");
  
  delay(2500);
//  serialESP.write("AT\n\r");
//  getWebSite();
//  
}


int h = 0;
void loop() {

     
  
  //Read serial from Serial Port
  serialRX();
  newSerialData();
  
  //Access button to open door from inside
  if(digitalRead(BUTTON_PIN)){
      accessGranted(true);    
   }
   
   Serial.println(h++);
//MFRC Reader Module
  if( ! mfrc522.PICC_IsNewCardPresent()){
    return;
  }
  if( ! mfrc522.PICC_ReadCardSerial()){
    return;
  }

  Serial.print("UID tag: ");
  String content = "";
  byte letter;
  for(byte i = 0; i < mfrc522.uid.size; i++){
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  String uid = content.substring(1);
  bool uidRecognised = findUidInArray(uid); 
  accessGranted(uidRecognised);


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

void serialRX(){
  static boolean recieveInProgress = false;
  static byte idx = 0;
  char rc;
  
  while(Serial.available() > 0 && newDataFromSerial == false){
      rc = Serial.read();
      receivedCharsSerial[idx] = rc;
      idx++;
      if(idx >= numChars -1 || rc == '\n'){
        receivedCharsSerial[idx] = '\0';
        newDataFromSerial = true;
        idx = 0;
        if((receivedCharsSerial[0] == 'A' && receivedCharsSerial[1]=='T')||receivedCharsSerial[0] == '+')
          newDataForESP = true;
        if(receivedCharsSerial[0] == 'g' && receivedCharsSerial[1] == 'e' && receivedCharsSerial[2] == 't'){
          getWebSite();
          newDataForESP = true;
        }
      }        
  }
  while(serialESP.available()>0 && newDataFromESP == false){
      rc = serialESP.read();
      receivedCharsESP[idx] = rc;
      idx++;
      
      if(idx >= numChars -1 || rc == '\n'){
        receivedCharsESP[idx] = '\0';
        newDataFromESP = true;
        idx = 0;
      }
  }
}

void newSerialData(){
  if(newDataFromSerial){
    if(newDataForESP){
      serialESP.write(receivedCharsSerial);
      newDataForESP = false;
    }else {
      Serial.write(receivedCharsSerial);
    }
    //serialESP.write(receivedCharsSerial);
    newDataFromSerial = false;
  }
  if(newDataFromESP){    
    Serial.write(receivedCharsESP);
    //Serial.write(receivedCharsESP);
    newDataFromESP = false;
  }

}
bool findUidInArray(String uid){
  Serial.print("UID: "+ uid + " detected, checking authority...");
  for(int i = 0; i < MAX_UIDS; i++){
    if(uids[i] == uid)
      return true;
  }
  return false;
}

void getWebSite(){
  serialESP.write("AT+CIPMUX=1\n\r");
  delay(500);
  String cmd = "GET / HTTP/1.1\r\n";
  int len = cmd.length() +4;
  serialESP.write("AT+CIPSTART=1,\"TCP\",\"www.google.co.uk\",80\n\r");
  delay(500);
  String s = "AT+CIPSEND=1,";
  s += len;
  s+="\n\r";
  serialESP.print(s);
  delay(500);
  serialESP.print(cmd);
  delay(500);
  serialESP.write("+IPD,1,len:data\n\r");
}


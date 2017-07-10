///libraries for RFID
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

//Defined Pins
  //0 RX
  //1 TX
#define SUCCESS_PIN 2  //2
#define SRX_PIN 3  //3 PWM
#define STX_PIN 4  //4
  //5 PWM
  //6 PWM
  //7
#define LOCK_PIN 8  //8
#define RST_PIN 9 //9 PWM
#define SS_PIN 10  //10 PWM
#define MOSI_PIN 11  //11 PWM
#define MISO_PIN 12  //12
#define SCK_PIN 13 //13
  //A0
  //A1
  //A2
  //A3
  //A4
  //A5


//variables for RFID
const int MAX_UIDS = 10;
String uids[MAX_UIDS] = {"80 F9 8C 75","01 02 03 04","2C F8 F8 A5","","","","","","",""};



SoftwareSerial ESP(SRX_PIN, STX_PIN);
const byte bufferSize = 64;
byte serialBuffer[bufferSize];
int serialIndex = 0;
byte espBuffer[bufferSize];
int espIndex = 0;
boolean newData = false;
String inputString = "";

//Instantiate MFRC module
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  ESP.begin(9600);

  //Initiate RFI
  SPI.begin();
  mfrc522.PCD_Init();
  //Print authorised cards to serial
  Serial.println("Recognised UIDs are...");
  for(int i = 0; i < MAX_UIDS; i++){
    Serial.print(i); Serial.print(": "); Serial.print(uids[i]); Serial.print("\n");
  }
  Serial.println("Swipe card...\n");
  delay(500);
  requestHouses();
}

void serialRX(){
  static boolean firstFill = true;
  static boolean writeToESP = false;
  static byte idx = 0;
  byte rb;
  while(Serial.available()>0 && !newData){
    rb = Serial.read(); 
    serialBuffer[idx] = rb;
    idx++;
    if(idx == bufferSize){
        inputString += (char*)serialBuffer;  
        idx = 0;
        memset(serialBuffer, 0, bufferSize);
    }
    if(rb == '\n'){
      inputString += (char*)serialBuffer;      
      idx = 0;
      memset(serialBuffer, 0, bufferSize);
      newData = true;
    }
    delay(10);
  }
}
void espRX(){
  while(ESP.available()>0){
    Serial.write(ESP.read());
    delay(10);
  }
}

void handleNewData(){
  String fetch = "fetch:";
  String esp = "esp>>";
  if(newData){
    if(inputString.indexOf(fetch) == 0){
      requestHouses();
    } else if(inputString.indexOf(esp)==0){
      String cmd = inputString.substring(esp.length(),inputString.length());
      ESP.print(cmd);
    } else {
      Serial.println(inputString);
    }
  } 
  newData = false;
  inputString = "";
}

boolean writeToBuffer(byte buff[], int *idx){

}

void loop() {
    serialRX();
    handleNewData();
    espRX();
  
  //Check if new card found, return if not
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

bool findUidInArray(String uid){
  Serial.print("UID: "+ uid + " detected, checking authority...");
  for(int i = 0; i < MAX_UIDS; i++){
    if(uids[i] == uid)
      return true;
  }
  return false;
}

void findKeyword(String){
  
}

void requestHouses(){
  String cmdCipsend = "AT+CIPSEND=1,42\r\n";
  String cmdGet = "GET / HTTP/1.1\r\nHost: lockserver.herokuapp.com\r\n\r\n\r\n";
  ESP.write("AT+CIPMUX=1\r\n");
  delay(500);
  ESP.write("AT+CIPSTART=1,\"TCP\",\"lockserver.herokuapp.com\",80\r\n");
  delay(500);
  ESP.write("AT+CIPSEND=1,58\r\n");
  delay(500);
  ESP.write("GET /houses HTTP/1.1\r\nHost: lockserver.herokuapp.com\r\n\r\n\r\n");
  delay(500);
  ESP.write("AT+CIPCLOSE=1\r\n");
}

void postToServer(){
  ESP.println("AT+CIPMUX=1");
  delay(500);
  ESP.write("AT+CIPSTART=1,\"TCP\",\"");
  delay(50);
  ESP.write("lockserver.herokuapp.com\",80");
  delay(500);
  ESP.println("AT+CIPSEND=1,42\r\n");
  delay(500);
  ESP.println("POST /iot/v1 HTTP/1.1\r\nHost: www.google.com");
  delay(500);
  ESP.println("AT+CIPCLOSE=1\r\n");

//  client.println("POST /tinyFittings/index.php HTTP/1.1");
//  client.println("Host:  artiswrong.com");
//  client.println("User-Agent: Arduino/1.0");
//  client.println("Connection: close");
//  client.println("Content-Type: application/x-www-form-urlencoded;");
//  client.print("Content-Length: ");
//  client.println(PostData.length());
//  client.println();
//  client.println(PostData);
}


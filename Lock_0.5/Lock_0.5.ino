///libraries for RFID
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
//#include <ArduinoJson.h>

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
const int MAX_UIDS = 15;
unsigned long luids[MAX_UIDS] = {0x80F98C75,0x01020304,0x2CF8F8A5,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

SoftwareSerial ESP(SRX_PIN, STX_PIN);
const byte bufferSize = 128;
byte serialBuffer[bufferSize];
int serialIndex = 0;
byte espBuffer[bufferSize];
int espIndex = 0;
boolean newData = false;
String inputString = "";

boolean receivingEspData = false;
const int commandArraySize = 4;
boolean receivingCommand[commandArraySize] = {false,false,false,false};
int commandIndex[commandArraySize] = {0,0,0,0};
const char commandArray[commandArraySize][10] = {"STARTLOCK", "LOCKEND","XXXX","XXXX"};

//Instantiate MFRC module
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  Serial.println("starting...");
  
  ESP.begin(4800);

  //Initiate RFI
  SPI.begin();
  mfrc522.PCD_Init();
  //Print authorised cards to serial
  Serial.println("Recognised UIDs are...");
  for(int i = 0; i < MAX_UIDS; i++){
    Serial.print(i); Serial.print(": "); Serial.print(luids[i], HEX); Serial.print("\n");
  }
  Serial.println("Swipe card...\n");
  Serial.print("Free RAM: "); Serial.print(freeRam()); Serial.print("B\n");
  delay(500);
  char *cmd = "H001";
  getDataForHouse(cmd);
}

void serialRX(){
  static boolean firstFill = true;
  static boolean writeToESP = false;
  static byte idx = 0;
  byte rb;
  while(Serial.available()>0){
    rb = Serial.read(); 
    serialBuffer[idx] = rb;
    idx++;
    if(idx == bufferSize-1){
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
  }
}
void espRX(){
  byte rb;
  String response = "";
  while(ESP.available()>0){
    rb = ESP.read();
    checkEspCommand(rb);
    espBuffer[espIndex] = rb;
    Serial.write(rb);
    ++espIndex;
    //response += rc;
  }

  espIndex = 0;
  char *p;
  p = espBuffer;
  char *tok;
   
  memset(espBuffer, 0, bufferSize); 
}

void checkEspCommand(char c){
    for(int i = 0; i < commandArraySize; i++){
      if(commandArray[i][commandIndex[i]] == c){
        ++commandIndex[i];
        if(commandIndex[i] == commandArraySize){
          Serial.println("^^^^^^^^^^^^^^");
          Serial.print("Received Command: ");
          Serial.println(commandArray[i]);
          Serial.println("^^^^^^^^^^^^^^");
          commandIndex[i] = 0;
        }
      } else {
        commandIndex[i] = 0;
      }
  }
}

void handleNewData(){
  String fetch = "fetch:";
  String esp = "esp>>";
  String house_id = "house_id:";
  if(newData){
    if(inputString.indexOf(fetch) == 0){
      requestHouses();
    } else if(inputString.indexOf(house_id)==0){
      String cmd = inputString.substring(house_id.length(),inputString.length());
      Serial.println(cmd);
      getDataForHouse(cmd);
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
  String content = "";
  byte letter;
  for(byte i = 0; i < mfrc522.uid.size; i++){
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  char b[10];
  for(int i = 0; i < content.length(); i++){
    b[i] = content[i];
  }
  unsigned long uid = strtoul(b,0,16);
  bool uidRecognised = findUidInArray(uid); 
  accessGranted(uidRecognised);
}

void accessGranted(bool granted){
  if(granted){
    Serial.print(" Access GRANTED. ");
    digitalWrite(LOCK_PIN, HIGH);
    Serial.print("Door UNLOCKED...");
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(50);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(50);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(50);
    digitalWrite(SUCCESS_PIN,LOW);
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    Serial.print(".");
    delay(1000);
    digitalWrite(SUCCESS_PIN,HIGH);
    delay(100);
    digitalWrite(SUCCESS_PIN,LOW);
    digitalWrite(LOCK_PIN, LOW);
    Serial.print(" Door LOCKED.\n\n");
  Serial.print("Free RAM: "); Serial.print(freeRam()); Serial.print("B\n");
    
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

bool findUidInArray(unsigned long uid){
  Serial.print("UID: "); Serial.print(uid);Serial.print(" / ");Serial.print(uid,HEX); Serial.print(" detected...");
  for(int i = 0; i < MAX_UIDS; i++){
    if(luids[i] == uid)
      return true;
  }
  return false;
}

void requestHouses(){
  ESP.write("AT+CIPMUX=1\r\n");
  delay(100);
  ESP.write("AT+CIPSTART=1,\"TCP\",\"lockserver.herokuapp.com\",80\r\n");
  delay(100);
  ESP.write("AT+CIPSEND=1,75\r\n");
  delay(10);
  ESP.write("GET /houses HTTP/1.1\r\nHost: lockserver.herokuapp.com\r\nConnection: close\r\n\r\n");;
}

void getDataForHouse(String house_id){
  String cipmux = "AT+CIPMUX=1\r\n";
  String cipstart = "AT+CIPSTART=1,\"TCP\",\"lockserver.herokuapp.com\",80\r\n";
  String content = "house_id="+house_id;
  String contentLength = "Content-Length: "; 
          contentLength += content.length(); 
          contentLength += "\r\n\r\n";
  String headers = "POST /iot/v1 HTTP/1.1\r\n";
          headers += "Host: lockserver.herokuapp.com\r\n";
          headers += "Connection: close\r\n";
          headers += "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
  String cipsend = "AT+CIPSEND=1,"; 
        cipsend+=(headers.length()+contentLength.length()+content.length()); 
        cipsend+="\r\n";

  ESP.print(cipmux);
  delay(200);
  ESP.print(cipstart);
  delay(200);
  ESP.print(cipsend);
  delay(200);
  ESP.print(headers);
  ESP.print(contentLength);
  ESP.print(content);
}


void removeChar(char *str, char ch){
  Serial.println(str);
  char *p, *i;
  p = str;
  i = str;
  while(*p){
    if(*p == ch){
      ++p;
    } else{
      *i = *p;
      ++i; 
      ++p;
    }
  }
  *i = '\0';
}

int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

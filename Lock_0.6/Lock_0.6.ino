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


#define VERSION_DATA_LENGTH 3
#define LOCK_DATA_LENGTH 4
#define KEYCARD_DATA_LENGTH 8
#define MAX_LOCKS 4
#define MAX_KEYCARDS 5

static byte rb = 0;
static const int cmdCount = 8;
static int cmdIndex[cmdCount] = {0,0}; 
static int currentCommand = -1;
static const int cmdLength = 16;
static const char commands[cmdCount][cmdLength] = {"LOCKSTART", "^", "VERSION", "VER?", "ARR?", "FETCH!", "KEYCARDSTART", "RAM?"}; 
static const int bSize = 16;
static byte dataBuffer[bSize] = {"\0"};
static int dataIndex = 0;
static boolean rcData = false;
static boolean rcKeyCardData = false;
static int lockNumber = -1;
static int lockIndex = 0;
static int keycardNumber = -1;
static int keycardIndex = 0;
static int versionNumber = 0;
unsigned long locks[MAX_LOCKS][MAX_KEYCARDS];
static String input = "";
const char *thisHouse = "H001";
//variables for RFID
//const int MAX_UIDS = 15;
//unsigned long luids[MAX_UIDS] = {0x80F98C75,0x01020304,0x2CF8F8A5,0x0,0x0,0x0,0x0,0x0,0x0,0x0};

SoftwareSerial ESP(SRX_PIN, STX_PIN);




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
//  Serial.println("Recognised UIDs are...");
//  for(int i = 0; i < MAX_LOCKS; i++){
//    for(int j = 0; j < MAX_KEYCARDS; j++){
//      Serial.print("Lock: "); Serial.print(i); Serial.print(" keycard: "); Serial.print(locks[i][j], HEX); Serial.print("\n");
//    }
//  }
  doCommand(4);
  Serial.println("Swipe card...\n");
  Serial.print("Free RAM: "); Serial.print(freeRam()); Serial.print("B\n");
  delay(500);
  getDataForHouse(thisHouse);
}

void readRX(byte b){
    input += (char)b;
    Serial.write(b);
    if(b == '\n'){       //end of line
      Serial.print("rcData: "); Serial.print(rcData==1); Serial.print('\n');
      clearDataBuffer();
    } else if (b == '\r'){
      //do nothing
    } else if (b == '^'){
      doCommand(currentCommand);
      rcData = false;
    } else if(rcData){
         registerData(b);
    } else {
          checkForCommand(b);  
    }
}

void checkForCommand(byte b){
  for(int i = 0; i < cmdCount; i++){
    if(commands[i][cmdIndex[i]] == b){
      ++cmdIndex[i];
      if(!commands[i][cmdIndex[i]]){     //Command received
        //doCommand(i);
        rcData = true;
        currentCommand = i;
        cmdIndex[i] = 0;
        clearDataBuffer();
      }
    } else {
      if(!rcData)
        cmdIndex[i] = 0;
    }
  }
}

void doCommand(int index){
  unsigned long kc;
  Serial.print("\nCommand Received: ");Serial.print(index);Serial.print(" - ");Serial.println(commands[index]);
  char *p;
  switch(index){
    case 0: //LOCKSTART
        
        p = (char *)dataBuffer;
        p++;
        Serial.println(p);
        lockNumber = atoi(p);
        keycardIndex = 0;
        clearDataBuffer();
      break;
    case 1: //^
        Serial.print("Data flushed from DataBuffer: "); Serial.print((char *)dataBuffer); Serial.print('\n');
        rcData = false;
        currentCommand = -1;
        clearDataBuffer();
      break;
    case 2: //VERSION
        rcData = true;
      break;
      case 3: //VER?
        Serial.print("Lock Version Number Is: ");Serial.println(versionNumber);
    break;
    case 4: //ARR?
        Serial.println("-----------------");
        Serial.println("** LOCK ARRAY");
        Serial.print("** VERSION: ");Serial.println(versionNumber);
        Serial.println("-----------------");
        for(int i = 0; i < MAX_LOCKS; i++){
          Serial.print("L");Serial.print(i+1);Serial.println("---------");
          for(int j = 0; j < MAX_KEYCARDS; j++){
            Serial.print("-KC");Serial.print(j);Serial.print(": ");;Serial.println(locks[i][j]);
          }
        }
        Serial.println("-----------------");
    break;
    case 5: //FETCH!
        Serial.println("-----------------");
        Serial.println("** Fetching data from lockserver...");
        Serial.println("-----------------");
        getDataForHouse(thisHouse);
    break;
    case 6: //KEYCARDSTART!p = (char *)dataBuffer;
        Serial.println((char *)dataBuffer);
        p = (char *)dataBuffer;
        kc = strtoul(p,0,16);
        Serial.print("Loaded Lock: ");Serial.print(lockNumber);Serial.print(" Keycard Index: ");Serial.print(keycardIndex);Serial.print(" ID: ");Serial.println(kc,HEX);
        locks[lockNumber-1][keycardIndex] = kc;
        ++keycardIndex;
        clearDataBuffer();
        break;
    case 7: //KEYCARDSTART!p = (char *)dataBuffer;
        freeRam();
        break;
    default:
        Serial.println("Command unknown");
      break;
  }       
}
//LOCKSTARTL001K80F98C75^
void registerData(byte b){
  dataBuffer[dataIndex] = b;
  ++dataIndex;
  if(dataIndex == bSize){
    //buffer full
    dataBuffer[dataIndex] = '\0';
    Serial.println("Buffer full");
    Serial.println((char *)dataBuffer);
    clearDataBuffer();
  }
}

void clearDataBuffer(){
    memset(dataBuffer, 0, bSize);
    dataIndex = 0;
}

void loop() {
    while(Serial.available())
      readRX(Serial.read());
    while(ESP.available())
      readRX(ESP.read());  

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
  for(int i = 0; i < MAX_LOCKS; i++){
    for(int j = 0; j < MAX_KEYCARDS; j++){
      if(locks[i][j] == uid)
      return true;
    }
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
  ESP.write("GET /houses HTTP/1.1\r\nHost: lockserver.herokuapp.com\r\nConnection: close\r\n\r\n");
}

void getDataForHouse(char house_id[4]){
  freeRam();
  char cipmux[] = "AT+CIPMUX=1\r\n";
  char cipstart[] = "AT+CIPSTART=1,\"TCP\",\"lockserver.herokuapp.com\",80\r\n";
  char content[] = "house_id=";
          strcat(content,house_id);
  char contentLength[] = "Content-Length: "; 
          strcat(contentLength, itoa(sizeof content / sizeof content[0])); 
          strcat(contentLength, "\r\n\r\n");
  char headers[] = "POST /iot/v1 HTTP/1.1\r\n";
          strcat(headers,"Host: lockserver.herokuapp.com\r\n");
          strcat(headers, "Connection: close\r\n");
          strcat(headers, "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n");
  char cipsend[] = "AT+CIPSEND=1,"; 
        int s = sizeof headers / sizeof headers[0] + sizeof contentLength / sizeof contentLength[0] + sizeof content / sizeof content[0];
        strcat(cipsend,itoa(s)); 
        strcat(cipsend,"\r\n");

  ESP.print(cipmux);
  delay(100);
  ESP.print(cipstart);
  delay(100);
  ESP.print(cipsend);
  delay(100);
  ESP.print(headers);
  ESP.print(contentLength);
  ESP.print(content);
  freeRam();
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
  int fr = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
  Serial.print("****************Free RAM: "); Serial.print(fr); Serial.print("B\n");
  return fr;
}

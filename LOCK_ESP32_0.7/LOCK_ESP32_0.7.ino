///libraries for RFID
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <PubSubClient.h>

//Defined Pins
  //0 RX
  //1 TX
#define SUCCESS_PIN 2  //2
#define SRX_PIN 3  //3 PWM
#define STX_PIN 4  //4
  //5 PWM
  //6 PWM
  //7
#define LOCK_PIN 17  //17
#define RST_PIN 22 //9 PWM
#define SS_PIN 5  //10 PWM

//#define MOSI_PIN 11  //11 PWM
//#define MISO_PIN 12  //12
//#define SCK_PIN 13 //13

#define MAX_LOCKS 4
#define MAX_KEYCARDS 5

//#define REQUEST_DELAY 60000

//Wifi Info
const char* ssid     = "EE-a27mcy";
const char* password = "pedal-send-red";
const char* host = "lockserver.herokuapp.com";
const char *mqttServer = "m20.cloudmqtt.com";
const int mqttPort = 19179;
const char *mqttUser = "egtzpbxa";
const char *mqttPassword = "nLm4RUAiD_yk";
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

static byte rb = 0;
static const int cmdCount = 9;
static int cmdIndex[cmdCount] = {0,0}; 
static int currentCommand = -1;
static const int cmdLength = 16;
static const char commands[cmdCount][cmdLength] = {"LOCKSTART", "^", "VERSION", "VER?", "ARR?", "FETCH!", "KEYCARDSTART", "RAM?", "UNLOCK"}; 
static const int bSize = 16;
static byte dataBuffer[bSize] = {"\0"};
static int dataIndex = 0;
static boolean rcData = false;
static boolean rcKeyCardData = false;
static int lockNumber = -1;
static int lockIndex = 0;
static int keycardNumber = -1;
static int keycardIndex = 0;
unsigned long locks[MAX_LOCKS][MAX_KEYCARDS];
static String input = "";
static char thisHouse[] = "H001";
static int versionNumber = 0;

unsigned long prevTime = 0;

void callback(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
//    Serial.print((char)payload[i]);
    readRX(payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}
void connectToMqtt(){
  while(!mqttClient.connected()){
    Serial.println("Connecting to MQTT...");
    if(mqttClient.connect("ESP32Client", mqttUser, mqttPassword)){
      Serial.println("Connected.");
      mqttClient.publish(thisHouse, versionNumber+"");
    } else {
      Serial.print("Failed with state ");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}
//Instantiate MFRC module
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  //Start serial coms
  Serial.begin(115200);
  Serial.println("starting...");
  
  //Set pins
  pinMode(SUCCESS_PIN, OUTPUT);
  pinMode(LOCK_PIN, OUTPUT);

  //Start Wifi Connection
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
   
  //connectToMqtt();
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
  connectToMqtt();
  mqttClient.subscribe("#");
  
  //After delay, Wifi should be connecte    
  //getDataForHouse(thisHouse);

  //Initiate RFI
  SPI.begin();
  mfrc522.PCD_Init();
  
  Serial.println("\n\nSwipe card...\n");
}

int requestCount = 0;
void loop() {
  
  mqttClient.loop();
  
//    if(millis() - prevTime >= REQUEST_DELAY){ //request new data every 60 seconds
//      Serial.print("Requested data "); Serial.print(requestCount); Serial.println(" times.");
//      getDataForHouse(thisHouse);
//      prevTime = millis();
//      requestCount++;
//    }
    while(Serial.available())
      readRX(Serial.read());
    while(wifiClient.available())
      readRX(wifiClient.read());  

  //Check if new card found, return if not
  checkRfid();
}

void checkRfid(){
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
      currentCommand = -1;
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
  unsigned long keycardID;
  Serial.print("\nCommand Received: ");Serial.print(index);Serial.print(" - ");
  if(index > -1)
    Serial.println(commands[index]);
  else 
    Serial.println("No command");
  char *p;
  switch(index){
    case -1: //NO COMMAND
        clearDataBuffer();
    break;
    case 0: //LOCKSTART
        p = (char *)dataBuffer;
        Serial.println(p);
        p++;
        lockNumber = atoi(p);
        keycardIndex = 0;
        clearDataBuffer();
    break;
    case 1: //^
//        Serial.print("Data flushed from DataBuffer: "); Serial.print((char *)dataBuffer); Serial.print('\n');
//        rcData = false;
//        currentCommand = -1;
//        clearDataBuffer();
      break;
    case 2: //VERSION
        p = (char *)dataBuffer;
        Serial.println(p);
        versionNumber = atoi(p);
        Serial.print("Lock Version Number Is: ");Serial.println(atoi(p));Serial.print("P: ");Serial.println(atoi(p));
        clearDataBuffer();
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
    case 6: //KEYCARDSTART!^
        p = (char *)dataBuffer;
        Serial.println(p);
//        keycardID = strtoul(p,0,16);
        keycardID = strtoul(p,0,16);
        locks[lockNumber][keycardIndex++] = keycardID;
        clearDataBuffer();
        break;
    case 7: //RAM?^
//        freeRam();
        break;
    case 8: //UNLOCK^
        Serial.print("Unlock House: ");Serial.print(stringToUL((char *)&dataBuffer[1], 3));
        Serial.print(" Lock: ");Serial.println(stringToUL((char *)&dataBuffer[5], 3));
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
//
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

void getDataForHouse(String house_id){
    Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    const int httpPort = 80;
    if (!wifiClient.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
    }

    // We now create a URI for the request
    String url = "/iot/v1";
    String content = "house_id="+house_id + "&version="+versionNumber;
    String request = "POST " + url + " HTTP/1.1\r\n";
          request += "Host: lockserver.herokuapp.com\r\n";
          request += "Connection: close\r\n";
          request += "Content-Type: application/x-www-form-urlencoded; charset=utf-8\r\n";
          request += "Content-Length: ";
          request += content.length();
          request+= "\r\n\r\n";
          request += content;
    Serial.print("Requesting: ");
    Serial.println(request);

    // This will send the request to the server
    wifiClient.print(request);
//    unsigned long timeout = millis();
//    while (client.available() == 0) {
//        if (millis() - timeout > 5000) {
//            Serial.println(">>> Client Timeout !");
//            client.stop();
//            return;
//        }
//    }
//
//    // Read all the lines of the reply from server and print them to Serial
//    while(client.available()) {
//        String line = client.readStringUntil('\r');
//        Serial.print(line);
//    }

    Serial.println();
    Serial.println("closing connection");
}

unsigned long stringToUL(char *src, int len){
    unsigned long result = 0;
    for(int i = 0; *src && isdigit(*src) && i < len ; src++, i++)
      result = 10 * result + *src - '0';      
    return result;
}

void receiveMqtt(char* topic, byte* payload, unsigned int length) {
 
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
 
  Serial.println();
  Serial.println("-----------------------");
 
}

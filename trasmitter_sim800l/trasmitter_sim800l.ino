#include <SoftwareSerial.h>
#include "sim800.h"
#define _SS_MAX_RX_BUFF 512
#define sstx 2
#define ssrx 4
#define button 3
#define sensor A7
SoftwareSerial mySerial(ssrx,sstx);
const int MAX_BUFF = 128;
char buff[MAX_BUFF];
int buf_idx = 0;
int value = 0;
char id[] = "8944303412715826776f";

void setup() {
  pinMode(button,INPUT); 
  pinMode(sensor,INPUT); 
  Serial.begin(9600);
  while(!Serial);
  
  mySerial.begin(9600);
  Serial.println("Initializing software serial port.");
  delay(1000);
  setupTCP();
}

void loop() {
  updateSerial();
  if(digitalRead(button) == 0){
    value = analogRead(sensor);
    sendData(value);
  }
}

void updateSerial(){
  while(Serial.available()){
    buff[buf_idx] = Serial.read();
    if(buff[buf_idx] == '\n'){
      for(int i = 0; i <= buf_idx; i++){
        mySerial.write(buff[i]);
      }
      updateSerial(500, "OK");
      buf_idx = 0;
    } else {
      buf_idx++;
    }
  }
  while(mySerial.available()){
    Serial.write(mySerial.read());
    delay(1);
  }
  delay(1);
}
bool updateSerial(long timeout, char pass[], char fail[]){
  long start = millis();
  if(timeout == 0){
    timeout = 30000;
  }
  while(millis() - start < timeout){
    while(mySerial.available()){
      buff[buf_idx] = mySerial.read();
      buf_idx++;
      if(buf_idx == MAX_BUFF){
        Serial.println("flushing buffer: ***");
        Serial.print(buff);
        Serial.print("\n");
        Serial.println("flushed buffer: ***");
        buf_idx = 0;
      }
      delay(1);
    }
    if(buf_idx > 0){
      buff[buf_idx] = '\0';
      char str2[buf_idx+1];
      strncpy(str2,buff, buf_idx+1);  
      Serial.print("SIM800L:\"");
      Serial.print(str2);
      Serial.println("\"");
      char * p;
      p = strstr(str2,pass);
      buf_idx = 0;
      if(p){
        Serial.println("Message OK");
        return true;
      } else {
        p = strstr(str2,fail);
        if(p){
          Serial.println("Message FAILED");
          return false;
        }
      }
    }
  }
  Serial.println("request timeout");
  buf_idx = 0;
  return false;
}
bool updateSerial(long timeout, char pass[]){
  char fail[] = "ERROR";
  long start = millis();
  if(timeout == 0){
    timeout = 30000;
  }
  while(millis() - start < timeout){
    while(mySerial.available()){
      buff[buf_idx] = mySerial.read();
      buf_idx++;
      if(buf_idx == MAX_BUFF){
        Serial.println("flushing buffer: ***");
        Serial.print(buff);
        Serial.print("\n");
        Serial.println("flushed buffer: ***");
        buf_idx = 0;
      }
      delay(1);
    }
    if(buf_idx > 0){
      buff[buf_idx] = '\0';
      char str2[buf_idx+1];
      strncpy(str2,buff, buf_idx+1);  
      Serial.print("SIM800L:\"");
      Serial.print(str2);
      Serial.println("\"");
      char * p;
      p = strstr(str2,pass);
      buf_idx = 0;
      if(p){
        Serial.println("Message OK");
        return true;
      } else {
//        p = strstr(str2,fail);
//        if(p){
//          Serial.println("Message FAILED");
//          return false;
//        }
      }
    }
  }
  Serial.println("request timeout");
  buf_idx = 0;
  return false;
}
void sendText(char str[]){
  //prep 
  mySerial.println("AT");
  updateSerial();
  mySerial.println("AT+CSQ");
  updateSerial();
  mySerial.println("AT+CCID");
  updateSerial();
  mySerial.println("AT+CREG=1");
  updateSerial();

  
  mySerial.println("AT+CMGF=1"); //configure text mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+447415858542\""); 
  
  int i = 0;
  char c = 'g';
  while((c=str[i++]) != '\0'){
    mySerial.print(c);
  }
  updateSerial();
  mySerial.write(26);
}
void setupTCP(){ 
  mySerial.println("AT");
  updateSerial();
}

void sendData(int val){
  Serial.print("sensor reading: ");
  Serial.println(val);
  mySerial.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  updateSerial(1000, "OK");
  mySerial.println("AT+SAPBR=3,1,\"APN\",\"everywhere\"");
  updateSerial(1000, "OK");
  mySerial.println("AT+SAPBR=1,1");
  updateSerial(1000, "OK");
  mySerial.println("AT+SAPBR=2,1");
  updateSerial(1000, "OK");
  
  mySerial.println("AT+HTTPINIT");
  updateSerial(1000, "OK");
  mySerial.println("AT+HTTPSSL=1");
  updateSerial(1000, "OK");
  mySerial.println("AT+HTTPPARA=\"CID\",1");
  updateSerial(1000, "OK");
  mySerial.println("AT+HTTPPARA=\"URL\",\"cloudcycle.co/api/v1/sensor\"");
  updateSerial(1000, "OK");
  mySerial.println("AT+HTTPPARA=\"Content\",\"text/plain\"");
  updateSerial(1000, "OK");
  int len = 3+(sizeof(id)/sizeof(id[0]))+1+4+1+(val<10 ? 1 : val < 100 ? 2 : val < 1000 ? 3 : 4)+1;
  mySerial.print("AT+HTTPDATA=");
  mySerial.print(len);
  mySerial.println(",10000");
  updateSerial(1000, "DOWNLOAD");
  mySerial.print("id=");
  mySerial.print(id);
  mySerial.print("&photo=");
  mySerial.println(val);
  updateSerial(1000, "OK");
  mySerial.println("AT+HTTPACTION=1");
  updateSerial(10000, "+HTTPACTION: ");
  mySerial.println("AT+HTTPREAD");
  updateSerial(5000, "API OK");
  mySerial.println("AT+HTTPTERM");
  updateSerial(1000, "OK");
  mySerial.println("AT+SAPBR=0,1");
  updateSerial(1000, "OK");
  Serial.print("data sent.");
}

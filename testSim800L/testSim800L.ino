#include <Sim800lHttp.h>
#include <SoftwareSerial.h>
#include <MemoryFree.h>

#define _SS_MAX_RX_BUFF 256

#define pinReset 5
#define sstx 17
#define ssrx 16
#define pinPostButton 12

SoftwareSerial mySerial(ssrx,sstx);
Sim800lHttp *sim;
bool postInProgress = false;
//char url[] = "cloudcycle.co/api/v1/sensor";
//char data[] = "_ABCDE_";
//char content[] = "text/plain";
void setup() {
  Serial.begin(9600);
  while(!Serial);
  pinMode(pinReset,OUTPUT);
  digitalWrite(pinReset,HIGH);
  pinMode(pinPostButton,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinPostButton),post, FALLING);
  mySerial.begin(9600);
  sim = new Sim800lHttp(10, Serial, mySerial);
}

void loop() {
  sim->update();
  bool *s = sim->status();
  if(postInProgress && !*s){
    postInProgress = false;
  }

}
void post(){
  if(!postInProgress){
    postInProgress = true;
    Serial.println("Posting");
    sim->post("cloudcycle.co/api/v1/sensor", "_ABCDE_");
  }
}

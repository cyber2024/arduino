const int PHOTORESIST_IN = A7;
const int R = 2;
const int G = 3;
const int B = 4;

int sensorValue = 0;
int outputValue = 0;
int currentPin = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(R,OUTPUT);
  pinMode(G,OUTPUT);
  pinMode(B,OUTPUT);
  Serial.begin(9600);
  Serial.print("Ready...\n");
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorValue = analogRead(PHOTORESIST_IN);
  outputValue = normaliseSensorValue(sensorValue);
  Serial.print(sensorValue);
  Serial.print("\t=>\t");
  Serial.print(outputValue);
  Serial.print("\n");
  digitalWrite(R,LOW);
  digitalWrite(G,LOW);
  digitalWrite(B,LOW);
  if(outputValue < 64){
    
  } else if(outputValue < 128){
    digitalWrite(R,HIGH);
  } else if(outputValue < 192){
    digitalWrite(R,HIGH);
    digitalWrite(G,HIGH);
    
  } else {
    digitalWrite(R,HIGH);
    digitalWrite(G,HIGH);
    digitalWrite(B,HIGH);
  } 
}

int normaliseSensorValue(int val){
  return map(val,0,1023,0,255);
}

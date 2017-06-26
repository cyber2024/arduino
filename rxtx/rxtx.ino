#define rfTransmitPin 4
#define txLedPin 13
#define rfReceivePin A0
#define rxLedPin 7

unsigned int data = 0;
const unsigned int upperThreshold = 674;
const unsigned int lowerThreshold = 674;

void setup() {
  pinMode(rfTransmitPin, OUTPUT);
  pinMode(txLedPin,OUTPUT);
  pinMode(rxLedPin, OUTPUT);
  Serial.begin(9600);

}

int count = 0;
unsigned long d = 1000, e = 2000, elapsed = 0, startTime = 0;
int i = 1;
unsigned long timer = 0;
void loop() {
  if(startTime == 0)
    startTime = millis();
    
    elapsed = millis() - startTime;
    count += i;
    if( elapsed > d){
      digitalWrite(rfTransmitPin, HIGH);
      digitalWrite(txLedPin, HIGH);
    }

    if(elapsed > e){
      digitalWrite(rfTransmitPin, LOW);
      digitalWrite(txLedPin, LOW);
      startTime = millis();
    }
    
    if(count >5 || count < 1)
      i *= -1;

    data = analogRead(rfReceivePin);
    if(data > upperThreshold){
      digitalWrite(rxLedPin, HIGH);
      Serial.println("HIGH");
    } else if ( data < lowerThreshold){
      digitalWrite(rxLedPin, LOW);
      Serial.println("LOW");
    }
//    Serial.println(data);

}

void setup() {
  for(int i = 1; i < 14; i++){
    pinMode(i,OUTPUT);  
  }
  pinMode(0, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  Serial.begin(9600);
}

void loop() {
  for(int i = 1; i < 14; i++){
    digitalWrite(i,HIGH);
  }
  analogWrite(5,255);
  analogWrite(A0, 400);
  analogWrite(A1, 300);
  analogWrite(A2, 200);
  analogWrite(A3, 100);
  Serial.println(analogRead(A4));
 // Serial.println(analogRead(A5));
  //Serial.println(analogRead(A6));
  //Serial.println(analogRead(A7));
  delay(2000);
  for(int i = 1; i < 14; i++){
    digitalWrite(i,LOW);
  }
  
  analogWrite(5,56);
  analogWrite(A0, 1024);
  analogWrite(A1, 1024);
  analogWrite(A2, 1024);
  analogWrite(A3, 1024);
  Serial.println(analogRead(A4));
  //Serial.println(analogRead(A5));
  //Serial.println(analogRead(A6));
  //Serial.println(analogRead(A7));
  delay(2000);
}

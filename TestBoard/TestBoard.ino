void setup() {
  //Serial.begin(115200);
  Serial.println("starting...");
  pinMode(13, INPUT);
  for(int i = 0; i < 13; i++){
    pinMode(i,OUTPUT);
    digitalWrite(i,HIGH);
    delay(10);
    Serial.print(String(i)+": HIGH");
    Serial.print("-");
    Serial.print(digitalRead(13));
    
    Serial.print(" \n");
    digitalWrite(i,LOW);
    delay(10);
    Serial.print(String(i)+": LOW");
    Serial.print("-");
    Serial.print(digitalRead(13));
    
    Serial.print(" \n");
  }
 }
boolean firstRound = true;
void loop() {

    for(int i = 0; i < 14; i++){
    Serial.println("------"+String(i)+"---------");
      int j = 1;
      int dir = 1; 
      while(j > 0){

        analogWrite(i,j);
        delay(2); 
        j += 1* dir;
        if(j >= 255)
          dir = -1;
      }
      digitalWrite(i,LOW);
    }

  firstRound = false;
}

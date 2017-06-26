String inString = "";
boolean stringComplete = false;

void setup(){
  Serial.begin(9600);
  inString.reserve(200);
}

void loop() {
  if(stringComplete){
    Serial.println(inString);
    inString = "";
    stringComplete = false;
  }
}

void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    if(inChar = '\n'){
      stringComplete = true;
    } else {
      inString += inChar; 
    }
  }
}


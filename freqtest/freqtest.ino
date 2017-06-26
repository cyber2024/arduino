void setup() {
  Serial.begin(115200);
  Serial.println(":startig");

}
unsigned long prevTime = millis();
int hz = 0;
void loop() {
    unsigned long prevTime = millis();
  Serial.println("starting");
  for(int i = 0; i < 1000000; i++){
    unsigned long int r = 555;
    float f = 0.6;
    float j = r/f;
    if(i % 100000 == 0){
      unsigned long dur = millis() - prevTime;
      Serial.println("10000 took so far");
      Serial.println(dur/1000);
    }
    
  }
  Serial.println("calcs took");
  unsigned long dur = millis() - prevTime;
  Serial.println(dur/1000);
  hz++;
  Serial.println(hz);
//  for(int i = 0; i < 1; i++){
//    unsigned long int r = 555;
//    float f = 0.6;
//    float j = r/f;
//  }
}

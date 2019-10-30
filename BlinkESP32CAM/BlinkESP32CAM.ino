
#define TX 26
#define RX 27
#define RESET 33
#define ONBOARD_SIM800L_POW 4

void setup() {
  pinMode(ONBOARD_SIM800L_POW, OUTPUT);
  digitalWrite(ONBOARD_SIM800L_POW, LOW);
  pinMode(RESET, OUTPUT);
  digitalWrite(RESET, LOW);
  Serial.begin(115200);
  Serial.println("\nSoftware serial test started");
  
}


// the loop function runs over and over again forever
void loop() {
  while (Serial.available() > 0) {
    char c = Serial.read();
    Serial.print(c);
  }
}

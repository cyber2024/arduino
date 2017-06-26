void setup() {
  // initialize both serial ports:
  Serial.begin(115200);
  Serial3.begin(9600); //needs to match baud of chicp
  
  delay(50);
  Serial.println(Serial3.write("AT+RST\r\n"));
  Serial.println("AT Commands Are");
  Serial.println("AT - Test AT startup");
  Serial.println("AT+RST - Restart module");
  Serial.println("AT+GMR - View version info");
  Serial.println("AT+GSLP - Enter deep-sleep mode");
  Serial.println("ATE - Enable / Disable echo");
  Serial.println("AT+CWMODE - WIFI mode（station, AP, station + AP）");
  Serial.println("AT+CWJAP - Connect to AP");
  Serial.println("AT+CWLAP - Lists available APs");
  Serial.println("AT+CIFSR - Get IP Address");

}
String command = "";
void loop() {
  // read from port 1, send to port 0:
  while(Serial3.available()) {
    char inByte = Serial3.read();
    Serial.write(inByte);
    if(inByte != '\0' || inByte != '\n'){
      command += inByte;
    } else {
      Serial.print("new command: ");
      Serial.print(command);
      Serial.print("\r\n");
      command = "";
    }
  }

  // read from port 0, send to port 1:
  while (Serial.available()) {
    int inByte = Serial.read();
    Serial3.write(inByte);
  }
}

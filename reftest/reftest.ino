char buff[32] = "This is my buff";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Started.");
  Serial.print("&buff=");
  Serial.print(&buff, HEX);
  Serial.print("\nbuff=");
  Serial.println(buff);
  Serial.print("*buff=");
  Serial.println(*buff);
}

void loop() {
  // put your main code here, to run repeatedly:

}

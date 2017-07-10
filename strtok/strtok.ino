void setup() {
  Serial.begin(115200);
  byte buf[] = ",this,is,some,random,str";
  char *p;
  p = buf;
  Serial.print("First occurence is: ");
  char *q = strpbrk(buf,"r");
  Serial.println(strpbrk(buf,"this") - (char *)buf);
  Serial.println((char *)buf);
  strtok(p, ",");
  Serial.println(p);
  while((p = strtok(NULL,",")) != NULL){
    Serial.println(p);
  }

}


void loop() {
  // put your main code here, to run repeatedly:

}

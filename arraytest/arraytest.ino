
char str[32];
byte arr[32];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial);
  Serial.print(sizeof(str));
  Serial.print("\n");
  Serial.print(sizeof(arr));
  Serial.print("\n");
  Serial.print("str: ");
  Serial.println(str);
  Serial.print("arr: ");
  for(int i = 0; i < sizeof(arr); i++){
    arr[i] = i;
    for(int j = 0; j < 8; j++){
      Serial.print(bitRead(arr[i],7-j));
    }
      Serial.print(" ");
  }
  Serial.print("\n");
  for(int i = 0; i < sizeof(arr); i++){
    arr[i] = 97+i;
    char ch[2];
    sprintf(ch,"%02X",arr[i]);
    Serial.print(ch);
    Serial.print(" ");
  }
  Serial.print("\n");
}

void loop() {
  // put your main code here, to run repeatedly:

} 

void printBin(uint8_t num){
  char binChar[8];
  sprintf(binChar,"%08B",num);
  Serial.print(binChar);
}

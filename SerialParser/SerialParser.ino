const int bSize = 8;        //max buffer size
byte buff[bSize];             //serial buffer
int index = 0;                //serial buffer index
byte rb;                      //received byte from serial
const char commandStartChar = '{';
const char commandEndChar = '}';
boolean readCommand = false;//read to command buffer

const int maxLocks = 10;
const int maxKeycards = 10;
const int maxDigits = 4;
char lock[maxLocks][maxKeycards][maxDigits];



void setup() {
  Serial.begin(4800);
  Serial.println("Starting...");
  char buffer[16] = "80F98C75";
  Serial.println(buffer);
  unsigned long i = 0x80F98C75;
  ltoa(i, buffer, 16);
  Serial.write(buffer);
  Serial.write('\n');
  Serial.println(sizeof(i));

  strcpy(lock[0][0],"hgey");
//
//  char locks[maxLocks][maxKeycards][maxDigits];
//  for(int i = 0; i < maxLocks; i++){
//    for(int j = 0; j < maxKeycards; j++){
//     strcpy(locks[i][j][0],"1234");
//    }
//  }
//  for(int i = 0; i < maxLocks; i++){
//    for(int j = 0; j < maxKeycards; j++)
//      Serial.println(locks[i][j][0]);
//  }

Serial.print("Free RAM: "); Serial.print(freeRam()); Serial.print("B\n");
}

void loop() {
  rx();

}

void rx(){
  while(Serial.available()){
    rb = Serial.read();
    buff[index] = rb;
    index++;
    
    if(readCommand && rb == commandEndChar){
      buff[index-1] = '\0';
      parse(buff);
      readCommand = false;
      clearBuffer(buff, 0, bSize, &index);
    } 

    if(index == bSize || rb == '\n'){
      buff[index] = '\0';
      Serial.println("Buffer full, flushing.");
      parse(buff);
      clearBuffer(buff, 0, bSize, &index);
    }
    if(rb == commandStartChar) {
      readCommand = true;
      clearBuffer(buff, 0, bSize, &index);
    }
  }
}

void parse(byte *b){
  byte *p;
  p = b;
  if(readCommand) Serial.println("Command detected."); 

  Serial.println((char *)b);
}

void clearBuffer(byte *buffer, char character, const int bufferSize, int *bufferIndex){
  memset(buffer,character,bufferSize);
  *bufferIndex = 0;
}


int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

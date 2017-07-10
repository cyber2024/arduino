#define VERSION_DATA_LENGTH 3
#define LOCK_DATA_LENGTH 4
#define KEYCARD_DATA_LENGTH 8
#define MAX_LOCKS 2
#define MAX_KEYCARDS 10

static byte rb = 0;
static const int cmdCount = 5;
static int cmdIndex[cmdCount] = {0,0}; 
static const int cmdLength = 12;
static const char commands[cmdCount][cmdLength] = {"LOCKSTART", "^", "VERSION", "VER?", "ARR?"}; 
static const int bSize = 16;
static byte dataBuffer[bSize] = {"\0"};
static int dataIndex = 0;
static boolean rcData = false;
static boolean rcKeyCardData = false;
static int lockNumber = -1;
static int lockIndex = 0;
static int keycardNumber = -1;
static int keycardIndex = 0;
static int versionNumber = 0;


unsigned long locks[MAX_LOCKS][MAX_KEYCARDS];

void setup() {
  Serial.begin(115200);
}

void loop() {
  rx();
}

void rx(){
  if(Serial.available()){
    rb = Serial.read();
    if(rb == '\n'){       //end of line
      Serial.print("rcData: "); Serial.print(rcData==1); Serial.print('\n');
    } else if (rb == '\r'){
      //do nothing
    } else if (rb == '^'){
      doCommand(1);
    } else if(rcData){
         registerData(rb);
    } else {
          checkForCommand();  
    }
  }
}

void checkForCommand(){
  for(int i = 0; i < cmdCount; i++){
    if(commands[i][cmdIndex[i]] == rb){
      ++cmdIndex[i];
      if(!commands[i][cmdIndex[i]]){     //Command received
        doCommand(i);
        cmdIndex[i] = 0;
      }
    } else {
        cmdIndex[i] = 0;
    }
  }
}

void doCommand(int index){
  Serial.println("\nCommand Received: ");
  switch(index){
    case 0: //LOCKSTART
        Serial.println(commands[index]);
        rcData = true;
      break;
    case 1: //LOCKEND
        Serial.println(commands[index]);
        Serial.print("DataBuffer: "); Serial.print((char *)dataBuffer); Serial.print('\n');
        rcData = false;
        clearDataBuffer();
      break;
    case 2: //VERSION
        Serial.println(commands[index]);
        rcData = true;
      break;
      case 3: //VER?
        Serial.print("Lock Version Number Is: ");Serial.println(versionNumber);
      break;
      case 4: //ARR?
        Serial.println("-----------------");
        Serial.println("** LOCK ARRAY");
        Serial.print("** VERSION: ");Serial.println(versionNumber);
        Serial.println("-----------------");
        for(int i = 0; i < MAX_LOCKS; i++){
          Serial.print("L");Serial.print(i+1);Serial.println("---------");
          for(int j = 0; j < MAX_KEYCARDS; j++){
            Serial.print("-KC");Serial.print(j);Serial.print(": ");;Serial.println(locks[i][j]);
          }
        }
        Serial.println("-----------------");
      break;
    default:
        Serial.println("Command unknown");
      break;
  }       
}
//LOCKSTARTL001K80F98C75^
void registerData(byte b){
  dataBuffer[dataIndex] = b;
  ++dataIndex;
  //dataBuffer[dataIndex]='\0';
  char *p;
  if(dataIndex == (LOCK_DATA_LENGTH)  && dataBuffer[0] == 'L'){
    Serial.print("Received lock number for: "); Serial.print((char *)dataBuffer); Serial.print('\n');
     p = dataBuffer;
    p++;
    lockNumber = atoi(p);
    keycardIndex = 0;
    clearDataBuffer();
  } else if (dataIndex == (KEYCARD_DATA_LENGTH+1)  && dataBuffer[0] == 'K'){
    if(lockNumber != -1 && lockNumber < MAX_LOCKS && keycardIndex < MAX_KEYCARDS){
      p = dataBuffer;
      p++;
      unsigned long kc = strtoul(p,0,16);
      locks[lockNumber-1][keycardIndex] = kc;
      ++keycardIndex;
      Serial.print("Keycard recodnised: ");Serial.println(kc, HEX);
    }
    clearDataBuffer();
  }
  if(dataIndex == bSize){
    //buffer full
    dataBuffer[dataIndex] = '\0';
    Serial.println("Buffer full");
    Serial.println((char *)dataBuffer);
    clearDataBuffer();
  }
}

void clearDataBuffer(){
    memset(dataBuffer, 0, bSize);
    dataIndex = 0;
}


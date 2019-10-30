#include <Arduino.h>
#include "Sim800lHttp.h"
#include <HardwareSerial.h>

char * ERROR_CODE = "ERROR";
char * OK_CODE= "OK";
char * URL = "AT+HTTPPARA=\"URL\",\"";
char * INIT_GPRS = "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\n";
char * SET_APN = "AT+SAPBR=3,1,\"APN\",\"everywhere\"\n";
char * SET_BEARER = "AT+HTTPPARA=\"CID\",1\n";
// char  * SET_CONTENT = "AT+HTTPPARA=\"Content\",\"text/plain\"\n";
char  * SET_CONTENT = "AT+HTTPPARA=\"Content\",\"application/x-www-form-urlencoded\"\n";


Sim800lHttp::Sim800lHttp(int resetPin, HardwareSerial &hserial, HardwareSerial &sserial){
    _resetPin = resetPin;
    _hSerial = &hserial;
    _sSerial = &sserial;
    _hSerial->println(F("Initialized Sim800L"));
    _buff_idx = 0;
    pinMode(resetPin, OUTPUT);
    memset(_buff, 0, MAX_BUFF);
    this->setState(-20);
    this->awaitingResponse = false;
    this->posting = false;
    this->complete = false;
    this->failed = false;
    this->timer = 0;
    strcpy(this->data, ""); 
    // this->dataLength = "0";
    this->timeoutState = 0;
    
}

bool* Sim800lHttp::status(){
    return &this->posting;
};

void Sim800lHttp::update(){
    updateComms();
    updateState();
}
char* Sim800lHttp::post(char url[], char data[]){
    // strcpy(this->URL, url);
    // strcpy(this->data, data);   
    // sprintf(this->dataLength, "%d", strlen(data));
    strcpy(this->url, url);
    strcpy(this->data, strlen(data) == 0 ? "NO DATA" : data);
    // _hSerial->print("Data: ");_hSerial->println(this->data);
    // _hSerial->print("Data: ");_hSerial->println(data);
    this->setState(10);
    this->posting = true;
}
void Sim800lHttp::getline(HardwareSerial &stream){
    char c;
    while(stream.available()){
        c = stream.read();
        writeToBuffer(c);
        if(_buff[_buff_idx] == '\n'){
            return;
        }
        delay(1);
    }
} 
void Sim800lHttp::updateState(){
    if(timer > 0 && millis() > timer){
        Serial.println("TIMEOUT");
        setState(this->timeoutState);
        timer = 0;
    }
    switch(this->state){
        case 0: //do nothing
        break;
        case 10: 
            this->initGPRS();
        break;
        case 20: 
            this->setAPN();
        break;
        case 30: 
            this->openGPRS();
        break;
        case 40: 
            this->checkGPRS();
        break;
        case 50: 
            this->initHTTP();
        break;
        case 60: 
            this->setSSL();
        break;
        case 70: 
            this->setHTTPBearer();
        break;
        case 80: 
            this->setURL();
        break;
        case 90: //set content
            this->setContent();
        break;
        case 100: //set metadata
            this->setMetaData();
        break;
        case 110: //set data
            this->setData();
        break;
        case 120: //read action
            this->setAction();
        break;
        case 130: //read action
            this->readHTTP();
        break;
        case -10: //closing GPRS
            this->closeGPRS();
        break;
        case -20: 
        // _hSerial->print(F("trying to terminate..."));
            this->terminateHTTP();
        break;
        default: //do nothing if state isnt here
        // _hSerial->print(F("Did not recognise state: "));
        // _hSerial->println(this->state);
        break;
    }
}
void Sim800lHttp::updateComms(){
    if(_sSerial->available()){
        while(_sSerial->available()){
            getline(*_sSerial);
            _hSerial->print("SIM: ");
            printBuffer();
            _hSerial->println("");
            processResponse();
        }
        flushBuffer();
    }
    if(_hSerial->available()){
        while(_hSerial->available()){
            getline(*_hSerial);
            printBuffer();
            _hSerial->println("");
            writeBufferToModem();
        }
        flushBuffer();
    }
}
bool Sim800lHttp::processResponse(){
    /*
        Determine what to do with a recognised reponse from _sSerial
    */
    switch(this->state){
        case 0:
            //if unit is in ground state, it is not expecting a response, do nothing;
            _hSerial->print(F("**unexpected** SIM: "));
            _hSerial->println(_buff);
        break;
        case 11:
        case 21:
        case 31:
        case 41:
        case 51:
        case 61:
        case 71:
        case 81:
        case 91:
            checkResponseForStrings(OK_CODE, ERROR_CODE);
        break;
        case 101:
            checkResponseForStrings("DOWNLOAD", ERROR_CODE);
        break;
        case 111:
            checkResponseForStrings(OK_CODE, ERROR_CODE);
        break;
        case 121:
            checkResponseForStrings("+HTTPACTION: ", ERROR_CODE);
        break;
        case 131:
            checkResponseForStrings(OK_CODE, -20, ERROR_CODE, -20);
        break;
        case -11:
            checkResponseForStrings(OK_CODE, 0, ERROR_CODE, 0);
            posting = false;
        break;
        case -21:
            checkResponseForStrings(OK_CODE, -10, ERROR_CODE, -10);
            
        break;
        default:
        break;
        this->_hSerial->print(F("State not recognised: "));
        this->_hSerial->println(this->state);
    }
}
void Sim800lHttp::checkResponseForStrings(char * passString, int passStatusCode, char * failString, int failStatusCode){
    int fail = findPatternInString(_buff, _buff_idx+1, failString, strlen(failString));
    if(fail >= 0){
        setState(failStatusCode);
        resetTimer();
        return;
    }
    int pass = findPatternInString(_buff, _buff_idx+1, passString, strlen(passString)); 
    if(pass >= 0){
        setState(passStatusCode);
        resetTimer();
        return;
    }
}
void Sim800lHttp::checkResponseForStrings(char * passString, char * failString){
    int fail = findPatternInString(_buff, _buff_idx+1, failString, strlen(failString));
    if(fail >= 0){
        setState(-20);
        resetTimer();
        return;
    }
    int pass = findPatternInString(_buff, _buff_idx+1, passString, strlen(passString)); 
    if(pass >= 0){
        setState(this->state+10 - this->state%10);
        resetTimer();
        return;
    }
}

int Sim800lHttp::findPatternInString(char * string, int stringLen, char * code, int codeLen){
    int j = 0;
    for(int i = 0; i < stringLen; i++){
        if(string[i] == code[j]){
            if(j == codeLen-1)
                return i-(codeLen-1);
            j++;
        } else {
            j = 0;
        }
        // if(i + codeLen > stringLen+1)
        //     return -1;
    }
    return -1;
}
void Sim800lHttp::setState(int newState){
    _hSerial->print(F("State change: "));
    _hSerial->print(this->state);
    _hSerial->print(F("->"));
    _hSerial->println(newState);
    state = newState;
    //always set posting to false if state is 0;
    if(state == 0)
        posting = false;
}
void Sim800lHttp::initGPRS(){
    _hSerial->println(F("Initializing GPRS..."));
    writeToBuffer(INIT_GPRS, strlen(INIT_GPRS));
    writeBufferToModem();
    setState(11);
    setTimer(1000, -20);
}
void Sim800lHttp::closeGPRS(){
    _hSerial->println(F("Closing GPRS..."));
    char str[] = "AT+SAPBR=0,1\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(-11);
    setTimer(1000, 0);
}
void Sim800lHttp::setAPN(){
    _hSerial->println(F("Setting APN..."));
    writeToBuffer(SET_APN, strlen(SET_APN));
    writeBufferToModem();
    setState(21);
    setTimer(1000, -20);
}
void Sim800lHttp::openGPRS(){
    _hSerial->println(F("Opening GPRS..."));
    char str[] = "AT+SAPBR=1,1\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(31);
    setTimer(1000, -20);
}
void Sim800lHttp::checkGPRS(){
     _hSerial->println(F("Checking GPRS..."));
    char str[] = "AT+SAPBR=2,1\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(41);
    setTimer(1000, -20);
}
void Sim800lHttp::initHTTP(){
    _hSerial->println(F("Initializing Http..."));
    char str[] = "AT+HTTPINIT\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(51);
    setTimer(1000, -20);
}
void Sim800lHttp::terminateHTTP(){
    _hSerial->println(F("Terminating HTTP..."));
    char str[] = "AT+HTTPTERM\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(-21);
    setTimer(1000, 0);
}
void Sim800lHttp::setSSL(){
    _hSerial->println(F("Setting SSL..."));
    char str[] = "AT+HTTPSSL=1\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(61);
    setTimer(1000, -20);
}
void Sim800lHttp::setHTTPBearer(){
    // _hSerial->println(F("Setting HTTPBearer..."));
    writeToBuffer(SET_BEARER, strlen(SET_BEARER));
    writeBufferToModem();
    setState(71);
    setTimer(1000, -20);
}
void Sim800lHttp::setURL(){
    _hSerial->println(F("Setting url..."));
    
    writeToBuffer(URL, strlen(URL));
    writeToBuffer(this->url, strlen(this->url));
    writeToBuffer("\"\n", strlen(this->url));
    writeBufferToModem();
    setState(81);
    setTimer(1000, -20);
}
void Sim800lHttp::setContent(){
    _hSerial->println(F("Setting content..."));
    writeToBuffer(SET_CONTENT, strlen(SET_CONTENT));
    writeBufferToModem();
    setState(91);
    setTimer(2000, -20);
}
void Sim800lHttp::setMetaData(){
    _hSerial->println(F("Setting meta data..."));
    char str[256];
    sprintf(str,"AT+HTTPDATA=%d,%d\n",strlen(this->data),3000);
    // strcat(str,"3000\n");
    //"5,5000\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(101);
    setTimer(1000, -20);
}
void Sim800lHttp::setData(){
    _hSerial->println(F("Setting data..."));
    char str[256];
    sprintf(str, "%s",this->data);
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(111);
    setTimer(1000, -20);
}
void Sim800lHttp::setAction(){
    _hSerial->println(F("Setting action..."));
    char str[] = "AT+HTTPACTION=1\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(121);
    setTimer(5000, -20);
}
void Sim800lHttp::readHTTP(){
    _hSerial->println(F("Reading HTTP response..."));
    char str[] = "AT+HTTPREAD\n";
    writeToBuffer(str, strlen(str));
    writeBufferToModem();
    setState(131);
    setTimer(10000, -20);
}
void Sim800lHttp::writeToBuffer(char c){
    if(_buff_idx > MAX_BUFF-1){
        dumpBuffer();
        setState(-20);
        return;
    }
    _buff[_buff_idx] = c;
    _buff_idx++;
    return;
}
void Sim800lHttp::writeToBuffer(char *string, int len){
    int i = 0;
    if(len + _buff_idx > MAX_BUFF - 1 ){
        dumpBuffer();
        setState(-20);
        return;
    }
    while(_buff_idx < MAX_BUFF && i < len){
        _buff[_buff_idx] = string[i];
        _buff_idx++;
        i++;
    }
    return;
}
void Sim800lHttp::writeBufferToModem(){
    for(int i = 0; i < MAX_BUFF; i++){
        _sSerial->print(_buff[i]);
        if(_buff[i] == '\0'){
            break;
        }
    }
    flushBuffer();
    return;
}
void Sim800lHttp::printBuffer(){
    int i = 0;
    char addr[20];
    _hSerial->print(addr);
    while(i < MAX_BUFF){
        if(_buff[i] == '\n'){
            _hSerial->print("\\n");
        } else if(_buff[i] == '\0'){
            _hSerial->print(" ");
        } else {
            _hSerial->print(_buff[i]);
        }
        i++;
    }
}
void Sim800lHttp::dumpBuffer(){
    _hSerial->println(F("****BUFFER OVERFLOW****"));
    _hSerial->print(F("Dumping Buffer: "));
    printBuffer();
    _hSerial->print("\n");
    // _hSerial->println(buff);
    flushBuffer();
    _hSerial->println(F("***********************"));
}
void Sim800lHttp::flushBuffer(){
    memset(_buff,0,MAX_BUFF);
    _buff_idx = 0;
}
void Sim800lHttp::setTimer(long delay){
    timer = millis()+delay;
}
void Sim800lHttp::setTimer(long delay, int newState){
    timer = millis()+delay;
    this->timeoutState = newState;
}
void Sim800lHttp::resetTimer(){
    timer =0;
    this->timeoutState = 0;
}
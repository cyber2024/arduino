/*
    Sim800lHttp is a helper class to deal with the SIM800L
*/
#ifndef Sim800lHttp_h
#define Sim800lHttp_h

#include <Arduino.h>

#define MAX_BUFF 512


enum State {IDLE, 
    RESET, RESETTING, 
    // GPRS, GPRS_INITIALIZING, GPRS_CLOSING, GPRS_CLOSED,
    // GPRS, GPRS_INITIALIZING, GPRS_CLOSING, GPRS_CLOSED,

    };

class Sim800lHttp {
    public:
        Sim800lHttp(int resetPin, HardwareSerial &hserial,  HardwareSerial &sserial);
        void update();
        char* post(char url[], char data[]);
        // char* get(char url[]);
        bool* status();
        int findPatternInString(char * string, int stringLen, char * code, int codeLen);
    private:

        int _resetPin;

        char _buff[MAX_BUFF];
        int _buff_idx;

        char url[128];
        char data[512];
        char dataLength[8];

        int state;
        bool posting;
        bool complete;
        bool failed;
        bool awaitingResponse;
        long timer;
        int timeoutState;

        HardwareSerial *_sSerial;
        HardwareSerial *_hSerial;
        
        //update functions
        void updateComms();
        void updateState();
        bool processResponse();
        void setTimer(long delay);
        void setTimer(long delay, int newState);
        void resetTimer();

        //stream (UART) functions
        // void sendCommandToSerial(char * command, int newState);
        // void sendCommandToSerial(int newState);

        //modem sub functions
        void initGPRS();
        void openGPRS();
        void setAPN();
        void checkGPRS();
        void initHTTP();
        void setSSL();
        void setHTTPBearer();
        void setURL();
        void setContent();
        void setMetaData();
        void setData();
        void setAction();
        void readHTTP();
        void terminateHTTP();
        void closeGPRS();

        //buffer functions
        void flushBuffer();
        void dumpBuffer();
        void printBuffer();
        void getline(HardwareSerial &stream);
        void writeToBuffer(char *string, int len);
        void writeToBuffer(char c);
        void writeBufferToModem();
        void writeBufferToHardwareSerial(char *buff);
        void writeBufferToSoftwareSerial(char *buff);

        //state functions
        void setState(int newState);

        //utility functions
        // bool checkResponseForString(char * code);
        // bool checkResponseForString(char * response, char * code);
        // bool checkResponseForString(char * pass, int passCode, char * fail, int failCode);
        void checkResponseForStrings(char * passString, int passStatusCode, char * failString, int failStatusCode);
        void checkResponseForStrings(char * passString, char * failString);
};

#endif

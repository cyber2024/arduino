// TTGO T-Call pins
#define MODEM_RST            5
#define MODEM_PWKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26


// Set serial for debug console (to Serial Monitor, default speed 115200)
//#define Serial Serial
// Set serial for AT commands (to SIM800 module)
#define AT Serial1

#define uS_TO_S_FACTOR 1000000     /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  30        /* Time ESP32 will go to sleep (in seconds) 3600 seconds = 1 hour */

void setup() {
  // Set serial monitor debugging window baud rate to 115200
  Serial.begin(115200);

  // Set modem reset, enable, power pins
  pinMode(MODEM_PWKEY, OUTPUT);
  pinMode(MODEM_RST, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_PWKEY, LOW);
  digitalWrite(MODEM_RST, HIGH);
  digitalWrite(MODEM_POWER_ON, LOW);

  // Set GSM module baud rate and UART pins
  AT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(1000);

  // Configure the wake up source as timer wake up  
//  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}
void loop() {
  linkComms(Serial, AT);
//  esp_deep_sleep_start();
}

char c;
void linkComms(Stream &stream1, Stream &stream2){
  while(stream1.available()){
      c = stream1.read();
      stream1.print(c);
      stream2.print(c);
  }
  while(stream2.available()){
      c = stream2.read();
      stream1.print(c);
  }
}

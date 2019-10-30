#define TX_DATA 5
#define CLK 3
#define TX_RATE 5 //bits per second
const char *message = "Hello World!";


#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_DC    9
#define OLED_RESET 10
#define OLED_CLK   12 //d0
#define OLED_MOSI   11 //d1
// #define OLED_CS    12
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, 0);

volatile bool bit_sent = true;
volatile bool pulsed = false;
int byte_index = 0;
int bit_index = 0;
char tx_byte;
bool tx_bit;

void setup() {
  Serial.begin(115200);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(message);
  display.display();
  
  pinMode(TX_DATA,OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK),onClockPulse,RISING);
  //digitalWrite(CLK,LOW);
  display.setCursor(0,10);
//  for(int byte_idx = 0; byte_idx < strlen(message); byte_idx++){
//    char tx_byte = message[byte_idx];
//    for(int bit_idx = 0; bit_idx < 8; bit_idx++){
//      bool tx_bit = tx_byte & (0x80 >> bit_idx);
//      digitalWrite(TX_DATA,tx_bit);
//      delay((1000/TX_RATE)/2);
//      display.print(tx_bit);
//      display.display();
//      
//      digitalWrite(CLK, HIGH);
//      delay((1000/TX_RATE)/2);
//      digitalWrite(CLK, LOW);
//      
//      
//    }
//    
//      display.print(" ");
//  }
  digitalWrite(TX_DATA,LOW);
}
void onClockPulse(){
    if(byte_index < strlen(message)){
      if(bit_index == 8){
        bit_index = 0;
        byte_index++;
      }
      if(bit_index == 0){
        tx_byte = message[byte_index];
      }
      tx_bit = tx_byte & (0x80 >> bit_index);
      bit_index++;
      digitalWrite(TX_DATA,tx_bit);
      
      pulsed = true;
    }
}
void loop() {
  if(pulsed){
    pulsed = false;  
    if(bit_index == 1 && byte_index > 0){
      display.print(" ");
    }
    Serial.print(byte_index);
    Serial.print(bit_index);
    Serial.println(tx_bit);
    display.print(tx_bit);
    display.display();  
  }
}

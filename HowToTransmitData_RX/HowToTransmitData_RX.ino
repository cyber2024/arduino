#define CLK 3
#define RX_DATA 6 
#define TX_RATE 5 //bits per second


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

char message[16];
volatile byte rx_byte = 0;
volatile int bit_position = 0;  
volatile bool update_lcd = false;
void setup() {
  Serial.begin(115200);
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  
  pinMode(RX_DATA,INPUT);
  attachInterrupt(digitalPinToInterrupt(CLK),onClockPulse, RISING);
  display.setCursor(0,10);
  strcpy(message,"");

}
void onClockPulse(){
  bool rx_bit = digitalRead(RX_DATA);
  if(bit_position == 8){
    bit_position = 0;
    rx_byte = 0;
  }
  if(rx_bit){
    rx_byte |= (0x80 >> bit_position);
  }
  bit_position++;
  if(bit_position == 8){
    strncat(message, &rx_byte,1);
  }
  update_lcd = true;
}
void loop() {
  if(update_lcd){  
    display.clearDisplay();
    display.setCursor(0,0);
    display.print(message);
    display.setCursor(0,10);
    for(int i = 0; i < bit_position; i++){
      display.print(rx_byte & (0x80 >> i) ? "1" : "0");
    }
    display.display();
    update_lcd = false;
  }
  

}

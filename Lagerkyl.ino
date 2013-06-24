/*
 * Copyright 2013 Marcus Kempe
 *
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>

int pwmpin = 10;
int controlval = 255;
OneWire  ds(6);

#define OLED_DC 11
#define OLED_CS 12
#define OLED_CLK 8
#define OLED_MOSI 9
#define OLED_RESET 13
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

float temp1 = 11.1;
float temp2 = 9.4;
float temp3 = 36.4;

void setup()   {                
  Serial.begin(9600);
  setPwmFrequency(pwmpin, 8);
  Serial.print("Current control value: ");
  Serial.print(controlval);
  Serial.println("");
  display.begin(SSD1306_SWITCHCAPVCC);
  display.display(); // show splashscreen
  //delay(2000);
}

byte curSensor = 0;

void loop(){
  if(Serial.available()){
    int val = Serial.read();
    if(val == 97){
      controlval += 10;
    }
    else if(val == 122){
      controlval -= 10;
    }
    if(controlval > 255) controlval = 255;
    if(controlval < 0) controlval = 0;
    Serial.print("Current control value: ");
    Serial.print(controlval);
    Serial.println("");
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("Ctrl-value:");
    display.print(controlval);
    delay(1000);
  }
  analogWrite(pwmpin,controlval);
  
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    //Serial.print//Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  
  //Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    //Serial.write(' ');
    //Serial.print(addr[i], HEX);
  }

  if (OneWire::crc8(addr, 7) != addr[7]) {
      //Serial.println("CRC is not valid!");
      return;
  }
  //Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  } 

  ds.reset();
  ds.select(addr);
  ds.write(0x44,0);         // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  //Serial.print("  Data = ");
  //Serial.print(present,HEX);
  //Serial.print(" ");
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    //Serial.print(data[i], HEX);
    //Serial.print(" ");
  }
  //Serial.print(" CRC=");
  //Serial.print(OneWire::crc8(data, 8), HEX);
  //Serial.println();

  // convert the data to actual temperature

  unsigned int raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // count remain gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw << 3;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw << 2; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw << 1; // 11 bit res, 375 ms
    // default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.println(" Celsius, ");
  //Serial.print(fahrenheit);
  //Serial.println(" Fahrenheit");
  
  switch(curSensor){
    case 0: //Lower
      temp2 = celsius;
      break;
    case 1: //Upper
      temp1 = celsius;
      break;
    case 2: //Heatsink
      temp3 = celsius;
      update();
      break;        
  }
  curSensor++;
  if(curSensor > 2) curSensor = 0;  
}



void calculate(){
  //temp1 = random(-55,99);
  //temp2 = random(-55,99);
  //temp3 = random(-55,99);
}

void update(){
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  if(temp1 >= 10){  
    display.print("Upper        ");    
  }
  else if(temp1 < 0 && temp1 > -10){
    display.print("Upper        ");
  }
  else if(temp1 <= -10){
    display.print("Upper       ");
  }
  else{
    display.print("Upper         ");
  }
  display.print(temp1);
  display.print(" ");
  display.write(247);
  display.print("C");
  if(temp2 >= 10){  
    display.print("Lower        ");    
  }
  else if(temp2 < 0 && temp2 > -10){
    display.print("Lower        ");
  }
  else if(temp2 <= -10){
    display.print("Lower       ");
  }
  else{
    display.print("Lower         ");
  }
  display.print(temp2);
  display.print(" ");
  display.write(247);
  display.print("C");
  if(temp3 >= 10){  
    display.print("Heatsink     ");    
  }
  else if(temp3 < 0 && temp3 > -10){
    display.print("Heatsink     ");
  }
  else if(temp3 <= -10){
    display.print("Heatsink    ");
  }
  else{
    display.print("Heatsink      ");
  }
  display.print(temp3);
  display.print(" ");
  display.write(247);
  display.print("C");
  display.display();
  delay(10);
}

/**
 * Divides a given PWM pin frequency by a divisor.
 * 
 * The resulting frequency is equal to the base frequency divided by
 * the given divisor:
 *   - Base frequencies:
 *      o The base frequency for pins 3, 9, 10, and 11 is 31250 Hz.
 *      o The base frequency for pins 5 and 6 is 62500 Hz.
 *   - Divisors:
 *      o The divisors available on pins 5, 6, 9 and 10 are: 1, 8, 64,
 *        256, and 1024.
 *      o The divisors available on pins 3 and 11 are: 1, 8, 32, 64,
 *        128, 256, and 1024.
 * 
 * PWM frequencies are tied together in pairs of pins. If one in a
 * pair is changed, the other is also changed to match:
 *   - Pins 5 and 6 are paired on timer0
 *   - Pins 9 and 10 are paired on timer1
 *   - Pins 3 and 11 are paired on timer2
 * 
 * Note that this function will have side effects on anything else
 * that uses timers:
 *   - Changes on pins 3, 5, 6, or 11 may cause the delay() and
 *     millis() functions to stop working. Other timing-related
 *     functions may also be affected.
 *   - Changes on pins 9 or 10 will cause the Servo library to function
 *     incorrectly.
 * 
 * Thanks to macegr of the Arduino forums for his documentation of the
 * PWM frequency divisors. His post can be viewed at:
 *   http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1235060559/0#4
 */
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

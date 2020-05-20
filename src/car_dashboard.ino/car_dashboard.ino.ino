#include <Arduino.h>
#include <SPI.h>

#include "Adafruit_ILI9341esp.h"
#include "Adafruit_GFX.h"
#include "XPT2046.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define TFT_DC 2
#define TFT_CS 15

/******************* Temperature Sensor => D3 (eps) */
#define ONE_WIRE_BUS D3

/******************* UI details */
#define BUTTON_X 40
#define BUTTON_Y 20
#define BUTTON_W 60
#define BUTTON_H 30
#define BUTTON_SPACING_X 20
#define BUTTON_SPACING_Y 20
#define BUTTON_TEXTSIZE 2

// text box where numbers go
#define TEXT_X 10
#define TEXT_Y 10
#define TEXT_W 220
#define TEXT_H 50
#define TEXT_TSIZE 3
#define TEXT_TCOLOR ILI9341_MAGENTA

// the data (phone #) we store in the textfield
#define TEXT_LEN 12
char textfield[TEXT_LEN + 1] = "";
uint8_t textfield_i = 0;

// Setup a oneWire instance to communicate with any OneWire devices
//OneWire oneWire(ONE_WIRE_BUS);
OneWire  ds(D3);
// Pass our oneWire reference to Dallas Temperature.
//DallasTemperature sensors(&oneWire);
// variable to hold device addresses
DeviceAddress Thermometer;
int deviceCount = 0;
float tempSensor1;
uint8_t sensor1[8] = { 0x28, 0x14, 0x3F, 0x79, 0x97, 0x19, 0x03, 0xA7  };
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/ 5);

/* create 15 buttons, in classic candybar phone style */
char buttonlabels[15][5] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "*", "0", "#", "<", "Ok", ">" };
uint16_t buttoncolors[15] = {ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE,
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE,
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE,
                             ILI9341_BLUE, ILI9341_BLUE, ILI9341_BLUE,
                             ILI9341_ORANGE, ILI9341_GREEN, ILI9341_ORANGE
                            };
Adafruit_GFX_Button buttons[15];

void setup() {
  // put your setup code here, to run once:
  delay(1000);

  Serial.begin(115200);
  SPI.setFrequency(ESP_SPI_FREQ);
  // Start reading from D3
//  sensors.begin();
  delay(1000);
  tft.begin();
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
  Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
  // Replace these for your screen module
  touch.setCalibration(1864, 1792, 280, 251);
//  // locate devices on the bus
//  Serial.println("Locating devices...");
//  Serial.print("Found ");
//  deviceCount = sensors.getDeviceCount();
//  Serial.print(deviceCount, DEC);
//  Serial.println(" devices.");
//  Serial.println("");
//  
//  Serial.println("Printing addresses...");
//  for (int i = 0;  i < deviceCount;  i++)
//  {
//    Serial.print("Sensor ");
//    Serial.print(i+1);
//    Serial.print(" : ");
//    sensors.getAddress(Thermometer, i);
//    printAddress(Thermometer);
//  }
  // createNumpad();

  // create 'text field'
  //tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);

}

void printAddress(DeviceAddress deviceAddress)
{ 
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.print("0x");
    if (deviceAddress[i] < 0x10) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(", ");
  }
  Serial.println("");
}

void createNumpad() {
  // create buttons
  for (uint8_t row = 0; row < 5; row++) {
    for (uint8_t col = 0; col < 3; col++) {
      buttons[col + row * 3].initButton(&tft, BUTTON_X + col * (BUTTON_W + BUTTON_SPACING_X),
                                        BUTTON_Y + row * (BUTTON_H + BUTTON_SPACING_Y), // x, y, w, h, outline, fill, text
                                        BUTTON_W, BUTTON_H, ILI9341_WHITE, buttoncolors[col + row * 3], ILI9341_WHITE,
                                        buttonlabels[col + row * 3], BUTTON_TEXTSIZE);
      buttons[col + row * 3].drawButton();
    }
  }
}
float printTemperature(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
 
  if ( !ds.search(addr)) 
  {
    ds.reset_search();
    delay(250);
    return -1000;
  }
 
 
  if (OneWire::crc8(addr, 7) != addr[7]) 
  {
      Serial.println("CRC is not valid!");
      return -1000;
  }
  Serial.println();
 
  // the first ROM byte indicates which chip
  switch (addr[0]) 
  {
    case 0x10:
      type_s = 1;
      break;
    case 0x28:
      type_s = 0;
      break;
    case 0x22:
      type_s = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      return -1000;
  } 
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end  
  delay(1000);
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
 
  for ( i = 0; i < 9; i++) 
  {           
    data[i] = ds.read();
  }
 
  // Convert the data to actual temperature
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) 
    {
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } 
  else 
  {
    byte cfg = (data[4] & 0x60);
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
 
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  
  return celsius;
}

void loop() {
  // put your main code here, to run repeatedly:
  uint16_t x, y;
  tft.setCursor(30, 90);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  float t = printTemperature();
  if(t > -1000) {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(9);
    tft.print(t);

    delay(10);
  }
//  tempSensor1 = sensors.getTempC(sensor1); // Gets the values of the temperature
//  Serial.print("Temperature sensor 1: ");  Serial.println(tempSensor1);
  if (touch.isTouching())
    touch.getPosition(x, y);

  // go thru all the buttons, checking if they were pressed
  for (uint8_t b = 0; b < 15; b++) {
    if (buttons[b].contains(x, y)) {
      //Serial.print("Pressing: "); Serial.println(b);
      buttons[b].press(true);  // tell the button it is pressed
    } else {
      buttons[b].press(false);  // tell the button it is NOT pressed
    }
  }

  // now we can ask the buttons if their state has changed
  for (uint8_t b = 0; b < 15; b++) {
    if (buttons[b].justReleased()) {
      // Serial.print("Released: "); Serial.println(b);
      buttons[b].drawButton();  // draw normal
    }

    if (buttons[b].justPressed()) {
      buttons[b].drawButton(true);  // draw invert!

      // if a numberpad button, append the relevant # to the textfield
      if (b >= 3) {
        if (textfield_i < TEXT_LEN) {
          textfield[textfield_i] = buttonlabels[b][0];
          textfield_i++;
          textfield[textfield_i] = 0; // zero terminate

          //fona.playDTMF(buttonlabels[b][0]);
        }
      }

      // clr button! delete char
      if (b == 1) {

        textfield[textfield_i] = 0;
        if (textfield > 0) {
          textfield_i--;
          textfield[textfield_i] = ' ';
        }
      }

      // update the current text field
      Serial.println(textfield);
      tft.setCursor(TEXT_X + 2, TEXT_Y + 10);
      tft.setTextColor(TEXT_TCOLOR, ILI9341_BLACK);
      tft.setTextSize(TEXT_TSIZE);
      tft.print(textfield);

      // its always OK to just hang up
      if (b == 2) {
        //status(F("Hanging up"));
        //fona.hangUp();
      }
      // we dont really check that the text field makes sense
      // just try to call
      if (b == 0) {
        //status(F("Calling"));
        //Serial.print("Calling "); Serial.print(textfield);

        //fona.callPhone(textfield);
      }
      delay(100); // UI debouncing
    }
  }

}

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_ILI9341esp.h"
#include "Adafruit_GFX.h"
#include "XPT2046.h"
#include <OneWire.h>
#include <DallasTemperature.h> // ???
#include <Wire.h>       //I2C library
#include <RtcDS3231.h>  //RTC library

/******************************** Setup display GPIOs */
#define TFT_DC 2
#define TFT_CS 15

/******************************** Setup display lib instances */
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/ 5);

/******************************** UI details */
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

/******************************** Setup a oneWire instance to communicate with any thermometers */
OneWire  ds(D3);
float tempSensor1;
uint8_t sensor1[8] = { 0x28, 0x14, 0x3F, 0x79, 0x97, 0x19, 0x03, 0xA7  }; // address for device 1
RtcDS3231<TwoWire> Rtc(Wire);

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
  /*           serial communtication start          */
  Wire.begin(10, 9);
  Serial.begin(115200);
  SPI.setFrequency(ESP_SPI_FREQ);
  delay(1000);
  /*           RTC             */
  RTCSetup();
  /*           TFT             */
  tft.begin();
  touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
  tft.fillScreen(ILI9341_BLACK);
  tft.setRotation(3);
  // Replace these for your screen module from calibration sketch
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

  // create 'text field'
  //tft.drawRect(TEXT_X, TEXT_Y, TEXT_W, TEXT_H, ILI9341_WHITE);

}

#define countof(a) (sizeof(a) / sizeof(a[0]))

void printDateTime(const RtcDateTime& dt)
{
    char datestring[20];

    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    //Serial.print(datestring);
    tft.setCursor(1,1);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(2);
    tft.print(datestring);
}

void RTCSetup(){
    //--------RTC SETUP ------------
    // if you are using ESP-01 then uncomment the line below to reset the pins to
    // the available pins for SDA, SCL
    Wire.begin(3, 1); 
    Rtc.Begin(); 
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) first time you ran and the device wasn't running yet
            //    2) the battery on the device is low or even missing

            Serial.println("RTC lost confidence in the DateTime!");

            // following line sets the RTC to the date & time this sketch was compiled
            // it will also reset the valid flag internally unless the Rtc device is
            // having an issue

            Rtc.SetDateTime(compiled);
        }
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }

    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
    {
        Serial.println("RTC is newer than compile time. (this is expected)");
    }
    else if (now == compiled) 
    {
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
    }

    // never assume the Rtc was last configured by you, so
    // just clear them to your needed state
    Rtc.Enable32kHzPin(false);
    Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone); 
}

void printTime(){
  if (!Rtc.IsDateTimeValid()) 
    {
        if (Rtc.LastError() != 0)
        {
            // we have a communications error
            // see https://www.arduino.cc/en/Reference/WireEndTransmission for 
            // what the number means
            Serial.print("RTC communications error = ");
            Serial.println(Rtc.LastError());
        }
        else
        {
            // Common Causes:
            //    1) the battery on the device is low or even missing and the power line was disconnected
            Serial.println("RTC lost confidence in the DateTime!");
        }
    }

    RtcDateTime now = Rtc.GetDateTime();
    printDateTime(now);
    //Serial.println();

//  RtcTemperature temp = Rtc.GetTemperature();
//  temp.Print(Serial);
//  // you may also get the temperature as a float and print it
//    // Serial.print(temp.AsFloatDegC());
//    Serial.println("C");

    //delay(10000); // ten seconds
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

  printTime();
  tft.setCursor(30, 90);
  tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
  float t = printTemperature();
  if(t > -1000) {
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLACK);
    tft.setTextSize(6);
    tft.print(t);

    delay(10);
  }
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

//        textfield[textfield_i] = 0;
//        if (textfield > 0) {
//          textfield_i--;
//          textfield[textfield_i] = ' ';
//        }
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

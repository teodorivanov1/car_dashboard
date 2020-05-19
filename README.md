# Car Dashboard with ESP8266 and TFT Display
Reading three temperature sensors and one RTC interfaced with 2.4 inch TFT display

## Hardware
* NodeMCU - HW-625
* 2.4 inch 320*240 SPI Serial TFT LCD Module Display Screen with Touch Panel Driver IC ILI9341 for MCU
* DS18B20 - waterproof temperature sensor
## Display wiring

    VCC         => 3.3V
    GND         => GND
    CS          => D8
    RESET       => RST
    DC          => D4
    SDI (MOSI)  => D7
    SCK         => D5
    LED         => 3.3V
    SDD         => D6
    T_CLK       => D5
    T_CS        => D2
    T_DIN       => D7
    T_DO        => D6
    T_IRQ       => D1

## Built With

* [ArduinoIDE]              https://www.arduino.cc/en/Main/Software - v 1.8.12
* [nobrok]                  http://nobrok.com/connecting-tft-lcd-touch-screen-with-nodemcu-esp8266/ - tft display wiring
* [TFT Display wiki page]   http://www.lcdwiki.com/2.4inch_SPI_Module_ILI9341_SKU:MSP2402

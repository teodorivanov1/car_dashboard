# Simple car dashboard
[![Build Status](https://travis-ci.org/joemccann/dillinger.svg?branch=master)](https://travis-ci.org/joemccann/dillinger)

Car dashboard v1.0 can provide information from three temperature sesnors and show current date and time.
# Hardware
  - DS18B20 - direct waterproof  digital temperature sensor (probe)
  - RTC-DS3231 - real time clock
  - NodeMCU HW-625 - NodeMCU 12E clone
  - 2.4 inch TFT LCD Display module - ILI9341 driver
### Display wiring (Display <----> NodeMCU)

    VCC         <---->      3.3V
    GND         <---->      GND
    CS          <---->      D8
    RESET       <---->      RST
    DC          <---->      D4
    SDI (MOSI)  <---->      D7
    SCK         <---->      D5
    LED         <---->      3.3V
    SDD (MISO)  <---->      D6
    T_CLK       <---->      D5
    T_CS        <---->      D2
    T_DI        <---->      D7
    T_DO        <---->      D6
    T_IRQ        <---->     D1

### RTC wiring (RTC <----> NodeMCU)
    VCC         <---->      3.3V
    GND         <---->      GND
    SDA         <---->      RX
    SCL         <---->      TX

### DS18B20(thermometer) wiring (DS18B20 <----> NodeMCU)
    VCC         <---->      3.3V
    GND         <---->      GND
    DAT         <R47K>      D3

> Used sources for display touch issues
http://nobrok.com/connecting-tft-lcd-touch-screen-with-nodemcu-esp8266/

# Software
* ArduinoIDE v1.8.12

# Installation

License
----

MIT


**Free Software, Hell Yeah!**


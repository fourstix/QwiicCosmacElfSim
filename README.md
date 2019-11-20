QwiicCosmacElfSim
==================

An 1802 Cosmac Elf Simulator using an Arduino with Qwiic Components.

Introduction
-------------

This 1802 Cosmac Elf Arduino Qwiic simulator is based on code written by Al Williams for the Kim-UNO
DYI Kim-1 clone written by Oscar Vermeulen.

The Sparkfun Qwiic interface is a 3.3V I2C based intervace that makes it very easy to connect various
hardware to the Arduiono.  All hardware defineitions for this code are configured in the config.h file.
By changing the definitions in the config.h file one can change the hardware used by the simulation.
All hardware components are optional, and maybe used in any combination.

It's even possible to comment out all hardware defines in the config.h file and use the simulation on
an Arduino alone through its serial interface.

Information on the Sparkfun Qwiic interface is available [here.](https://www.sparkfun.com/qwiic)

The code can use the following hardware as part of the 1802 simulation:

This simiulator uses [Adafruit 7 segment LED backpack](https://www.adafruit.com/product/878) for
Hexadecimal output.
 
A [Sparkfun Qwiic 3x4 Keypad](https://www.sparkfun.com/products/15290) can be used for key input.  The star key is used as an escape key for
hexadeciamal keys A-F (*, 0-6) and control inputs, like run, wait (*8), load (*7), etc.
 

This code simulates a cdp1861 Pixi video chip to load a video ram buffer, rather than just dumping
the 256 bytes of ram to the display.  This code uses a 128 x 64 graphics display supported by the
U8G2 library as a video display.  U8G2 supports many kinds of 128 x 64 displays.  

The U8G2 graphics library is available at: https://github.com/olikraus/u8g2

Please see https://github.com/olikraus/u8g2/wiki/u8g2setupcpp for a list of supported displays.
 
This SSD1306 I2C 128 x64 OLED display available from Adadruit works fine with Qwiic.
https://www.adafruit.com/product/938
 
This code can support the [Sparkfun Qwiic EEPROM](https://www.sparkfun.com/products/14764) as a ROM source.

 
It can also support an [Adafruit I2C 32K Fram](https://www.adafruit.com/product/1895) as an expanded RAM
memory as well. The expanded ram is accessed in 256-byte pages.

 

License Information
-------------------

This code is public domain under the MIT License, but please buy me a beer
if you use this and we meet someday (Beerware).

References to any products, programs or services do not imply
that they will be available in all countries in which their respective owner operates.

Sparkfun, the Sparkfun logo, and other Sparkfun products and services are
trademarks of the Sparkfun Electronics Corporation, in the United States,
other countries or both. 

Adafruit, the Adafruit logo, and other Adafruit products and services are
trademarks of the Adafruit Industries, in the United States,other countries or both. 

Other company, product, or services names may be trademarks or services marks of others.

All libraries used in this code are copyright their respective authors.
  
Universal 8bit Graphics Library
Copyright (c) 2016, olikraus
All Rights Reserved
 
Sparkfun Qwiic Keypad Arduino Library
Copyright (c) 2016 SparkFun Electronics
Written by Pete Lewis @ SparkFun Electronics, 3/12/2019

Adadruit LED Backpack Library
Copyright (c) 2012 Adafruit Industries
Written by Limor Fried/Ladyada, 2012 

Adafruit Fram I2C Library
Copyright (c) 2012 Adafruit Industries
Written by KTown, 2013

1802 Simulation for KIM-UNO hardware
Copyright (c) 2017 by Al Williams
Written by Al Williams, July 2017 
 
Kim-UNO DYI Kim-1 clone 
Copyright (c) 2016
Written by Oscar Vermeulen

Many thanks to the original authors for making their designs and code avaialble as open source.
 

This code, firmware, and software is released under the [MIT License](http://opensource.org/licenses/MIT).

The MIT License (MIT)

Copyright (c) 2019 by Gaston Williams

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

**THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.**
QwiicCosmacElfSim
==================

This is an 1802 Cosmac Elf Simulator using an Arduino with Qwiic Components. 

My first computer was a Netronics Elf II. It was an RCA 1802 based single board computer that was sold as a kit,
but it had a hexadecimal keypad, an LED, video and an expansion bus. Like many high school kids, I mowed yards
and spent my hard-earned dollars to buy a kit from Netronics [based on their ads.](http://www.cosmacelf.com/gallery/netronics-ads/)

My orignal Elf II was lost in a move long ago, so I wanted to simulate the Elf using hardware I had at hand. 
I rewrote the UNO1802 code to support an Arduino with Qwiic hardware to simulate the video, seven segment display and
hexadecimal keypad from my original Elf.

Introduction
-------------

A very good source of information on the RCA 1802 chip and Cosmac Elf computer can be found on the 
[CosmacElf web page.](http://www.cosmacelf.com) The 1802 was a fantastic microprocessor that still has quite a 
dedicated following today.

This 1802 Cosmac Elf Arduino Qwiic simulator is based on the UNO1802 code written by Al Williams for the Kim-UNO
DYI Kim-1 clone written by Oscar Vermeulen.  Information on the 1802 opcode set is available in the 
[documentation.](https://github.com/fourstix/QwiicCosmacElfSim/blob/master/docs/QwiicCosmacElfSim.pdf)

The Sparkfun [Qwiic](https://www.sparkfun.com/qwiic) interface is a 3.3V I2C based interface that makes
it very easy to connect various hardware to the Arduiono.  All hardware definitions for this code are 
configured in the config.h file.

By changing the definitions in the config.h file one can change the hardware used by the simulation.
All hardware components are optional, and may be used in any combination.

It's even possible to comment out all hardware defines in the config.h file and use the simulation on
an Arduino alone through its serial monitor interface.

Information on the Sparkfun Qwiic interface is available [here.](https://www.sparkfun.com/qwiic)

The code can use the following hardware as part of the 1802 simulation:

This simiulator uses [Adafruit 7 segment LED backpack](https://www.adafruit.com/product/878) for
hexadecimal output Please see the 
[documentation](https://github.com/fourstix/QwiicCosmacElfSim/blob/master/docs/QwiicCosmacElfSim.pdf)
for more information.
 
A [Sparkfun Qwiic 3x4 Keypad](https://www.sparkfun.com/products/15290) can be used for key input. The
star key (*) is used as a shift key for hexadeciamal keys A-F (*, 0-6) and control inputs, 
like run, wait (*8), load (*7), etc.  The hash key (#) is used as the input key. See the 
[documentation](https://github.com/fourstix/QwiicCosmacElfSim/blob/master/docs/QwiicCosmacElfSim.pdf)
for more information.
 

This code simulates a cdp1861 Pixi video chip, rather than just dumping the 256 bytes of ram to the 
display.  This code uses a video ram buffer with a 128 x 64 graphics display supported by the
[U8G2 graphics library](https://github.com/olikraus/u8g2) as a video display.  The code will simulate
the interrupts, external flag 1 signal, and DMA Output requests from the original pixie video.  This
allows [programs](https://github.com/fourstix/QwiicCosmacElfSim/blob/master/docs/Cdp1802SampleProgramCode.txt)
written for the original Cosmac Elf hardware to run directly on the simulator.

U8G2 supports many kinds of 128 x 64 displays.  A list of supported displays is available 
[here.](https://github.com/olikraus/u8g2/wiki/u8g2setupcpp)


For example, this [SSD1306 I2C 128 x64 OLED display](https://www.adafruit.com/product/938) available
from Adadruit works fine with the Qwiic interface and is supported by Uthe 8G2 graphics library.

 
This code can support the [Sparkfun Qwiic EEPROM](https://www.sparkfun.com/products/14764) as a ROM source.

 
It can also support an [Adafruit I2C 32K Fram](https://www.adafruit.com/product/1895) as an expanded RAM
memory as well. The expanded ram is accessed in 256-byte pages.

Note that EEProm and Fram share the same default I2C address, but either or both addresses can be changed.

Sample Configurations
---------------------
Here are some sample configurations running actual [CDP1802 programs](https://github.com/fourstix/QwiicCosmacElfSim/blob/master/docs/Cdp1802SampleProgramCode.txt)
on different combinations of Qwiic compatible hardware.

<table class="table table-hover table-striped table-bordered">
  <tr align="center">
   <td><img src="https://github.com/fourstix/QwiicCosmacElfSim/blob/master/pics/QwiicCosmacElfSim_EEProm.jpg"></td>
   <td><img src="https://github.com/fourstix/QwiicCosmacElfSim/blob/master/pics/QwiicCosmacElfSim_Fram.jpg"></td> 
  </tr>
  <tr align="center">
    <td>Sparkfun Blackboard, Sparkfun Qwiic EEProm, SSD1306 128x64 OLED display, Sparkfun Qwiic 3x4 Keypad and Adafruit 7 Segment backpack</td>
    <td>Sparkfun Redboard Arduino with Sparkfun Qwiic Shield, SH1106 128x64 OLED display, Sparkfun Qwiic 3x4 Keypad, Adafruit 7 Segment backpack and Adafruit I2C 32K Fram</td>
  </tr>
  <tr align="center">
   <td><img src="https://github.com/fourstix/QwiicCosmacElfSim/blob/master/pics/QwiicCosmacElfSim_Monochron.jpg"></td>
   <td><img src="https://github.com/fourstix/QwiicCosmacElfSim/blob/master/pics/QwiicCosmacElfSim_Other.jpg"></td> 
  </tr>
  <tr align="center">
    <td>Sparkfun Blackboard, Adafruit Monochron 128x64 OLED display with a homemade Arduino shield, Sparkfun Qwiic 3x4 Keypad and Adafruit 7 Segment backpack</td>
    <td>Sparkfun Blackboard, Sparkfun Qwiic EEProm, SSD1306 Blue/Yellow 128x64 OLED display, Sparkfun Qwiic 3x4 Keypad and Adafruit 7 Segment backpack</td>
  </tr>  
</table>

Repository Contents
-------------------
* **/src/CosmacElfSim/**
  * CosmacElfSim.ino -- main Arduino file
  * Cdp1802.ino -- 1802 processor simulation engine based largely on Al Williams code.
  * InputOutput.ino -- all input/output routines for Qwiic devices
  * PixieVideo.ino -- simulation of Pixie Video using a 128 x 64 graphic display. 
  * Rom.ino -- rom simulator using Arduino PROGMEM or Sparkfun Qwiic EEProm
  * Config.h -- definitions to enable/disable support for various qwiic hardware
* **/docs** -- documentation files
  * QwiicCosmacElfSim.pdf -- documentation of 1802 Instructions, Keypad and 7 Segment backpack
  * Cdp1802SampleProgramCode.txt -- Sample 1802 code listings
* **/pics** -- pictures of sample configurations


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
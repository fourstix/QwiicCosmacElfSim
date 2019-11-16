/*
 * Arduino Simulator for Cosmac Elf
 * Copyright (c) 2019 by Gaston Williams
 * 
 * Based on code written by Al Williams for
 * the Kim-UNO DYI Kim-1 clone written by Oscar Vermeulen.
 * 
 * This simiulator uses Adafruit 7 segment LED backpack 
 * for Hexadecimal output. It is available at:
 * https://www.adafruit.com/product/878
 * 
 * A Sparkfun Qwiic 3x4 Keypad was used for key input.  The
 * star key is used as an escape key for hexadeciamal keys A-F 
 * and control inputs, like run, wait, load, etc.
 * 
 * The 12 button Qwiic Keypad is available here:
 * https://www.sparkfun.com/products/15290
 * 
 * This code simulates a cdp1861 Pixi video chip to load a video
 * ram buffer, rather than just dumping the 256 bytes of ram to 
 * the display.
 *
 * The U8G2 graphics library is available at:
 * https://github.com/olikraus/u8g2
 * 
 * This code uses a 128 x 64 graphics display supported by the U8G2 library
 * as a video display.  U8G2 supports many kinds of 128 x 64 displays.  
 * 
 * Please see https://github.com/olikraus/u8g2/wiki/u8g2setupcpp for a list.
 * 
 * This SSD1306 I2C 128 x64 OLED display available from 
 * Adadruit works fine: https://www.adafruit.com/product/938
 * 
 * This code can also support the Sparkfun Qwiic EEPROM
 * as a ROM source. https://www.sparkfun.com/products/14764
 * 
 * It can also support an Adafruit I2C 32K Fram as an expanded RAM
 * memory as well. The expanded ram is accessed in 256-byte pages.
 * https://www.adafruit.com/product/1895
 * 
 * Hardware is defined in the config.h tab
 * 
 * All libraries are copyright their respective authors.
 * 
 * Universal 8bit Graphics Library
 * Copyright (c) 2016, olikraus@gmail.com
 * All Rights Reserved
 * 
 * Sparkfun Qwiic Keypad Arduino Library
 * Copyright (c) 2016 SparkFun Electronics
 * Written by Pete Lewis @ SparkFun Electronics, 3/12/2019
 * 
 * Adadruit LED Backpack Library
 * Copyright (c) 2012 Adafruit Industries
 * Written by Limor Fried/Ladyada, 2012 
 * 
 * Adafruit Fram I2C Library
 * Copyright (c) 2012 Adafruit Industries
 * Written by KTown, 2013
 * 
 * 1802 Simulation for KIM-UNO hardware
 * Copyright (c) 2017 by Al Williams
 * Written by Al Williams, July 2017 
 * 
 * Kim-UNO DYI Kim-1 clone 
 * Copyright (c) 2016
 * Written by Oscar Vermeulen
 * 
 * Many thanks to the original authors for making their designs and code avaialble.
 */
#include <Wire.h>
#include "config.h" // contains configuration definitions

// elf ram buffer
uint8_t elf_ram[ELF_RAM_SIZE]; 

/*
 * Cdp1802 registers and flags
 */
//Cdp1802 8-bit registers
byte p_reg = 0;
byte x_reg = 0;
byte d_reg = 0;
byte t_reg = 0;

//Cdp1802 16-bit registers
uint16_t reg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// Cdp1802 single bit registers
boolean df = false;
boolean ie = true;
boolean q_reg = false;

//Cdp1802 external flags
boolean ef1 = false; // used for video
boolean ef2 = false;
boolean ef3 = false;
boolean ef4 = false; // Used for keyboard input

//Simulate cdp1802 interrupt request line
boolean irq = false;

//Status flags for cdp1802 state
boolean run_code = false;
boolean load_code = false;  
boolean cpu_idle  = false;
boolean mem_protect = false; 

//Single-step mode for debugging
boolean single_step = false;

//Data bus for hex display
byte databus = 0;  
//Input from keypad or serial 
//Separate from databus so OUT 4 won't overwrite
byte keybuffer = 0; 

//Address of instruction
uint16_t address = 0;

// Video parameters
boolean video    = false; // turn video on/off
// video cycle counter, reset to zero when video changes state'
unsigned int cycle = 0; 

// Used to indicate an update to display
boolean update_display = false;

//Used to indicate more frequent and complete display updates
boolean verbose_status = false;

//Initialize all registers to zero
void initReg() {
  p_reg = 0;
  x_reg = 0;
  t_reg = 0;
  d_reg = 0;
  q_reg = 0;
  df = 0;
  ie = 1;
  
  for (int i = 0; i < 16; i++) {
    reg[i] = 0;
  } // for
  ef4 = false;
  //Turn of the LED when Q is false
  digitalWrite(Q_PIN, LOW);
} //initReg


//Clear the registers
void reset1802() {
  p_reg = 0;
  x_reg = 0;
  q_reg = false;
  //Register 0 is cleared
  reg[p_reg] = 0;
  // Registers 1-F are not affected
  // T register is not cleared and
  // DF is not affected
  ie = true;
  cpu_idle = false;
  //Turn of the LED when Q is false
  digitalWrite(Q_PIN, LOW);
  //Clear the keypad buffer
  keybuffer = 0;
  //Set the address and opcode for display
  address = reg[p_reg];
  databus = readByte(address);
  //Turn video off
  stopVideo();
} // reset1802

//Check the ram page
 
// read a byte from memory
uint8_t readByte(uint16_t addr) {
  byte data = 0xFF;      
  
  if (isRamAddress(addr)) {
    data = readByteFromRam(addr);    
  } else if (addr >= ROM_ADDR) {
    
    data = readByteFromRom(addr);
  } // if-else if
  return data;
} // readByte

// write a byte to memory
boolean writeByte(uint16_t addr, uint8_t data) {
  boolean result = false;
  
  if (isRamAddress(addr) && !mem_protect ) {        
    // elf_ram[addr] = data;
    writeByteToRam(addr, data);
    result = true;
  }  // if addr < ELF_RAM_SIZE
  return result;
} // writeByte

// Set first 256 of ram memory to 0x00
void eraseRamMemory() {
  for (int addr = 0; addr < ELF_RAM_SIZE; addr++) {
    elf_ram[addr] = 0x00;
  } // for
} 

void setup() {
  Wire.begin();  // start I2C
  Wire.setClock(400000); // fast I2C mode
  
  Serial.begin (115200); // Start serial

  //Set up Q output
  pinMode(Q_PIN, OUTPUT);

#ifdef QWIIC_KEYPAD  
  //Set up pin for keypad input
  pinMode(KEY_RDY_PIN, INPUT);
#endif
  
  //Setup Keypad, LED Backpack and Fram
  setupInOutDevices();
  
  //Setup OLED for Pixie Video Simulation
  setupVideo();

 

  //Set up initial cpu state
  initReg();
  databus = 0;
  video = false;
  mem_protect = false;


  displayStatus();
  showMenu();
} // setup

void loop() {
  //Reset flag for next display
  update_display = false;
  
  //Process input from keypad and Serial
  processKeyInput();
  
  // Logic to load bytes in ram
  if (load_code) {
    if (ef4) {
      ef4 = false;
      if (!mem_protect) {
        writeByte(address, databus);         
      } // if mem_protect
      reg[p_reg]++;
      address = reg[p_reg];
      databus = readByte(address);
      update_display = true;  // always update after load
      } // if ef4
  } // if load_code

  //Logic to run bytes from ram
  if (run_code) {
    if (ie && irq) {
      handleInterrupt();
      cycle = 0; // We must count exactly 29 cycles after interrupt
      update_display = verbose_status;
    } // ie && irq
    
    if (cpu_idle) {
      // wait for interrupt or dma request
      if (video) {
        cycle++;
      }
    } else {
      // get an opcode and execute it 
      byte opcode = 0;     
      address = reg[p_reg];
      reg[p_reg]++;
      opcode = readByte(address);
      execute1802(opcode);
      if (video) {
        // Update cycle count for video synchronization    
        if (opcode > 0xC0 && opcode < 0XCF) {
          // Long Skip and Long branches take 3 cycles
          cycle += 3;
        } else {
          // Every other opcde take 2 cycles
          cycle += 2;
        } // if else opcde
      } // if video
    } // if-else cpu_idle

    //Simulate pixie video hardware after instruction was run
    if (video) {
      runPixieVideo();
    } // if video
    
    // Only run one instruction if in single step mode
    if (single_step) {
      run_code = false;      
      //Show next address
      address = reg[p_reg];
          
      //If nothing ready to show, then show next Opcode
      if (!update_display) {
        databus = readByte(address);       
      } // if !update_display
      
      // Always update after each step
      update_display = true; 
    } // if single_step           
  } // if run_code

  // Show CPU status if changed
  if (update_display) {
    displayStatus();
    update_display = false;  
  } // if update display
} // loop

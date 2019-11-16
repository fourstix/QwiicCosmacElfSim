/*
 * This file contains all the defines to enable or disable support for various
 * hardware in the Qwiic Cosmac Elf Simulator.
 * 
 * All hardware is accessed via Sparkfun Qwiic interface, which is 3.3v I2C.  
 * You can learn more about Qwiic here:  https://www.sparkfun.com/qwiic
 * 
 */
//Comment/uncommment next line to remove/add code for Adafruit I2C LED Backpack
#define LED_BACKPACK

//Comment/uncomment next line to remove/add code for Sparkfun QWiic 3x4 Keypad
#define QWIIC_KEYPAD

//Comment/uncomment next line to remove/add code for U8G2 128 x 64 Graphic Display
//U8G2 OLED constructor for display in PixieVideo tab
#define U8G2_DISPLAY

//Comment/uncomment next line to remove/add code for Sparkfun Qwiic EEPROM
//#define QWIIC_EEPROM

//Comment/uncomment next line to remove/add code for Adafruit I2C 32K FRam
#define I2C_FRAM

//Adafruit I2C FRAM is 32K
#define FRAM_SIZE 0x8000

//Adafruit I2C Fram address 
//Conflicts with Qwiic EEProm address, but can be changed using fram A0,A1,A2 pins
#define FRAM_ADDR 0x50

//I2C address for Qwiic EEPROM
#define EEPROM_ADR 0x50

//Maximum number of bytes in EEPROM cache
#define CACHE_SIZE 64

//Bit Mask used in Cache addressing
#define ROM_CACHE_MASK 0x003F

#define ROM_CACHE_DIRTY 0x0000

//Base address for ELf Spaceship code in ROM
#define SPACESHIP_BASE 0x8000

//Base address for EETops program in ROM
#define EETOPS_BASE 0x8100

//ROM simulated in Arduino Progmem
#define ROM_ADDR   0x8000

//Original Elf had only 256 bytes
#define ELF_RAM_SIZE 0x0100

//Pin for Q register output
//Change this pin if you wish to use SPI
#define Q_PIN 13

//Pin for Keyboard key ready line
#define KEY_RDY_PIN 2

//IO Definitions
//Space character used for invalid key input
#define NO_KEY_SPACE ' '

//Time to hold input key for keystroke during run
#define KEY_PRESS_DURATION 7200
//Time to release input key for keystroke during run
#define KEY_RELEASE_DURATION 800

//Video Definitions

//Video buffer for 32 x 64 resolution
#define VIDEO_RAM_SIZE 256

//End of Frame buffer cycles
// Original end of frame had time 941 2-cycle instructions  
#define END_BUFFER_CYCLES 1882
/*     
 *  There are about 27 interrupts per second in the pixie video
 *  simulation rather than the original 61. So scale any program
 *  timing constants by multiplying by 0.44.  For example, change
 *  the byte 0x3D (61) to 0x1B (27) at address 0x0049 in the
 *  TV Digital Clock Program 7.6, in Tom Pittman's 
 *  Short Course in Programming.
 */

// Video logic states
#define DISPLAY_BEGIN 0
#define DISPLAY_INT   1
#define DISPLAY_DMA   2
#define DISPLAY_END   3

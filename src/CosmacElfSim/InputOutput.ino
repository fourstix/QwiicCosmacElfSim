/*
 * All input and output routines for the LED backpack, the Qwiic
 * Keypad and the Arduino Serial monitor.
 */

#ifdef QWIIC_KEYPAD 
  #include "SparkFun_Qwiic_Keypad_Arduino_Library.h" 
#endif

#ifdef LED_BACKPACK
  #include "Adafruit_LEDBackpack.h"
#endif 

#ifdef I2C_FRAM
  #include "Adafruit_FRAM_I2C.h"  
  //Setup Fram for saving and restoring Ram between sessions
  //Fram is unfortunately a bit slow to be used as Ram, so we cache pages.
  Adafruit_FRAM_I2C fram = Adafruit_FRAM_I2C();
  const int page_count = 0x80; //128 256-byte pages in 32K FRAM
#else
  const int page_count = 1; //Only one page available without Fram
#endif
 
//Current page of ram
byte current_page = 0x00;

//Indicates if page needs to be saved
boolean ram_dirty = false;

// used to continuously hold the input key 
boolean hold_input = false; 
// used to confirm a destructive action
boolean confirmed = false;  
// used to indicate a shift on the keypad
boolean shifted = false;    
// used to indicate cpu reset after clear or initial start
boolean cpu_reset = true;   
int input_down = 0;  // used to hold input key for typical keystroke
int input_up = 0;    // used to release input key for typical keystroke

#ifdef QWIIC_KEYPAD
  //Define keypad
  KEYPAD qwiicKeypad; 
#endif

#ifdef LED_BACKPACK
  //Setup 4 digit hex display A1 A0 : D1 D0
  Adafruit_7segment sevenseg = Adafruit_7segment();
#endif

//Method called by main setup 
void setupInOutDevices() {
#ifdef QWIIC_KEYPAD
  qwiicKeypad.begin();
#endif

#ifdef LED_BACKPACK
  //Setup the Seven Segment Status Display
  sevenseg.begin(0x70);
  blankBackpack();
#endif

#ifdef I2C_FRAM
  fram.begin();
#endif
} // setupInOutDevices

//Show status on all displays, Serial, LED backpack or LCD.
void displayStatus() {
#ifdef LED_BACKPACK  
  printBackpackStatus();
#endif  
  if (verbose_status) {        
     printVerboseStatus();
  } else {
     printCompactStatus();
  } // if-else verbose_status
}

// Show Menu of commands on Serial monitor
void showMenu() {
  Serial.println(F(" L = Load code"));
  Serial.println(F(" R = Run"));
  Serial.println(F(" X = eXit load or run mode and reset."));  
  Serial.println(F(" M = toggle Memory Protect"));
  Serial.println(F(" J = Jump to address in load mode."));
  Serial.println(F(" W = stop and Wait for run to resume"));
  Serial.println(F(" S = debug toggle: Single-step in wait/run mode"));
  Serial.println(F("     - back up one byte, in load mode"));
  Serial.println(F(" 0-F = hex digit for code input"));
  Serial.println(F(" #,I = Input data to memory during load mode"));
  Serial.println(F("     - Multi-byte input load eg: 00,01,02,03#"));  
  Serial.println(F("     - simulate an Input button press in run mode")); 
  Serial.println(F(" H = Hold down/release input key"));
  Serial.println(F(" Q = Query cpu and register status"));
  Serial.println(F(" O = toggle Output verbose / compact"));
  Serial.println(F(" ? = show ram data")); 
  Serial.println(F(" ~ = erase Elf ram"));
  Serial.println(F(" Z = clear all registers to Zero and enable interrupts"));
  Serial.println(F(" V = Video on/off"));  
  Serial.println(F(" K = blanK video ram and display"));
  Serial.println(F(" T = Transfer rom data to video ram")); 
  Serial.println(F(" P = Paint video ram to display"));
  Serial.println(F(" ! = show video data"));  
  Serial.println(F(" $ = draw video data as A$CII art"));  
  Serial.println(F(" W,N,Y = shoW meNu Yet again")); 
#ifdef I2C_FRAM
  Serial.println(F(" G = Get ram data from fram page")); 
  Serial.println(F(" U = Upload ram data into fram page")); 
  Serial.println(F(" % = show fram page"));
  Serial.println(F(" ^ = compare ram data to fram page"));
  Serial.println(F(" & = print fram page status"));
#endif
}
/*
 * Seven Segment Display routines
 */
#ifdef LED_BACKPACK
//Blank the Status display 
void blankBackpack() {
  sevenseg.writeDigitRaw(0, 0x00);  
  sevenseg.writeDigitRaw(1, 0x00);
  sevenseg.drawColon(false);
  sevenseg.writeDigitRaw(3, 0x00);
  sevenseg.writeDigitRaw(4, 0x00);
  //Update display
  sevenseg.writeDisplay();
} //blankBackpack

// Show status on 4 digit Seven segment display
void printBackpackStatus() {
  byte hex_digit = 0x00;

  if (run_code) {
    // if we are running, blank out the first two digits
    sevenseg.writeDigitRaw(0, mem_protect ? 0x80 : 0x00);  
    sevenseg.writeDigitRaw(1, single_step ? 0x80 : 0x00);    
    //Don't show the colon unless idle (colon is in location 2)
    sevenseg.drawColon(cpu_idle);

    //Print first hex digit of databus byte with load flag as decimal
    hex_digit = (databus >> 4) & 0x0F;
    sevenseg.writeDigitNum(3, hex_digit, load_code);

    //Print second hex digit of databus byte with run flag as decimal
    hex_digit = databus & 0x0F;
    sevenseg.writeDigitNum(4, hex_digit, run_code);    
  } else {
    //Print first address byte with memory protect flag as decimal point
    hex_digit = (address >> 4) & 0x0F;
    sevenseg.writeDigitNum(0, hex_digit, mem_protect);
  
    //Print second address byte with single-step as decimal point
    hex_digit = address & 0x0F;
    sevenseg.writeDigitNum(1, hex_digit, single_step);
  
    //In all other modes draw the colon
    sevenseg.drawColon(true);

    //Print first hex digit of databus byte with load flag as decimal
    hex_digit = (databus >> 4) & 0x0F;
    sevenseg.writeDigitNum(3, hex_digit, load_code);

    //Print second hex digit of databus byte with run flag as decimal
    hex_digit = databus & 0x0F;
    sevenseg.writeDigitNum(4, hex_digit, run_code);
  } // if-else run_code
  
  //Update the display with changes
  sevenseg.writeDisplay();   
} //printBackpackStatus
#endif

/*
 * Keypad utilities 
 */
#ifdef QWIIC_KEYPAD
//Check the Keypad int pin and returns true
//if a key is ready to be read from the keypad
boolean buttonPressed() {
  //Ready line goes low when a key is pressed
  return !digitalRead(KEY_RDY_PIN);
}

//Check the keypad and get a charcter if button was pressed
char checkKeypad() {
  //Set character to invalid value
  char button = NO_KEY_SPACE;

  // Check ready line and get the last key pressed
  if (buttonPressed()) {
    //Put key press in buffer and read it
    qwiicKeypad.updateFIFO();
    button = qwiicKeypad.getButton();

    if (button > 0) {
      if (shifted) {
        button = shiftKey(button);
      } else if (button == '*') {
        //First asterisk is shift
        shifted = true;
      } // if-else shifted
    } else {
      //for anything else must return a space
      button = NO_KEY_SPACE;
    } // if-else button > 0
  }  // if buttonPressed()
  return button;
} // checkKeypad

//Handle shifted key
char shiftKey(char key) {
  char shifted_key = NO_KEY_SPACE;
  switch (key) {
    //Shift 1-6 is Hex A-F
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':      
      shifted_key = 'A' + key - '1';
    break;
    
    //Shift 7 is Load
    case '7':
      shifted_key = 'L';
    break;
    
    //Shift 8 is Run / Wait
    case '8':
      if (run_code) {
        shifted_key = 'W';
      } else {
        shifted_key = 'R';   
      } // if-else run_code
    break;

    //Shift 9 is Memory Protect toggle
    case '9':
      shifted_key = 'M';
    break;

    //Shift * is debug   
    case '*':
      // back up in load mode, single-step toggle in run/wait
      shifted_key = 'S';
    break;
    
    //Shift 0 is Clear
    case '0':
      shifted_key = 'X';
    break;

    //Shift pound toggles Hold/Release Input key
    case '#':
      shifted_key = 'H';
    break;

    default:
      //Ignore anything else
    break;    
  } // switch key
  
  //We're done shifting so clear the flag and return the shifted key
  shifted = false;
  return shifted_key;
}
#endif

/*
 *  Serial Input
 */
// Properly print two hex digits into array
void print2hex(uint8_t v) {  
  //If single Hex digit
  if (v < 0x10) {
   Serial.print(F("0"));
  } // if v < 0x10
  Serial.print(v, HEX);
}


//properly print four hex digits to Serial
void print4hex(uint16_t v)
{
  print2hex(v >> 8);
  print2hex(v);
}


// print the state based on flags to Serial
void printState() {
  if (cpu_idle) {
    Serial.print(F(" Idle "));
  } else if (run_code) {
    Serial.print(F(" Run "));
  } else if (load_code) {
    Serial.print(F(" Load "));
  } else if (single_step) {
    Serial.print(F(" Step "));
  } else if (cpu_reset) {
    Serial.print(F(" Init "));
  } else{
    Serial.print(F(" Wait "));
  } // if-else
} // printState

void printCompactStatus() {
  Serial.println();
  print4hex(address);
  Serial.print(F(":"));
  print2hex(databus);
  printState();
  if (mem_protect) {
    Serial.print(F("P "));
  } else {
    Serial.print(F("M "));
  } // if-else
  if (ie) {
    Serial.print(F("IE"));
  } else {
    Serial.print(F("DI"));
  } // if-else 
  //Next line 
  Serial.println();
  Serial.print(F("Q"));
  printFlag(q_reg);
  Serial.print(F(" X"));
  Serial.print(x_reg, HEX);  
  Serial.print(F(" P"));
  Serial.print(p_reg, HEX);
  Serial.print(F(" D="));
  print2hex(d_reg);
  Serial.print(F(" DF"));
  printFlag(df);
  //Show Carry/Borrow status (DF)
  if (df) {
    Serial.print(F(" C "));  
  } else {
    Serial.print(F(" B "));
  } // if df
  if (ef4) {
    Serial.print(F("#"));
  } 
  if (video) { 
    Serial.println();
    Serial.print(F("EF1="));
    printFlag(ef1);
    Serial.print(F(" IRQ="));
    printFlag(irq);    
    if (isDmaActive()) {
      Serial.print(F(" DMA="));
      Serial.print(getDmaCount());
    } // dma_count
  } // if video
      
 Serial.println();
}


//print the value of a flag as one or zero
void printFlag(boolean flag) {
  if (flag) {
    Serial.print(F("1"));
  } else {
    Serial.print(F("0"));
  } // if flag
} // printFlag

//print the boolean value of a flag as on or off
void printOnOff(boolean flag) {
  if (flag) {
    Serial.print(F("on"));
  } else {
    Serial.print(F("off"));
  } // if flag
} // printOnOff

// Print Complete status
void printVerboseStatus() {
  printCompactStatus();
  Serial.print(F("T="));
  Serial.print(t_reg, HEX);  
  Serial.print(F(" EF1="));
  printFlag(ef1);
  Serial.print(F(" EF2="));
  printFlag(ef2); 
  Serial.print(F(" EF3="));
  printFlag(ef3);
  Serial.print(F(" EF4="));
  printFlag(ef4);
      
  for (int i = 0; i < 16; i++) {
    if (i % 4 == 0) {
      Serial.println();
    } // if 
    Serial.print(F("R"));
    Serial.print(i, HEX);
    Serial.print(F(": "));
    print4hex(reg[i]);
    Serial.print(F(" "));     
  } // for
  Serial.println();
  Serial.print(F("Video: "));
  printFlag(video);
  Serial.print(F(" Machine cycle: "));
  Serial.print(cycle);
  Serial.print(F(" IRQ: "));
  printFlag(irq);
  Serial.println();
  Serial.print(F("Pixie State: "));
  Serial.print(getPixieState());
  Serial.print(F(" DMA count: "));
  Serial.print(getDmaCount());
  Serial.print(F(" Wait: "));
  printFlag(isPixieWait());  
  Serial.println();  
}

// Pretty print a block of hex bytes
void printBlockHex(byte data, uint16_t addr) {
  if ((addr % 8) == 0) {
      Serial.print(F("\n ")); 
      print4hex(addr);
      Serial.print(F(": "));
    } // if
    print2hex(data);
    Serial.print(F(" "));
}

// Show all 256 bytes of ram
void dumpRam() {
  uint8_t value;
  for (uint16_t a = 0; a < ELF_RAM_SIZE; a++) {
    value = elf_ram[a];
    printBlockHex(value, a);
  } // for
  Serial.println();
} // dumpRam



// Show 256 bytes of video ram
void dumpVideoRam() {
  uint8_t value;
  for (uint16_t a = 0; a < ELF_RAM_SIZE; a++) {
    value = getVideoByte(a);
    printBlockHex(value, a);
  } // for
  Serial.println();
} // dumpVideoRam

#ifdef I2C_FRAM
  //Load the 256 bytes of elf ram with data from a fram page,
  //using 32 byte buffer. Returns number of bytes read.
  int loadRamFromFram(int page) {  
    int i = 0;
    uint16_t framAddr = page * ELF_RAM_SIZE;  
    
    for (i = 0; i < ELF_RAM_SIZE; ) {
      Wire.beginTransmission(FRAM_ADDR);
      Wire.write(framAddr >> 8);
      Wire.write(framAddr & 0xFF);
      Wire.endTransmission();    
      Wire.requestFrom(uint8_t(FRAM_ADDR), 32);
      uint8_t j;
      for (j = 0; j < 32 && i < ELF_RAM_SIZE; i++, j++) {
        if(!Wire.available()) {
          return i;
        }
        elf_ram[i] = Wire.read();
      }
      framAddr += j;
    } // for i
    return i;
  } // loadRamFromFram

  //Save elf ram to Fram page, using 30-byte buffer
  //Returns number of bytes written.
  int saveRamToFram(int page) {
    int i = 0;
    uint16_t framAddr = page * ELF_RAM_SIZE;
    
    for (i = 0; i < ELF_RAM_SIZE; ) {
      Wire.beginTransmission(FRAM_ADDR);
      Wire.write(framAddr >> 8);
      Wire.write(framAddr & 0xFF);
      uint8_t j;
      for (j = 0; j < 30 && i < ELF_RAM_SIZE; i++, j++) {
        Wire.write(elf_ram[i]);
      } // for j
      Wire.endTransmission();
      framAddr += j;
    } // for i
    return i;
  } //saveRamToFram
   
  //Show the contents of a Fram page
  void dumpFramPage(int page) {
    uint8_t value;
    Serial.print(F("Ram page "));
    print2hex(page);
    Serial.print(F(": 0x"));
    print4hex(page * ELF_RAM_SIZE);
    Serial.println();
    for (uint16_t i = 0; i < ELF_RAM_SIZE; i++) {
      uint16_t addr = page*ELF_RAM_SIZE + i;
      value = fram.read8(addr);
      printBlockHex(value, addr);
    } // for
    Serial.println();
  } // dumpRam

   
  //Compare contents of Fram page to ram data
  boolean compareFramPage(int page) {
    uint8_t value;
    //Assume true and prove false
    boolean match = true;
    for (uint16_t i = 0; i < ELF_RAM_SIZE; i++) {
      uint16_t addr = page*ELF_RAM_SIZE + i;
      value = fram.read8(addr);
      //If we find a mis-match clear the flag and exit
      if (value != elf_ram[i]) {        
        match = false;
        break;
      } // if
    } // for
    return match;    
  } // compareFramPage

//Print the current page status, dirty or clean, to Serial
void printPageStatus() {
  Serial.print(F("Page "));
  print2hex(current_page);
  Serial.print(F(" is "));
  if (ram_dirty) {
    Serial.println(F("dirty"));
  } else {
    Serial.println(F("clean"));
  } // if-else ram_dirty
} // printPageStatus
  

//Check the page and load a new one if required
//Return true if new page was loaded  
boolean checkPage(uint16_t addr) {
  boolean result = false;
  byte new_page = (addr & 0x7F00) >> 8;

  //If we have crossed a page boundary,
  //save it and load the new page
  if (new_page != current_page) {
    if (verbose_status) {
      Serial.print(F("New Page: 0x"));
      print2hex(new_page);
      Serial.print(F(" Old Page: 0x"));
      print2hex(current_page);
      Serial.println();
      printPageStatus();
    } // if verbose_status
    //Memory protect will prevent cache writes to FRAM as well
    if (ram_dirty && !mem_protect) {
     saveRamToFram(current_page);
    } // if ram_dirty
    
    loadRamFromFram(new_page);
    current_page = new_page;
    //New page is clean until data is written to it
    ram_dirty = false;
    result = true;
  } // if new_page != page
  
  return result;
} // checkPage

#else 
//If no FRAM, then there is only one page
boolean checkPage(uint16_t addr) {
  return false;
} // checkPage

#endif

//Check to see if address points to valid ram
boolean isRamAddress(uint16_t addr) {
  return (addr < page_count * ELF_RAM_SIZE);
}

//Read a byte from Ram
void writeByteToRam(uint16_t addr, uint8_t data) {
  uint16_t offset = addr & 0x00FF;
  
  checkPage(addr);
  elf_ram[offset] = data;
  //If we write to a page, set the dirty flag
  ram_dirty = true;  
} // writeByteToRam

byte readByteFromRam(uint16_t addr) {
  uint16_t offset = addr & 0x00FF;
  checkPage(addr);
  return elf_ram[offset];  
} // readByteFromRam

//Confirm the chosen action by asking for a yes or no
boolean confirmChoice() {
    boolean result = false;
    Serial.println(F("Are you sure (y/n)?"));
    while (true) {
      while (! Serial.available());  // wait for characters
      char c = Serial.read();  // read a character 
      if (isAlpha(c)) {   
          if (c == 'y' || c == 'Y') {
            result = true;
          } // if c == y
          break;  // break out of reading loop
      }  // if isAlpha
    } // while reading characters
  if (!result) {
    Serial.println(F("Canceled."));
  } // !result
  return result;
} // confirm choice

//Show a completed message after action
void completedMsg() {
  Serial.println(F("Done."));
}
/*
 * Get a new address to load code at
 */
uint16_t getNewAddress() {
  Serial.println();
  Serial.println(F("Enter new address byte in hex."));
  return readSerialNumber();
}


/*
 * Get a page Number, prompt if needed
*/
int getPageNumber() {
  int result = 0;
  if (page_count > 1) {
    Serial.println();
    Serial.print(F("Enter rom page number 0 to "));
    print2hex(page_count - 1);
    Serial.println();
    result = readSerialNumber();
  } // if page_count > 1
  return result;
} // getPageNumber


/*
 * Read a number until a newline or other character is found
 */
uint16_t readSerialNumber() {
    uint16_t value = 0;
    boolean found = false;
    while (true) {
    while (! Serial.available());  // wait for characters
    char c = Serial.read();  // read a character    
    if (isHexadecimalDigit(c)) {
      value *= 16;  //move hexadecimal place for previous digits
      value += getHexValue(c);  // add numeric value
      found = true;
    } else if (found) {
      break;
    } // else - if is digit
  } // while reading Serial input
  return value;
}  // readSerialNumber

byte getHexValue(char d) {
  byte value = 0x00;
  // check to see what range value is in
  if (isDigit(d)) {
    value = d - '0';   
  } else {
      value = 10 + toupper(d) - 'A';
  } // if else
  return value;
} // getHexValue

//Process a character as a hex digit 
void processHexChar(char h) {
  keybuffer = (keybuffer << 4) & 0x00F0;
  keybuffer |= getHexValue(h);
  //databus is updated any time a key is pressed
  databus = keybuffer;
} //processHexChar

// Check for serial input 
char checkSerial () {
  char cs = NO_KEY_SPACE;  
     if (Serial.available()) {
    // read a character   
    cs = Serial.read();
    } // if Serial.available
    return cs;
} // checkSerial

//Process a character from either the keypad or serial monitor        
void processChar(char c) {
  //Process a character from either the keypad or serial monitor
  switch(c) {
    //Load
    case 'L':
    case 'l':
      load_code = true;
      run_code = false;
      //End the input counts
      input_down = 0;
      input_up = 0;
      //Load mode always causes a Reset
      reset1802();  
      //End cleared condition
      cpu_reset = false;
      update_display = true;
    break;

    //Jump to address
    case 'J':
    case 'j':
      //Jump is only valid during load
      if (load_code) {
        //Set the address register to input value
        reg[p_reg] = getNewAddress();
        //Force an update
        address = reg[p_reg];
        databus = readByte(address); 
        update_display = true;
      } // if load_code
    break;
    
    //Clear
    case 'X':
    case 'x':
      load_code = false;
      run_code = false;
      //End the input counts
      input_down = 0;
      input_up = 0;
      reset1802();
      //Indicate cpu cleared
      cpu_reset = true;
      update_display = true;
    break;

    //Run
    case 'R':
    case 'r':        
      run_code = true;
      load_code = false;
      //End cpu reset condition
      cpu_reset = false;
      //Don't update the display when single stepping
      if (!single_step) {       
        update_display = true;
      } // if !single_step
    break; 

    //Wait
    case 'W':
    case 'w':
      run_code = false;
      load_code = false;
      //End cleared condition
      cpu_reset = false;
      update_display = true;
    break; 

    //Memory Protect
    case 'M':
    case 'm':
      // Memory protect state is shown on the display
      mem_protect = !mem_protect;
      update_display = true;
    break;      

    //Step
    case 'S':
    case 's':
      //Debug    
      // In load mode, back up one address
      if (load_code && address > 0) {      
        // Back up one byte and display it
        reg[p_reg]--;
        address = reg[p_reg];
        databus = readByte(address);
        update_display = true;   
      } else {     
      //Otherwise toggle single step in run/wait states
      single_step = !single_step;
      Serial.print(F("Single step mode is "));
      printOnOff(single_step);
      Serial.println();
      update_display = true;   
      } // if-else load_code
    break;

    //Hold/Release Input button
    case 'H':
    case 'h':
      //Hold button down until toggle for release

      //Hold-Release overrides a press
      input_down = 0;
      input_up = 0;
  
      hold_input = !hold_input;
      ef4 = hold_input;
      Serial.print(F("The input key is now "));
      Serial.println( hold_input ? F("held down.") : F("released."));        
    break;      

    //Input
    case 'I':
    case 'i':
    case ',':
    case '#':
      //Set ef4 true for at least one cycle for input
      ef4 = true;
      //In run mode, if we're not holding the button down already
      //simulate an input button keystroke by holding ef4 true for awhile.
      if (run_code && !hold_input) {
        //Assert flag for long enough to see brief display change
        //Need to simulate Input down then up during a run
        input_down = KEY_PRESS_DURATION;
        input_up = KEY_RELEASE_DURATION;
      } // if run_code && !hold_input
    break;   
      
    //Output verbose/compact
    case 'O':
    case 'o':
      Serial.print(F("Verbose output is "));      
      verbose_status = !verbose_status;
      printOnOff(verbose_status);
      Serial.println();
    break;

    //Query Status
    case 'Q': 
    case 'q':      
      //Show complete status
      printVerboseStatus();     
    break;
      
    //Print Ram
    case '?':
      Serial.println(F("Elf ram:"));
      dumpRam();
      Serial.println(); //pretty print
    break;
    
    case 'Z':
    case 'z':
      // Should only do this if in load or wait (clear) mode      
      if (!run_code) {
        Serial.println(F("Clear all registers and enable interrupts."));
        confirmed = confirmChoice();
        if (confirmed) {
          initReg();
          completedMsg();
        } // if confirmed
      } // if !run_code
    break;

#ifdef I2C_FRAM
    //Load elf ram from fram
    case 'G':
    case 'g':
      // Should only do this if in load or wait (clear) mode      
      if (!run_code) { 
        byte page = getPageNumber();
        if (page >= 0 && page < page_count) {
          Serial.print(F("Load elf ram with data from fram page 0x"));
          print2hex(page);
          Serial.println();
          confirmed = confirmChoice();

          if (confirmed) {
            //Check in case we are on another page
            checkPage(0);
            loadRamFromFram(page);
            //Mark it dirty since it has been written
            ram_dirty = true;
            completedMsg();
          } // if confirmed
        } // if page valid
      } // if !run_code
    break;     
    case 'U':
    case 'u':      
      // Should only do this if in load or wait (clear) mode
      if (!run_code) {
        byte page = getPageNumber();
        // Make sure we have a valid page number
        if (page >= 0 && page < page_count) {
          Serial.print(F("Save elf ram data into fram page 0x"));
          print2hex(page);
          Serial.println();
          confirmed = confirmChoice();  
          if (confirmed) {
            saveRamToFram(page);
            update_display = true;
            completedMsg();
          } // if confirmed
        } // if page valid
      } // ! run_code
    break;

    case '%':
      // Should only do this if in load or wait (clear) mode
      if (!run_code) {
        byte page = getPageNumber();
        // Make sure we have a valid page number
        if (page >= 0 && page < page_count) {
          dumpFramPage(page);
        } // if page valid
      } // ! run_code
    break;
      
    case '^':
      // Should only do this if in load or wait (clear) mode
      if (!run_code) {
        byte page = getPageNumber();
        boolean match = false;
        
        // Make sure we have a valid page number
        if (page >= 0 && page < page_count) {
          match = compareFramPage(page);        
          Serial.print(F("Fram Page 0x"));
          print2hex(page);
          Serial.print(F(" and ram data "));
          if (!match) {
            Serial.print(F("do NOT "));
          } // if !matc
          Serial.println(F("match."));
        } // if page valid
      } // ! run_code
    break;
    
    case '&':
      printPageStatus();
      break;
#endif

    //Video on/off
    case 'V':
    case 'v':      
      if (video) {
        stopVideo();          
      } else {
        startVideo();
      }
      Serial.print(F("Video is now "));
      printOnOff(video);
      Serial.println();
    break;

    //Clear video ram
    case 'K':
    case 'k':      
      Serial.println(F("Clear video ram"));
      confirmed = confirmChoice();
      if (confirmed) {
        clearVideoRam();
        paintDisplay();
        completedMsg();
      } // if confirmed
    break;

    //Transfer Rom to Video Ram
    case 'T':
    case 't':      
      // Should only do this if in load or wait (clear) mode
      if (!run_code) {
        Serial.println(F("Transfer rom data into video ram"));
        confirmed = confirmChoice();  
        if (confirmed) {
          loadVideoFromRom();
          completedMsg();
        } // if confirmed
         } // ! run_code
    break;
    //Paint video display    
    case 'P':
    case 'p':      
      paintDisplay();
      Serial.println(F("Display painted with video data."));
    break;

    //Show video ram bytes  
    case '!':
      Serial.println(F("Video ram:"));
      dumpVideoRam();
      Serial.println(); //pretty print
    break;   

    //Draw video ram as ASCII art
    case '$':      
      drawVideoScreen();      
    break; 

    //Clear Elf ram
    case '~':
      // Should only do this if in load or wait (clear) mode
      if (!run_code) {
        Serial.println(F("Erase all ram data"));
        confirmed = confirmChoice();
        if (confirmed) {
          //Check in case we are on another page
          checkPage(0);
          eraseRamMemory();
          //Mark it dirty since it was written
          ram_dirty = true;
          completedMsg();
        } // if confirmed
      } // if !run_code
    break;

    //Process Hex character or show menu
    default:         
      if (isHexadecimalDigit(c)) {          
         processHexChar(c);
         if (load_code) {
         update_display = true;
         } // load_code
      } else if (isAlpha(c)) {
        showMenu();          
       }// if
       break;
  } // switch    
} //checkSerial

//Process keypad and Serial input
void processKeyInput() {
  char c_input = NO_KEY_SPACE; 
  //In run mode we have to simulate an input key down then key up
  //since many programs check for both
  if (run_code && input_down > 0) {  
    //In run mode, simulate input key press while counting down
    input_down--;          
    ef4 = true;
    //Don't process any other keystrokes yet, to avoid over-run
    return;       
  } else if (run_code && input_up > 0) {  
    //In run mode, simulate input key release while counting down
    input_up--;          
    ef4 = false;
    //Don't process any other keystrokes yet, to avoid over-run
    return; 
  } else if (!hold_input) {
    // clear the input flag if we aren't holding it down or simulating keypress    
    ef4 = false;
  } // if-else if press_input

  //Check keypad first
#ifdef QWIIC_KEYPAD
  c_input = checkKeypad();
#endif  

  // if no keypad input, check Serial
  if (isSpace(c_input)) {    
    c_input = checkSerial();
  } // if isSpace
  
  processChar(c_input);  
} // processKeyInput

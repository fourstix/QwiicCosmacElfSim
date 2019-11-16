/*
 * Simulation of Pixie Video using a 128 x 64 graphic display.     
 * 
 * There are about 27 interrupts per second in the pixie video
 * simulation rather than the original 61. So scale any program
 * video timing constants by multiplying by 0.44.  For example,
 * change the byte 0x3D (61) to 0x1B (27) at address 0x0049 in the
 * TV Digital Clock Program 7.6, in Tom Pittman's Short Course in 
 * Programming to get a more accurate seconds counter.
 */
#ifdef U8G2_DISPLAY
#include <U8g2lib.h>

// The Complete list of u8g2 constructors is available here:
// https://github.com/olikraus/u8g2/wiki/u8g2setupcpp

//Define generic SSD1306 I2C 128x64 OLED display
//U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//Generic SH1106 I2C  128x64 OLED  display
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

//This is an old KS0108 Monchron display I had lying around from Adafruit.
//It's huge, and takes about 12 pins on the Arduino.  It works just fine 
//and gives a nice retro look.

//KS0108 Monchron - Change Enable from A4 (18) to A2 (16) and tie R/W to GND
//U8G2_KS0108_128X64_1 u8g2(U8G2_R0, 8, 9, 10, 11, 4, 5, 6, 7, /*enable=*/ 16, /*dc=*/ 17, /*cs0=*/ 14, /*cs1=*/ 15, /*cs2=*/ U8X8_PIN_NONE, /* reset=*/  U8X8_PIN_NONE);

//Lookup table for XBM format bitmap conversion
// In XBM format LSB is the left most bit, 
// so we expand each nibble to a byte and flip bits
const uint8_t lookup_xbm[16] PROGMEM {
  0x00, 0xC0, 0x30, 0xF0, 0x0C, 0xCC, 0x3C, 0xFC,
  0x03, 0xC3, 0x33, 0xF3, 0x0F, 0xCF, 0x3F, 0xFF};

#endif

//Video state
byte pixie_state = DISPLAY_BEGIN;

//Video wait flag, true after display painted and waiting for new frame
boolean pixie_wait = false;

//DMA counter for Pixie video
byte dma_count = 0;

//Redraw flag
boolean redraw = false;

//Pixie video buffer
uint8_t video_ram[VIDEO_RAM_SIZE];

// Each pixie frame took about 1/60 of a second
// These variables can be used for timing the simulation
unsigned long t_frame_start;
unsigned long t_frame_stop;

//Set up Video
// Must be called in setup routine
void setupVideo() {
#ifdef U8G2_DISPLAY    
  //u8g2.setBusClock(400000); // fast I2C mode
  u8g2.begin();
  u8g2.setBitmapMode(false); //non-transparent
  u8g2.setDrawColor(1); // White

#endif  
} // setupVideo

//Start the simulation
void startVideo() {
  pixie_state = DISPLAY_BEGIN;
  video = true;  
  cycle = 0;
  //Always draw display at least once
  redraw = true;  
  pixie_wait = false;
} // startVideo

//Stop the simulation
void stopVideo() {
  //Clear the OLED and reset to start again
#ifdef U8G2_DISPLAY  
  u8g2.clear();
#endif  
  pixie_state = DISPLAY_BEGIN;
  video = false;
  lowerInterrupt();
  cycle = 0;
  redraw = false;
  pixie_wait = false;
} // stopVideo

// Load video buffer with ROM data
void loadVideoFromRom() {
  for (int i = 0; i < VIDEO_RAM_SIZE; i++) 
    video_ram[i] = readByteFromRom(ROM_ADDR + i);
} // loadVideoRom 



// Load video buffer with zeros
void clearVideoRam() {
  for (int i = 0; i < VIDEO_RAM_SIZE; i++) 
    video_ram[i]= 0x00;
} // clearVideoRam

// Paint entire display
void paintDisplay() {
#ifdef U8G2_DISPLAY  
  u8g2.firstPage();  
  do {
    drawDisplay();
  } while( u8g2.nextPage() );
#else
  //Draw display as ASCII art to serial port, kinda slow 
  drawVideoScreen();
#endif
}

// Clear video buffer and display.
void blankDisplay() {
  clearVideoRam();  
  paintDisplay();
} // blankVideo

//Print the screen as ASCII art to serial, rather slow
void drawVideoScreen() {
  Serial.println();
  //Print border at top of display
  Serial.print(F("+"));
  for (int i = 0; i < 64; i++) {
    Serial.print(F("-"));
  } //for
  Serial.println(F("+"));
  for (int addr = 0; addr < VIDEO_RAM_SIZE; addr++) {
    byte data = getVideoByte(addr);
    
     //Print border at beginning of each new line
     if (addr % 8 == 0) {
      Serial.print(F("|"));
     } // if
    //Print out each bit in byte 
    for (int i = 0; i < 8; i++) {
      //Check MSB and print it     
      if (data & 0x80) {
        Serial.print(F("#"));
      } else {
        Serial.print(F(" "));  
      } // if MSB bit is true
      data <<= 1;
    } // for each bit
     //Print border at end of each line
     if (addr % 8 == 7) {
      Serial.println(F("|"));
     } // if
  } // for addr
  //Print border at bottom of display
  Serial.print(F("+"));
  for (int i = 0; i < 64; i++) {
    Serial.print("-");    
  } //for
  Serial.println(F("+"));  
} //drawVideoScreen 

#ifdef U8G2_DISPLAY  
//Draw the entire display
void drawDisplay() {
  for(int row = 0; row < 32; row++) {
    byte raster_data[16];
 
    //Fill line with expanded data
    for (int i = 0; i < 8; i++) {
      //Get a byte from the video data and transform it for display    
      byte b = video_ram[row * 8 + i];
      byte hi = (b >> 4) & 0x0F;
      byte lo = b & 0x0F;
  
      // For XBM LSB is the left most, so we expand to byte and flip bits
      //raster_data[2*i]   = lookup_xbm[hi];
      //raster_data[2*i+1] = lookup_xbm[lo];    
      raster_data[2*i]   = pgm_read_byte_near(lookup_xbm + hi);
      raster_data[2*i+1] = pgm_read_byte_near(lookup_xbm + lo);  
    } // for i
  
    //Draw line twice for 32 x 64 resolution
    u8g2.drawXBM(0, 2*row, 128, 1, raster_data);
    u8g2.drawXBM(0, 2*row+1, 128, 1, raster_data);
  
  } // for row < 32
}  //drawDisplay
#endif

//Main routine to simulate the pixie video
void runPixieVideo() {
  // We are at the end of an instruction cycle, before processing next
  switch(pixie_state) {
    case DISPLAY_BEGIN:
     //First time through begin display logic
      if (!ef1) {
        // Set timestamp for frame timing
        t_frame_start = millis();
      } // if cycle
      
      //assert ef1 for 31 cycles before interrupt occurs
      ef1 = true;
      if (cycle >= 31) {
      //change state, raise interrupt and leave ef1 on
      pixie_state = DISPLAY_INT;
      cycle = 0;
      // Make an interrupt request
      raiseInterrupt();
      } // if cycle >= 31
    break;
    /**********************************************************
     * This is a bit of a hack.  Some programs track ef1
     * and depend on the fact that ef1 is asserted for 56
     * cycles or 28 2-cycle instructions before the first dma, 
     * and at the end of ef1  there is exactly 4 cycles or two 
     * 2-cycle instructions) before the first dma begins.  
     * Then on the last 4 lines of the display, ef1 is asserted 
     * for 6 cycles per line, for 24 cycles or 12 two-cycle 
     * instructions total.
     * 
     * See Programs 7.2 and 7.3 in Tom Pittman's
     * A Short Course in programming.
     * 
     * Other programs, like the classic Spaceship program 
     * depend on the fact that the interrupt request lasts
     * exactly 29 cycles before the first DMA, and use ef1
     * only to determine when DMA is ending.
     * 
     * This logic simulates both cases.   Since we aren't using
     * S0 and S1 cycles, we fake it by ending ef1 a bit early.
     */
    case DISPLAY_INT:
      // Drop ef1 at end of last line (4 cycles before dma)
      if (cycle >= 25) {
        ef1 = false;
      }
      // interrupt held for *exactly* 29 cycles before dma
      if (cycle >= 29) {
        lowerInterrupt();
        ef1 = false;
        pixie_state = DISPLAY_DMA;
        cycle = 0;
        dma_count = 0;
        
        // The first DMA request happens immediately afterwards
        dmaVideoRequest();
      } // if cycle >= 29
    break;

    /*
     * Continue DMA requests until video buffer loaded
     */
    case DISPLAY_DMA:
      // Pixie video continues the DMA requests to fill the video frame
      dmaVideoRequest();
    break;

    /*
     * Paint the display, then wait for the next frame to start again.
     */
    case DISPLAY_END:
      /*
       * This is where the time goes.  Painting the display
       * takes about 3x as long as the original Pixie video. 
       * So there are about 20 video interrupts per second 
       * instead of 61.
       * 
       * This means a timer constant in a clock program, would
       * need to be reduced to a third of it's original value.
       */      
      //first time through end of frame logic
      if (!pixie_wait) {
          //Check to see if we need to redraw the display
        if (redraw) {          
  #ifdef U8G2_DISPLAY            
          u8g2.clear();
  #endif
          redraw = false;          
       
          //If not using tiles, paint the entire display 
          paintDisplay();
          
        } //if redraw           
      } // if !pixie_wait
          
        
      // Set a flag to indicate we are done, and waiting at the frame end
      pixie_wait = true;

      /*     
       * There are about 20 interrupts per second rather than
       *  rather than the original 61. So scale any program timing 
       *  constants by dividing by 3.  For example, change
       *  the byte 0x3D (61) to 0x1B (14) at address 0x0049 in the
       *  TV Digital Clock Program 7.6, in Tom Pittman's 
       *  Short Course in Programming.
       */
      // Wait until enough time has passed for 600 instructions      
      if (cycle >= END_BUFFER_CYCLES) {   
        pixie_state = DISPLAY_BEGIN;
        cycle = 0;
        pixie_wait = false;
        t_frame_stop = millis();         
        //Serial.println(t_frame_stop - t_frame_start);                         
      } // if cycles      
    break;
    
    //Invalid video state
    default:
      //Something went wrong, so halt
      video = false;
      run_code = false;  
      Serial.print(F("Error: Unknown video state "));  
      Serial.println(pixie_state);
    break;    
  } // switch

} // pixie_video


// Check the DMA count to see if this is a real or fake DMA request
boolean isRealRequest(int n) {
  // Only the last line of each 4 line block is written to video memory
  return (n % 4 == 3);
}
/*
 * After the interrupt request, the pixie video logic will make 128 DMA(0) 
 * requests for 8 bytes, pausing for 3 cycles after each DMA burst.
 * DMA requests steal cycles from the processor, and the pixie hardware
 * uses DMA(0) so the R0 register is incremented to transfer data.
 * 
 * For a 32 x 64 display, programs reset R0 to duplicate the same 8 bytes 
 * for each set of 4 reads, making 32 lines of video data requiring a ram
 * buffer of only 256 Bytes.
 * 
 * To simulate this, this function only copies data into the video ram
 * on the last of each block of 4 DMA reads.  The first, second and third
 * DMA request are ignored, as they duplicated.  
 * 
 * Since there are 3 instruction cycles after each DMA request
 * these means that one data transfer happens every twelve instruction 
 * cycles.  On the last block of four lines, the pixie hardware asserts the
 * ef1 flag to indicate the video buffer is ending.  
 * 
 * Some programs count the ef1 flag to tell whether it's at the beginning
 * or end of the video frame.  There are 28 cycles where ef1 is true before
 * the video frame, but only 12 at the end.
 * 
 */
void dmaVideoRequest() {
  // after 6 cycles reset counter
  if (cycle >= 6) {
    cycle = 0;
  }
  if (cycle == 0) {
    // Do a DMA request 
    if (dma_count < 128) {
      // assert ef1 on last four lines of display
      if (dma_count >= 124) {
        ef1 = true;
      } // if dma_coung >= 124
      //Only write to video buffer on last of block of 4 lines
      if (isRealRequest(dma_count)) {
        // Real DMA request
        int video_line = dma_count / 4;        
        //DMA uses register 0 to transfer 8 bytes of data
        for (int i = 0; i < 8; i++) {
          // video_ram[video_line * 8 + i] = readByte(reg[0]);
          byte new_byte = readByte(reg[0]);
          byte old_byte = video_ram[video_line * 8 + i];
          //If any byte has changed save it and set update flag
          if (new_byte != old_byte) {
            redraw = true;
            video_ram[video_line * 8 + i] = new_byte;
          } // if new_byte != old_byte
          reg[0]++;
        } //for 8 byte line of video data 
      } else {
        // Fake DMA request, just move R(0) 8 bytes 
        reg[0] += 8;
      } // if - else isRealRequest
      // All DMA requests will wake up the cpu
      cpu_idle = false;
      dma_count++;
    } else {
      // After 128 DMA requests, we're done updating the display
      pixie_state = DISPLAY_END;
      ef1 = false;
      pixie_wait = false;
      cycle = 0;
    } // if-else dma_count < 128
  } // if cycle == 0
}

/*
 * Define getter methods used by routines in InputOutput tab
 */

//Returns true if video is waiting after display complete
boolean isPixieWait() {
  return pixie_wait;
 }

//Returns the current state of pixie video simulator state machine
int getPixieState() {
  return pixie_state;
 }

//Return the current value of dma counter
int getDmaCount() {
  return dma_count;
}

//Returns true if dma_count is in range of video processing
boolean isDmaActive() {
  return dma_count >= 0 && dma_count < 128;
}

//Return byte value at address
byte getVideoByte(uint16_t addr) {
  return video_ram[addr];
}

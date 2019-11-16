/*
 * Execute 1802 commands to simulate a cdp1802 chip.  
 * Based heavily on code written by Al Williams, with
 * a few stylistic changes in palces to make it a bit 
 * easier to read.
 * 
 */
void execute1802(uint8_t code) {  
  uint8_t i_reg = (code >> 4) & 0x0F;
  uint8_t n_reg = code & 0x0F;
  uint8_t data = 0; // byte of data from memory
  uint16_t work = 0; //scratch register for arithmetic
  uint16_t target = 0; // target for branches   
  uint16_t next = 0; // next instruction

  //First byte of instruction (I) usually determines the type 
  switch (i_reg) {
    case 0:    
      if (n_reg == 0x00) {
        //IDL - stop running program until DMA or Interrupt
        cpu_idle = true;
        // databus = code;
        update_display = true;
      } else {
      // LDN - load D from Memory pointed to by reg N
      d_reg = readByte(reg[n_reg]);
      }     
      break;

    case 1:
      // INC reg N
      reg[n_reg]++;
    break;

    case 2:
      // DEC reg N
      reg[n_reg]--;
    break;

    case 3:
      // Branches
      target = readByte(reg[p_reg]);
      reg[p_reg]++;
      next = reg[p_reg];
      switch(n_reg) {
        case 0:
          //BR - branch unconditionally
         next = target;
        break;        
        case 1:
          //BQ - branch if Q is 1
          if (q_reg) {
            next = target;
          }
        break;

        case 2:
          //BZ - branch if D is zero
          if (d_reg == 0) {
            next = target;
          }
        break;

        case 3:
          // BDF - branch if DF is 1
          if (df) {
            next = target;
          }
        break;

        case 4:
          //B1 - branch if ef1 is true
          if (ef1) {
            next = target;
          }
        break;

        case 5:
          //B2 - branch if ef2 is true
          if (ef2) {
            next = target;
          }
        break;

        case 6:
          //B3 - branch if ef1 is true
          if (ef3) {
            next = target;
          }        
        break;

        case 7:
          //B4 - branch if ef4 is true     
          if (ef4) {
            next = target;
          }        
        break;

        case 8:
          // SKP - skip one byte
        break;
        
        case 9:
          //BNQ - branch if Q is zero
          if (!q_reg) {
            next = target;
          }
        break;

        case 0x0A:
          //BNZ - branch if D is not zero
          if (d_reg != 0) {
            next = target;
          }
        break;
        
        case 0x0B:
          // BNF - branch if DF is 0
          if (!df) {
            next = target;
          }
        break;
        
        case 0x0C:
          //BN1 - branch if ef1 is false
          if (!ef1) {
            next = target;
          }
        break;

        case 0x0D:
          //BN2 - branch if ef2 is false
          if (!ef2) {
            next = target;
          }
        break;

        case 0x0E:
          //BN3 - branch if ef3 is false
          if (!ef3) {
            next = target;
          }        
        break;

        case 0x0F:
          //BN4 - branch if ef4 is false       
          if (!ef4) {
            next = target;
          }
        break;
      } // switch n_reg
      reg[p_reg] = (reg[p_reg] & 0xFF00) | next;
    break;

    case 4:
      //LDA N - load from Address in Reg(N) and Advance
      d_reg = readByte(reg[n_reg]);
      reg[n_reg]++;    
    break;

    case 5:
      //STR - store D into Address in Reg(N)
      writeByte(reg[n_reg], d_reg);
    break;

    case 6:
      switch (n_reg) {
        case 0:
          // IRX - increament Reg(X)
          reg[x_reg]++;
          update_display = verbose_status;
        break;

        case 1:
          //OUT 1 - turn pixie video off (n = 1)
          if (video) {
            stopVideo();
          } // if !video
          reg[x_reg]++;
          update_display = verbose_status;
        break;

        case 2:
          //OUT 2 - output to nowhere, used in some programming examples
        case 3:
        case 5:
        case 6:
//        case 7:     
          //OUT 2,3,5,6 - unused, output to nowhere
          reg[x_reg]++;
          update_display = verbose_status;
        break;        

        case 4:
          // OUT 4 - output from Memory(X) to databus
          databus = readByte(reg[x_reg]);
          reg[x_reg]++;
          update_display = true;
       
        break; 

        case 7: 
        //OUT 7 -  output from Memory(X) to Serial
          data = readByte(reg[x_reg]);
          reg[x_reg]++;
          Serial.print(data);
          update_display = verbose_status;
        break;
        
        case 8:
          // Not implemented, but used as a debug breakpoint by some monitors
          // HALT - Debug breakpoint, stop and wait for resume
          run_code = false;
          update_display = true;
        break;

        case 9:
          //INP 1 - turn pixie video on
          if (!video) {
            startVideo();
          } // if !video                   
          d_reg = 0x00; 
          writeByte(reg[x_reg], d_reg);          
          update_display = verbose_status;
        break;   

        case 0x0A:
          // INP 2 - input from nowhere, used some in programming examples
        case 0x0B:
        case 0X0D:
        case 0X0E:
        case 0x0F:
          // INP 2,3,5,7 - unused, input from nowhere   
          d_reg = 0xFF;  // No data, bus floats high
          writeByte(reg[x_reg], d_reg);
          update_display = verbose_status;
        break;         

        case 0x0C:
          // INP 4 - input data from keypad into d and Memory(X)                    
          d_reg = keybuffer;
          writeByte(reg[x_reg], d_reg);
          update_display = verbose_status;
        break;
        
        default:
          // Do nothing (NOP)
        break;
      } // switch n_reg;
    break;

    case 7:
      // Arthimetic and Control
      switch (n_reg) {
        case 0:
          // RET - Return and enable interrupts
        case 1:
          // DIS - Return and disable interrupts
          data = readByte(reg[x_reg]);
          reg[x_reg]++;
          x_reg = (data >> 4) & 0x0F;
          p_reg = data & 0x0F;
          ie = (n_reg == 0); // true for 0x70, false for 0x71
          update_display = verbose_status;
        break;

        case 2:
          // LDXA - Load D via Memory(X) and Advance
          d_reg = readByte(reg[x_reg]);
          reg[x_reg]++;
        break;

        case 3:
          // STXD - Store D in Memory(X) and Decrement
          writeByte(reg[x_reg],d_reg); 
          reg[x_reg]--;          
        break;

        case 4:
         // ADC - Add with Carry
         work = readByte(reg[x_reg]) + d_reg; // get memory byte and add
         work += (df ? 1 : 0);  // add in the carry flag       
         df =  (work & 0x100); // set carry flag if overflow
         d_reg = work & 0x00FF;  // mask D to single byte value
        break;

        case 5:
          // SDB - Subract D from Memory(X) with Borrow
          work = readByte(reg[x_reg]) - d_reg;
          work -= (df ? 0 : 1); //borrow if required
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value
        break;

        case 6:
          // SHRC - Shift D Right with Carry
          work = d_reg;
          if (df) {
            work |= 0x100; // sight shift in bit if Carry
          } // if df
          work >>= 1; // shift right
          df = d_reg & 0x01; // set carry to shift out bit
          d_reg = work & 0x00FF;  // mask D to single byte value          
        break;

        case 7:
          // SMB - Subract Memory(X) from D with Borrow
          work = d_reg - readByte(reg[x_reg]);
          work -= (df ? 0 : 1); //borrow if required      
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value        
        break;
        
        case 8:
          // SAV T - Save T in Memory(X)
          writeByte(reg[x_reg], t_reg);
          update_display = verbose_status;
        break;

        case 9:
          // MARK - Push X and P as T in Memory(R2) to mark a subroutine call
          t_reg = (x_reg <<4) | p_reg; // save x and p into T
          writeByte(reg[2],t_reg);  //put T into Memory(R2)
          x_reg = p_reg;  // Move P to X
          reg[2]--; // Decrement R2
          update_display = verbose_status;
        break;
          
        case 0x0A:
          // REQ - Reset Q
          q_reg = false;       
          digitalWrite(13, LOW);
          break;
          
        case 0x0B:
          // SEQ - Set Q
          q_reg = true;
          digitalWrite(13, HIGH);
          break;

        case 0x0C:
          // ADC - Add with Carry Immediate
         work = readByte(reg[p_reg]) + d_reg; // get next byte and add
         work += df ? 1 : 0;  // add in the carry flag      
         df = (work & 0x100); // set carry flag if overflow
         d_reg = work & 0x00FF;  // mask D to single byte value        
         reg[p_reg]++; // go to next opcode
        break;

        case 0x0D:
          // SDBI - Subtract D with Borrow from Memory Immediate
          work = readByte(reg[p_reg]) - d_reg;  // Subtract D from next byte
          work -= (df ? 0 : 1); // Borrow if required
          df = !(work&0x100); // Clear carry flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value  
          reg[p_reg]++; // Go to next opcode
        break;

        case 0x0E:
          // SHLC - Shift D Left with Carry
          work = (d_reg<<1);  // Shift D Left
          work |= df;     // Set shift in bit to Carry
          df = (work&0x100); // Set Carry Flag to shift out bit
          d_reg = work & 0x00FF; // mask D to single byte value  
        break;
        
        case 0x0F:
          // SMBI - Subtract Memory with Borrow from D, Immediate
          work = d_reg - readByte(reg[p_reg]);
          work -= (df ? 0 : 1); //borrow if required          
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value            
          reg[p_reg]++;        
        break;        
      } // switch n_reg
    break;

    case 8:
      // GLO N = Get Low byte of register N
      d_reg = reg[n_reg] & 0x00FF;
    break;
    
    case 9:
      // GHI N = Get High byte of register N
      d_reg = (reg[n_reg] >> 8) & 0x00FF;    
    break;

    case 0x0A:
      // PLO N = Put D into Low byte of register N
      reg[n_reg] = (reg[n_reg] & 0xFF00) | d_reg;      
    break;
    
    case 0x0B:
      // PHI N = Put D into High byte of register N
      reg[n_reg] = (reg[n_reg] & 0x00FF) | (d_reg << 8);          
    break;

    case 0x0C:
      //Long branches and skips
      target = (readByte(reg[p_reg])<<8) | readByte(reg[p_reg]+1);
      switch(n_reg) {
        case 0:
          // LBR - Long Branch Unconditionally
          reg[p_reg] = target;
        break;

        case 1:
          // LBQ - Long Branch if Q is on
          if (q_reg) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }
        break;

        case 2:
          // LBZ - Long Branch if D is zero
          if (d_reg == 0) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }
        break;

        case 3:
          // LBDF - Long Branch if DF 
          if (df) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }
        break;

        case 4:
          // NOP - No operation
        break;

        case 5:
          // LSNQ - Long Skip if Q is off
          if (!q_reg) {
            reg[p_reg] += 2;
          }
        break;

        case 6:
          // LSNZ - Long SKip if D is Not Zero
          if (d_reg != 0) {
            reg[p_reg] += 2;
          }
        break;

        case 7:
          // LSNF - Long Skip if not DF
          if (!df) {
            reg[p_reg] += 2;
          }
        break;

        case 8:
          // LSKP - Long Skip
          reg[p_reg] += 2;
        break;

        case 9:
          // LBNQ - Long Branch if Q is off
          if (!q_reg) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }        
        break;

        case 0x0A:
          // LBNZ - Long Branch if D is Not Zero
          if (d_reg != 0) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }
        break;

        case 0x0B:
          // LBNF - Long Branch if not DF 
          if (!df) {
            reg[p_reg] = target;
          } else {
            reg[p_reg] += 2;
          }        
        break;

        case 0x0C:
          // LSIE - Long Skip if Interrupts Enabled
          if (ie) {
            reg[p_reg] += 2;  
          }
        break;

        case 0x0D:
          // LSQ - Long Skip if Q is on
          if (q_reg) {
            reg[p_reg] += 2;
          }    
        break;

        case 0x0E:
          // LSZ - Long SKip if D is Zero
          if (d_reg == 0) {
            reg[p_reg] += 2;        
          }
        break;

        case 0x0F:
          // LSDF - Long Skip if DF
          if (df) {
            reg[p_reg] += 2;
          }
        break;        
      } // switch n_reg
    break;

    case 0x0D:
      // SEP N - Set P to N (making Reg(N) the new program counter)
      p_reg = n_reg;
      update_display = verbose_status;
    break;

    case 0x0E:
      // SEX N - Set X to N (making Reg(N) the new index register)
      x_reg = n_reg;
      update_display = verbose_status;
    break;

    case 0x0F:
      // Arthimetic and Control
      switch (n_reg) {
        case 0:
          // LDX - Load D via Memory(X)
          d_reg = readByte(reg[x_reg]);        
        break;

        case 1:
          // OR - Logical OR
          d_reg |= readByte(reg[x_reg]);
        break;

        case 2:
          // AND - Logical AND
          d_reg &= readByte(reg[x_reg]);
        break;

        case 3:
          // XOR - Exclusive OR
          d_reg ^= readByte(reg[x_reg]);
        break;

        case 4:
          // ADD - Add Memory(X) to D
          work = readByte(reg[x_reg]) + d_reg; // get memory byte and add
          df =  (work & 0x100); // set carry flag if overflow
          d_reg = work & 0x00FF;  // mask D to single byte value          
        break;

        case 5:
          // SD - Subtract D from Memory(X)
          work = readByte(reg[x_reg]) - d_reg;
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value        
        break;

        case 6:
          // SHR - Shift D Right
          df = (d_reg & 1); // Set carry to shift out bit
          d_reg >>= 1;          
        break;

        case 7:
          // SM - Subtract Memory(X) from D
          work = d_reg - readByte(reg[x_reg]);          
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value            
        break;

        case 8:
          // LDI - Load D Immediate
          d_reg = readByte(reg[p_reg]);
          reg[p_reg]++;
        break;

        case 9:
          // ORI - OR Immediate
          d_reg |= readByte(reg[p_reg]);
          reg[p_reg]++;
        break;

        case 0x0A:
          // ANI - And Immediate
          d_reg &= readByte(reg[p_reg]);
          reg[p_reg]++;          
        break;

        case 0x0B:
          // XRI - Exclusive OR Immediate
          d_reg ^= readByte(reg[p_reg]);
          reg[p_reg]++;        
        break;

        case 0x0C:
          // ADI - Add Immediate
          work = readByte(reg[p_reg]) + d_reg; // get next byte and add
          df = (work & 0x100); // set carry flag if overflow
          d_reg = work & 0x00FF;  // mask D to single byte value        
          reg[p_reg]++; // go to next opcode          
        break;

        case 0x0D:
          // SDI - Subtract D from Immediate byte
          work = readByte(reg[p_reg]) - d_reg;  // Subtract D from next byte
          df = !(work&0x100); // Clear carry flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value  
          reg[p_reg]++; // Go to next opcode          
        break;

        case 0x0E:
          // SHL - Shift D Left
          df = (d_reg & 0x80); // Set carry flag to shift out bit
          d_reg <<= 1; // Shift D left
        break;

        case 0x0F:
          // SMI Subtract Memory from D, Immediate
          work = d_reg - readByte(reg[p_reg]);
          df = !(work & 0x100); // clear borrow flag if underflow
          d_reg = work & 0x00FF; // mask D to single byte value            
          reg[p_reg]++;             
        break;
      } // switch n_reg
    break;
  } // switch
} // execute1802


/*
 * Interrupt and DMA routines
 */
// Make an interrupt request
void raiseInterrupt() {
  irq = true;
} // raiseInterrupt

// End an interrupt request
void lowerInterrupt() {
  irq = false;
} // lowerInterrupt

// Service an interrupt request. 
// When this happens, the CPU will do the following:
//    The P and X registers are copied to T.
//    The P and X registers are loaded with 1 and 2 respectively.
//    interrupts are disabled. 
void handleInterrupt() {
  cpu_idle = false; // Wake up CPU, if necessary
  t_reg = (x_reg << 4) | p_reg; // save x and p into T
  p_reg = 1; // Set P to register 1
  x_reg = 2; // Set X to register 2 (Stack)
  ie = 0; // Disable interrupts
}

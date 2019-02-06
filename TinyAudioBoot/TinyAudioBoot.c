/*

  AudioBoot - flashing a microcontroller by PC audio line out

  Originally the bootloader was made for Atmega8 and Atmega168 microcontrollers.

  Budi Prakosa a.k.a Iyok reworked the code to get it running on an Attiny85 microcontroller.

  Parts of the  * equinox-boot.c - bootloader for equinox
  from Frank Meyer and Robert Meyer are used to access the FLASH memory.

  Hardware:   Attiny85

  input pin:  should be connected to a voltage divider.
  output pin: LED for status indication of the bootloader

  The input pin is also connected by a 10nF capacitor to the PC line out
  
  This version has the ability to skip the bootloader at start up.
  This is useful for faster MCU operation at power on.
  The bootloader is only entered when a button is pressed to
  pull the soundprog line low.
  
  //===== historic comments ======
  The Atmega168 seems to have the switching voltage level at 2.2V
  The Atmega8 at 1.4V
  The switching levels of the input pins may vary a little bit from one
  MC to another.  If you to be able to adjust the voltages,
  use a 10k poti as voltage divider.

  As development platform an Arduino Diecimilla was used. Therefore you
  will find many #ifdefs for the Arduino in this code.
  If you want to optimize the bootloader further you may use an Arduino
  as development platform.


  necessary setup

  1. Project->ConfigurationOptions->Processortype
  2. Project->ConfigurationOptions->Programming Modell 'Os'
  3. Project->ConfigurationOptions->CustomOptions->LinkerOptions->see further down

  There is an article how to make an ATTINY boot loader ( German ):
  http://www.mikrocontroller.net/articles/Konzept_f%C3%BCr_einen_ATtiny-Bootloader_in_C
  ( thanks to the author of the article, very well written )


  Creating the bootloader with Atmel Studio 7
  ===========================================

  1. You have to define the bootloader sections and reset vector location

  => Toolchain/AVR_GNU_Linker/Memory Settings 
  .bootreset=0x00
  .text=0xE00 // for 1KB Bootloader
  .text=0x0C00 // for 2KB Bootloader

   explanation:
  .text=0x0E00 *2 = 0x1C00 ==> this is the start address of the boot loader with 1KB size
  .text=0x0C00 *2 = 0x1800 ==> this is the start address of the boot loader with 2KB size

  2. Disable unused sections optimization in the linker
  Be sure that in the linker parameters this is not used: -Wl, --gc-sections
  disable the following check box:
  ==>Toolchain/AVR_GNU_C Compiler/Optimization/Garbage collect unused sections


  Fuse settings for the bootloader
  ================================

  There fuses have to match certain conditions.
  Mainly SELFPROGEN has to be set, Brown-Out-Detection activated and
  CKDIV8 disabled to achieve the needed F_CPU of 8MHz

  FUSES Attiny 85 ( F_CPU 16MHz with PLL )
  ========================================
  Extended: 0xFE
  HIGH:     0xDD
  LOW:      0xE1

  ************************************************************************************

  v0.1  19.6.2008 C. -H-A-B-E-R-E-R-  Bootloader for IR-Interface
  v1.0  03.9.2011 C. -H-A-B-E-R-E-R-  Bootloader for audio signal
  v1.1  05.9.2011 C. -H-A-B-E-R-E-R-  changing pin setup, comments, and exitcounter=3
  v1.2  12.5.2012 C. -H-A-B-E-R-E-R-  Atmega8 Support added, java program has to be adapted too
  v1.3  20.5.2012 C. -H-A-B-E-R-E-R-  now interrupts of user program are working
  v1.4  05.6.2012 C. -H-A-B-E-R-E-R-  signal coding changed to differential manchester code
  v2.0  13.6.2012 C. -H-A-B-E-R-E-R-  setup for various MCs
  v3.0  30.1.2017 B. -P-r-a-k-o-s-a-  first version of Attiny85 Audio Bootloader
  v3.1  04.2.2017 C. -H-A-B-E-R-E-R-  clean reset vector added, description added, pins rerouted
  v3.2  18.7.2017 B. -P-r-a-k-o-s-a-  various refactor, added eeprom write mode, makefile for compiling using arduino ide toolchain
  v3.3  06.2.2019 C. -H-A-B-E-R-E-R-  check if bootloader shall be skipped when soundProgPin is high at start

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  It is mandatory to keep the list of authors in this code.

*/
/*

			
           schematic for audio input
           =========================			
  	
                      VCC	
                       |	 
                      | | 10K
                      | |
                       |
                       |
 audio in >-----||-----o------->  soundprog ( digital input pin )
               100nF   |
                       |  
                      | | 10K
                      | |
                       |
                       |----o
                       |     \===| press button to enter bootloader mode
                       |      \ 
                       |    o  
                       |    |
                      GND  GND



                              Pinout ATtiny25/45/85
                              =====================
			
                                     _______
                                    |   U   |
         (PCINT5/RESET/ADC0/dW) PB5-|       |- VCC
  (PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3-| ATTINY|- PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2) 
  (PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4-|   85  |- PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1) 
                                GND-|       |- PB0 (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)	 
                                    |_______|

				  
				  
				                  Pinout ARDUINO  
				                  ==============
                                     _______				  
                                    |   U   |				  
                          reset/PB5-|       |- VCC				  
             soundprog-> D3/A3  PB3-| ATTINY|- PB2       D2/A1				  
                 D4/A2          PB4-|   85  |- PB1       D1     -> ARDUINO_LED
                                GND-|       |- PB0       D0
                                    |_______|		


				  
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>

// This value has to be adapted to the bootloader size
// If you change this, please change BOOTLOADER_ADDRESS on Makefile too

#define BOOTLOADER_ADDRESS     0x1BC0               // bootloader start address, e.g. 0x1C00 = 7168, set .text to 0x0E00

//#define BOOTLOADER_ADDRESS     0x1800             // bootloader start address, e.g. 0x1800 = 6144, set .text to 0x0c00

#define RJMP                   (0xC000U - 1)          // opcode of RJMP minus offset 1
#define RESET_SECTION          __attribute__((section(".bootreset"))) __attribute__((used))

// this variable seems to be unused
// but in fact it is written into the flash section
// you could find it in the *.hex file
uint16_t resetVector RESET_SECTION = RJMP + BOOTLOADER_ADDRESS / 2;

#ifdef DEBUGON

	#define DEBUGPIN       ( 1<<PB2 ) 
	#define INITDEBUGPIN   { DDRB  |=  DEBUGPIN; }

	#define DEBUGPINON     { PORTB |=  DEBUGPIN;}
	#define DEBUGPINOFF    { PORTB &= ~DEBUGPIN;}
	#define TOGGLEDEBUGPIN { PORTB ^=  DEBUGPIN;}
	
#else

	#define DEBUGPIN       
	#define INITDEBUGPIN   

	#define DEBUGPINON     
	#define DEBUGPINOFF    
	#define TOGGLEDEBUGPIN 
	
#endif
	
	
#define USELED
#ifdef USELED

	#define LEDPORT    ( 1<<PB1 ); //PB1 pin 6 Attiny85
	#define INITLED    { DDRB|=LEDPORT; }

	#define LEDON      { PORTB|=LEDPORT;}
	#define LEDOFF     { PORTB&=~LEDPORT;}
	#define TOGGLELED  { PORTB^=LEDPORT;}

#else

	#define LEDPORT 
	#define INITLED 

	#define LEDON 
	#define LEDOFF 
	#define TOGGLELED 

#endif

#define INPUTAUDIOPIN (1<<PB3) //
#define PINVALUE (PINB&INPUTAUDIOPIN)
#define INITAUDIOPORT {DDRB&=~INPUTAUDIOPIN;} // audio pin is input

#define PINLOW (PINVALUE==0)
#define PINHIGH (!PINLOW)

#define WAITBLINKTIME 10000
#define BOOT_TIMEOUT  50

#define true (1==1)
#define false !true

//***************************************************************************************
// main loop
//***************************************************************************************

#define TIMER TCNT0 // we use timer1 for measuring time

// frame format definition: indices
#define COMMAND         0
#define PAGEINDEXLOW    1  // page address lower part
#define PAGEINDEXHIGH   2  // page address higher part
#define LENGTHLOW       3
#define LENGTHHIGH      4
#define CRCLOW          5  // checksum lower part 
#define CRCHIGH         6  // checksum higher part 
#define DATAPAGESTART   7  // start of data
#define PAGESIZE        SPM_PAGESIZE
#define FRAMESIZE       (PAGESIZE+DATAPAGESTART) // size of the data block to be received

// bootloader commands
#define NOCOMMAND       0
#define TESTCOMMAND     1
#define PROGCOMMAND     2
#define RUNCOMMAND      3
#define EEPROMCOMMAND   4
#define EXITCOMMAND     5

uint8_t FrameData[ FRAMESIZE ];

#define FLASH_RESET_ADDR        0x0000                 // address of reset vector (in bytes)
#define BOOTLOADER_STARTADDRESS BOOTLOADER_ADDRESS    // start address:
#define BOOTLOADER_ENDADDRESS   0x2000                // end address:   0x2000 = 8192
                                                      // this is the size of the Attiny85 flash in bytes
													  
#define LAST_PAGE (BOOTLOADER_STARTADDRESS - SPM_PAGESIZE) / SPM_PAGESIZE

#include <avr/boot.h>
#ifndef RWWSRE                                        // bug in AVR libc:
#define RWWSRE CTPB                                   // RWWSRE is not defined on ATTinys, use CTBP instead
#endif

void (*start_appl_main) (void);

#define BOOTLOADER_FUNC_ADDRESS (BOOTLOADER_STARTADDRESS - sizeof (start_appl_main))

//AVR ATtiny85 Programming: EEPROM Reading and Writing - YouTube
//https://www.youtube.com/watch?v=DO-D6YmRpJk

inline void eeprom_write(unsigned short address, unsigned char data)
{
   while(EECR & (1<<EEPE));

   EECR = (0<<EEPM1) | (0<<EEPM0);

   if (address < 512)
   {
      EEAR = address;
   }else{
      EEAR = 511;
   }

   EEDR = data;

   EECR |= (1<<EEMPE);
   EECR |= (1<<EEPE);  
}

//***************************************************************************************
// receiveFrame()
//
// This routine receives a differential manchester coded signal at the input pin.
// The routine waits for a toggling voltage level.
// It automatically detects the transmission speed.
//
// output:    uint8_t flag:     true: checksum OK
//            uint8_t FramData: global data buffer
//
//***************************************************************************************
inline uint8_t receiveFrame()
{
  //uint16_t store[16];

  uint16_t counter = 0;
  volatile uint16_t time = 0;
  volatile uint16_t delayTime;
  uint8_t p, t;
  uint8_t k = 8;
  uint8_t dataPointer = 0;
  uint16_t n;

  //*** synchronisation and bit rate estimation **************************
  time = 0;
  // wait for edge
  p = PINVALUE;
  while (p == PINVALUE);

  p = PINVALUE;

  TIMER = 0; // reset timer
  for (n = 0; n < 16; n++)
  {
    // wait for edge
    while (p == PINVALUE);
    t = TIMER;
    TIMER = 0; // reset timer
    p = PINVALUE;

    //store[counter++] = t;

    if (n >= 8)time += t; // time accumulator for mean period calculation only the last 8 times are used
  }

  delayTime = time * 3 / 4 / 8;
  // delay 3/4 bit
  while (TIMER < delayTime);

  //p=1;

  //****************** wait for start bit ***************************
  while (p == PINVALUE) // while not startbit ( no change of pinValue means 0 bit )
  {
    // wait for edge
    while (p == PINVALUE);
    p = PINVALUE;
    TIMER = 0;

    // delay 3/4 bit
    while (TIMER < delayTime);
    TIMER = 0;

    counter++;
  }
  p = PINVALUE;
  
  //****************************************************************
  //receive data bits
  k = 8;
  for (n = 0; n < (FRAMESIZE * 8); n++)
  {
    // wait for edge
    while (p == PINVALUE);
    TIMER = 0;
    p = PINVALUE;

    // delay 3/4 bit
    while (TIMER < delayTime);

    t = PINVALUE;
	  

    counter++;

    FrameData[dataPointer] = FrameData[dataPointer] << 1;
    if (p != t) FrameData[dataPointer] |= 1;
    p = t;
    k--;
    if (k == 0) {
      dataPointer++;
      k = 8;
    };
  }
  //uint16_t crc = (uint16_t)FrameData[CRCLOW] + FrameData[CRCHIGH] * 256;
  
  
  return true;
}

/*-----------------------------------------------------------------------------------------------------------------------
   Flash: fill page word by word
  -----------------------------------------------------------------------------------------------------------------------
*/
#define boot_program_page_fill(byteaddr, word)      \
  {                                                 \
    uint8_t sreg;                                   \
    sreg = SREG;                                    \
    cli ();                                         \
    boot_page_fill ((uint32_t) (byteaddr), word);   \
    SREG = sreg;                                    \
  }

/*-----------------------------------------------------------------------------------------------------------------------
   Flash: erase and write page
  -----------------------------------------------------------------------------------------------------------------------
*/

#define boot_program_page_erase_write(pageaddr)     \
  {                                                 \
    uint8_t sreg;                                   \
    eeprom_busy_wait ();                            \
    sreg = SREG;                                    \
    cli ();                                         \
    boot_page_erase ((uint32_t) (pageaddr));        \
    boot_spm_busy_wait ();                          \
    boot_page_write ((uint32_t) (pageaddr));        \
    boot_spm_busy_wait ();                          \
    boot_rww_enable ();                             \
    SREG = sreg;                                    \
  }


/*-----------------------------------------------------------------------------------------------------------------------
   write a block into flash
  -----------------------------------------------------------------------------------------------------------------------
*/
inline void
pgm_write_block (uint16_t flash_addr, uint16_t * block, size_t size)
{
  uint16_t        start_addr;
  uint16_t        addr;
  uint16_t        w;
  uint8_t         idx = 0;

  start_addr = (flash_addr / SPM_PAGESIZE) * SPM_PAGESIZE;        // round down (granularity is SPM_PAGESIZE)

  for (idx = 0; idx < SPM_PAGESIZE / 2; idx++)
  {
    addr = start_addr + 2 * idx;

    if (addr >= flash_addr && size > 0)
    {
      w = *block++;
      size -= sizeof (uint16_t);
    }
    else
    {
      w = pgm_read_word (addr);
    }

    boot_program_page_fill (addr, w);
  }

  boot_program_page_erase_write(start_addr);                      // erase and write the page
}


//***************************************************************************************
//  void boot_program_page (uint32_t page, uint8_t *buf)
//
//  Erase and flash one page.
//
//  input:     page address and data to be programmed
//
//***************************************************************************************
void boot_program_page (uint32_t page, uint8_t *buf)
{
  uint16_t i;
  cli(); // disable interrupts

  boot_page_erase(page);
  boot_spm_busy_wait ();      // Wait until the memory is erased.

  for (i = 0; i < SPM_PAGESIZE; i += 2)
  {
    //read received data
    uint16_t w = *buf++; //low section
    w += (*buf++) << 8; //high section
    //combine low and high to get 16 bit

    //first page and first index is vector table... ( page 0 and index 0 )
    if (page == 0 && i == 0)
    {
      
      //1.save jump to application vector for later patching
      void* appl = (void *)(w - RJMP);
      start_appl_main =  ((void (*)(void)) appl);

      //2.replace w with jump vector to bootloader
      w = 0xC000 + (BOOTLOADER_ADDRESS / 2) - 1;
    }
    // else if (page == LAST_PAGE && i == 60)
    // {
      //3.retrieve saved reset vector
      // w = saved_reset_vector;
    // }

    boot_page_fill (page + i, w);
    boot_spm_busy_wait();       // Wait until the memory is written.
  }

  boot_page_write (page);     // Store buffer in flash page.
  boot_spm_busy_wait();       // Wait until the memory is written.
}

inline void resetRegister()
{
    DDRB = 0;
    cli();
    TCCR0B = 0; // turn off timer1
	ADCSRA = 0;
	ADMUX=0;
}

void startMainApplication()
{
	resetRegister();
	(*start_appl_main) ();
	
}

// use this routine on bootloader timeout and no flash values are written
void exitBootloader()
{  
  memcpy_P (&start_appl_main, (PGM_P) BOOTLOADER_FUNC_ADDRESS, sizeof (start_appl_main));

  if (start_appl_main)
  {
     startMainApplication();
  }
}

// use this routine after new flash values are written
void runProgramm(void)
{
  pgm_write_block (BOOTLOADER_FUNC_ADDRESS, (uint16_t *) &start_appl_main, sizeof (start_appl_main));

  startMainApplication();
}

// initADC(): tributes to
// https://www.marcelpost.com/wiki/index.php/ATtiny85_ADC
inline void initADC()
{
	  /* this function initialises the ADC 

        ADC Prescaler Notes:
	--------------------

	   ADC Prescaler needs to be set so that the ADC input frequency is between 50 - 200kHz.
  
           For more information, see table 17.5 "ADC Prescaler Selections" in 
           chapter 17.13.2 "ADCSRA : ADC Control and Status Register A"
          (pages 140 and 141 on the complete ATtiny25/45/85 datasheet, Rev. 2586M-AVR-07/10)

           Valid prescaler values for various clock speeds
	
	     Clock   Available prescaler values
           ---------------------------------------
             1 MHz   8 (125kHz), 16 (62.5kHz)
             4 MHz   32 (125kHz), 64 (62.5kHz)
             8 MHz   64 (125kHz), 128 (62.5kHz)
            16 MHz   128 (125kHz)

           Below example set prescaler to 128 for mcu running at 8MHz
           (check the datasheet for the proper bit values to set the prescaler)
  */

  // 8-bit resolution
  // set ADLAR to 1 to enable the Left-shift result (only bits ADC9..ADC2 are available)
  // then, only reading ADCH is sufficient for 8-bit results (256 values)

#if (INPUTAUDIOPIN == (1<<PB4))

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB4), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB4), MUX bit 2
            (1 << MUX1)  |     // use ADC2 for input (PB4), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB4), MUX bit 0
			
#elif (INPUTAUDIOPIN == (1<<PB3))

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB3), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB3), MUX bit 2
            (1 << MUX1)  |     // use ADC2 for input (PB3), MUX bit 1
            (1 << MUX0);       // use ADC2 for input (PB3), MUX bit 0

#elif (INPUTAUDIOPIN == (1<<PB2))

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB2), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB2), MUX bit 2
            (0 << MUX1)  |     // use ADC2 for input (PB2), MUX bit 1
            (1 << MUX0);       // use ADC2 for input (PB2), MUX bit 0
			
#elif (INPUTAUDIOPIN == (1<<PB5))

  ADMUX =
            (1 << ADLAR) |     // left shift result
            (0 << REFS1) |     // Sets ref. voltage to VCC, bit 1
            (0 << REFS0) |     // Sets ref. voltage to VCC, bit 0
            (0 << MUX3)  |     // use ADC2 for input (PB5), MUX bit 3
            (0 << MUX2)  |     // use ADC2 for input (PB5), MUX bit 2
            (0 << MUX1)  |     // use ADC2 for input (PB5), MUX bit 1
            (0 << MUX0);       // use ADC2 for input (PB5), MUX bit 0
			
			
#endif

  ADCSRA = 
            (1 << ADEN)  |     // Enable ADC 
            (1 << ADPS2) |     // set prescaler to 64, bit 2 
            (1 << ADPS1) |     // set prescaler to 64, bit 1 
            (0 << ADPS0);      // set prescaler to 64, bit 0  

}

inline void disableADC()
{
	  ADCSRA &=	 ~ (1 << ADEN) ;    // disable ADC
}

// soundprog indicates if the bootloader shall be skipped.
// if the level of soundprog exceeds a certain level at MCU start up
// the bootloader shall be skipped to avoid program start up delay
	
inline void checkBootloaderSkip()
{
  initADC();

  ADCSRA |= (1 << ADSC);         // start ADC measurement
  while (ADCSRA & (1 << ADSC) ); // wait till conversion complete
  ADCSRA |= (1 << ADSC);         // start ADC measurement
  while (ADCSRA & (1 << ADSC) ); // wait till conversion complete
  
  if (ADCH > 75) exitBootloader(); // skip the bootloader if the right button is pressed
}

//***************************************************************************************
// main loop
//***************************************************************************************
static inline void a_main()
{
  uint8_t p;
  uint16_t time = WAITBLINKTIME;
  uint8_t timeout = BOOT_TIMEOUT;

  p = PINVALUE;

  //*************** wait for toggling input pin or timeout ******************************
  uint8_t exitcounter = 3;
  while (1)
  {

    if (TIMER > 100) // timedelay ==> frequency @16MHz= 16MHz/8/100=20kHz
    {
      TIMER = 0;
      time--;
      if (time == 0)
      {
        TOGGLELED;

        time = WAITBLINKTIME;
        timeout--;
        if (timeout == 0)
        {
          LEDOFF; // timeout,
          // leave bootloader and run program
          exitBootloader();
        }
      }
    }
    if (p != PINVALUE)
    {
      p = PINVALUE;
      exitcounter--;
    }
    if (exitcounter == 0) break; // signal received, leave this loop and go on
  }
  //*************** start command interpreter *************************************
  LEDON;
  while (1)
  {
    if (!receiveFrame())
    {
      //*****  if data transfer error: blink fast, press reset to restart *******************

      while (1)
      {
        if (TIMER > 100) // timerstop ==> frequency @16MHz= 16MHz/8/100=20kHz
        {
          TIMER = 0;
          time--;
          if (time == 0)
          {
            TOGGLELED;
            time = 1000;
          }
        }
      }
    }
    else // succeed
    {
      switch (FrameData[COMMAND])
      {

        case PROGCOMMAND:
        {
            uint16_t pageNumber = (((uint16_t)FrameData[PAGEINDEXHIGH]) << 8) + FrameData[PAGEINDEXLOW];
			      uint16_t address=SPM_PAGESIZE * pageNumber;
			
            if( address < BOOTLOADER_ADDRESS) // prevent bootloader form self killing
        		{
        			boot_program_page (address, FrameData + DATAPAGESTART);  // erase and program page
        			TOGGLELED;
        		}
        }
        break;

        case RUNCOMMAND:
        {
            // after programming leave bootloader and run program
            runProgramm();
        }
        break;

        // case EXITCOMMAND:
        // {
        //     // after programming leave bootloader and run program
        //     exitBootloader();
        // }
        // break;

        case EEPROMCOMMAND:
        {
            uint8_t pageNumber = FrameData[PAGEINDEXLOW];
            uint8_t data_length = FrameData[LENGTHLOW];
            uint8_t address = SPM_PAGESIZE * pageNumber;

            uint8_t *buf = FrameData + DATAPAGESTART;
            
            for (uint8_t i = 0; i < data_length; i++)
            {
              //write received data to EEPROM
              uint8_t w = *buf++;
              eeprom_write(address + i, w);
            }

            //Leave bootloader after eeprom signal received
            //todo: wait until all data sent > spm pagesize (64)
            //fix this!!!!
            LEDOFF;
            exitBootloader();

        }
        break;
      }
      FrameData[COMMAND] = NOCOMMAND; // delete command
    }
  }
}

int main()
{
  INITAUDIOPORT;
  
  checkBootloaderSkip();
	
  INITDEBUGPIN
  INITLED;

  // Timer 2 normal mode, clk/8, count up from 0 to 255
  // ==> frequency @16MHz= 16MHz/8/256=7812.5Hz
  TCCR0B = _BV(CS01);

  a_main(); // start the main function
}

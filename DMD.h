/*--------------------------------------------------------------------------------------

 DMD.h   - Function and support library for the Freetronics DMD, a 512 LED matrix display
           panel arranged in a 32 x 16 layout.

 Copyright (C) 2011 Marc Alexander (info <at> freetronics <dot> com)

 Note that the DMD library uses the SPI port for the fastest, low overhead writing to the
 display. Keep an eye on conflicts if there are any other devices running from the same
 SPI port, and that the chip select on those devices is correctly set to be inactive
 when the DMD is being written to.


LED Panel Layout in RAM
                            32 pixels (4 bytes)
        top left  ----------------------------------------
                  |                                      |
         Screen 1 |        512 pixels (64 bytes)         | 16 pixels
                  |                                      |
                  ---------------------------------------- bottom right

 ---
 
 This program is free software: you can redistribute it and/or modify it under the terms
 of the version 3 GNU General Public License as published by the Free Software Foundation.

 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 See the GNU General Public License for more details.

 You should have received a copy of the GNU General Public License along with this program.
 If not, see <http://www.gnu.org/licenses/>.

--------------------------------------------------------------------------------------*/
#ifndef DMD_H_
#define DMD_H_

//Arduino toolchain header, version dependent
#if defined(ARDUINO) && ARDUINO >= 100
	#include "Arduino.h"
#else
	#include "WProgram.h"
#endif

//SPI library must be included for the SPI scanning/connection method to the DMD
#include "pins_arduino.h"
#include <avr/pgmspace.h>
#include <SPI.h>

// ######################################################################################################################
// ######################################################################################################################
#warning CHANGE THESE TO SEMI-ADJUSTABLE PIN DEFS!
//Arduino pins used for the display connection
#define PIN_DMD_nOE       9    // D9 active low Output Enable, setting this low lights all the LEDs in the selected rows. Can pwm it at very high frequency for brightness control.
#define PIN_DMD_A         6    // D6
#define PIN_DMD_B         7    // D7
#define PIN_DMD_CLK       13   // D13_SCK  is SPI Clock if SPI is used
#define PIN_DMD_SCLK      8    // D8
#define PIN_DMD_R_DATA    11   // D11_MOSI is SPI Master Out if SPI is used
//Define this chip select pin that the Ethernet W5100 IC or other SPI device uses
//if it is in use during a DMD scan request then scanDisplayBySPI() will exit without conflict! (and skip that scan)
//#define PIN_OTHER_SPI_nCS  10
// ######################################################################################################################
// ######################################################################################################################

//Handy defines
#define  ASCIINUMBASE      0x30  //base value for ascii number '0'
#define  CHAR_CR           0x0D  //carriage return character
#define  CHAR_LF           0x0A  //line feed character

//DMD I/O pin macros
#define LIGHT_DMD_ROW_01_05_09_13()       { digitalWrite( PIN_DMD_B,  LOW ); digitalWrite( PIN_DMD_A,  LOW ); }
#define LIGHT_DMD_ROW_02_06_10_14()       { digitalWrite( PIN_DMD_B,  LOW ); digitalWrite( PIN_DMD_A, HIGH ); }
#define LIGHT_DMD_ROW_03_07_11_15()       { digitalWrite( PIN_DMD_B, HIGH ); digitalWrite( PIN_DMD_A,  LOW ); }
#define LIGHT_DMD_ROW_04_08_12_16()       { digitalWrite( PIN_DMD_B, HIGH ); digitalWrite( PIN_DMD_A, HIGH ); }
#define LATCH_DMD_SHIFT_REG_TO_OUTPUT()   { digitalWrite( PIN_DMD_SCLK, HIGH ); digitalWrite( PIN_DMD_SCLK,  LOW ); }
#define OE_DMD_ROWS_OFF()                 { digitalWrite( PIN_DMD_nOE, LOW  ); }
#define OE_DMD_ROWS_ON()                  { digitalWrite( PIN_DMD_nOE, HIGH ); }

//Pixel/graphics writing modes (bGraphicsMode)
#define GRAPHICS_NORMAL   0
#define GRAPHICS_INVERSE  1
#define GRAPHICS_TOGGLE   2
#define GRAPHICS_OR       3
#define GRAPHICS_NOR      4

//drawTestPattern Patterns
#define PATTERN_ALT_0     0
#define PATTERN_ALT_1     1
#define PATTERN_STRIPE_0  2
#define PATTERN_STRIPE_1  3

//display screen (and subscreen) sizing
#define DMD_PIXELS_ACROSS         32      //pixels across x axis (base 2 size expected)
#define DMD_PIXELS_DOWN           16      //pixels down y axis
#define DMD_BITSPERPIXEL           1      //1 bit per pixel, use more bits to allow for pwm screen brightness control
#define DMD_RAM_SIZE_BYTES        ((DMD_PIXELS_ACROSS*DMD_BITSPERPIXEL/8)*DMD_PIXELS_DOWN)
                                  // (32x * 1 / 8) = 4 bytes, * 16y = 64 bytes per screen here.
//lookup table for DMD::writePixel to make the pixel indexing routine faster
static byte bPixelLookupTable[8] =
{
   0x80,   //0, bit 7
   0x40,   //1, bit 6
   0x20,   //2. bit 5
   0x10,   //3, bit 4
   0x08,   //4, bit 3
   0x04,   //5, bit 2
   0x02,   //6, bit 1
   0x01    //7, bit 0
};

//The main class of DMD library functions
class DMD
{
  public:
    //Instantiate the DMD
    DMD();
	//virtual ~DMD();

	//Set or clear a pixel at the x and y location (0,0 is the top left corner)
	void writePixel( byte bX, byte bY, byte bGraphicsMode, byte bPixel );

	//Draw a character with the 5 x 7 font table at the x and y location. bSet true is on, false is inverted
  void drawCharacter_5x7( byte bX, byte bY, byte bChar, byte bGraphicsMode );

	//Draw a character with the 6 x 16 font table at the x and y location. bSet true is on, false is inverted
	//The 6 x 16 font table is designed to be good for large clock and 4 digit number displays with a colon in the centre
	void drawCharacter_6x16( byte bX, byte bY, byte bChar, byte bGraphicsMode );

	//Clear the screen in DMD RAM
  void clearScreen( byte bNormal );

  //Draw or clear a line from x1,y1 to x2,y2
  void drawLine( byte x1, byte y1, byte x2, byte y2, byte bGraphicsMode );

	//Draw or clear a circle of radius r at x,y centre
  void drawCircle( int xCenter, int yCenter, int radius, byte bGraphicsMode );

	//Draw or clear a box(rectangle) with a single pixel border
  void drawBox( byte x1, byte y1, byte x2, byte y2, byte bGraphicsMode );

	//Draw or clear a filled box(rectangle) with a single pixel border
  void drawFilledBox( byte x1, byte y1, byte x2, byte y2, byte bGraphicsMode );

  //Draw the selected test pattern
  void drawTestPattern( byte bPattern );

 	//Scan the dot matrix LED panel display, from the RAM mirror out to the display hardware.
 	//Call 4 times to scan the whole display which is made up of 4 interleaved rows within the 16 total rows.
 	//Insert the calls to this function into the main loop for the highest call rate, or from a timer interrupt
	void scanDisplayBySPI();

  private:
  void drawCircleSub( int cx, int cy, int x, int y, byte bGraphicsMode );

	//Mirror of DMD pixels in RAM, ready to be clocked out by the main loop or high speed timer calls
	byte bDMDScreenRAM[DMD_RAM_SIZE_BYTES];

	//scanning pointer into bDMDScreenRAM, setup init @ 48 for the first valid scan
	volatile byte bDMDByte;

};

#endif /* DMD_H_ */
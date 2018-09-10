// LCD.h
// Steven Prickett (steven.prickett@gmail.com)
// Runs on LM4F120 or TM4C123
// August 23, 2013

// Driver for the SSD2119 interface on a Kentec 320x240x16 BoosterPack
// - Uses all 8 bits on PortB for writing data to LCD 
//   and bits 4-7 on Port A for control signals
// - Adapted from original Kentec320x240x16_ssd2119_8bit.c driver by TI

//Copyright 2013 by Steven Prickett (steven.prickett@gmail.com)
//   You may use, edit, run or distribute this file
//   as long as the above copyright notice remains
//THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
//OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
//MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
//PRICKETT SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
//OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

#ifndef LCD_H
#define LCD_H
// color definitions encoded as 5-6-5 RGB
#define BLACK         0x0000  // R=0   G=0   B=0
#define WHITE         0xFFFF  // R=255 G=255 B=255
#define PINK          0xF81F  // R=255 G=0   B=255
#define RED           0xF800  // R=255 G=0   B=0
#define BURNTORANGE   0xC300  // R=197 G=96  B=6
#define ORANGE        0xFC00  // R=255 G=128 B=0
#define YELLOW        0xFFE0  // R=255 G=255 B=0
#define GREEN         0x07E0  // R=0   G=255 B=0
#define LIGHTBLUE     0x041F  // R=0   G=128 B=255
#define BLUE          0x001F  // R=0   G=0   B=255
#define VIOLET        0x801F  // R=128 G=0   B=255
// ************** LCD_GPIOInit ****************************
// Initializes Ports A and B for Kentec EB-LM4F120-L35
// Port A bits 4-7 are output to four control signals
// Port B bits 0-7 are output data is the data bus 
// Initialize all control signals high (off)
//  PA4     RD  Read control signal             -------------------------
//  PA5     WR  Write control signal            | PA7 | PA6 | PA5 | PA4 |
//  PA6     RS  Register/Data select signal     | CS  | RS  | WR  | RD  |
//  PA7     CS  Chip select signal              -------------------------
// wait 40 us
// In LCD.s file, students implement this in Lab 7
// ********************************************************
void LCD_GPIOInit(void);

// ************** LCD_WriteCommand ************************
// - Writes an 8-bit command to the LCD controller
// - RS low during command write
// In LCD.s file, students implement this as part of Lab 7
// 8-bit command passed in R0
// 1) LCD_DATA = 0x00;    // Write 0 as MSB of command 
// 2) LCD_CTRL = 0x10;    // Set CS, WR, RS low
// 3) LCD_CTRL = 0x70;    // Set WR and RS high
// 4) LCD_DATA = command; // Write 8-bit LSB command 
// 5) LCD_CTRL = 0x10;    // Set WR and RS low
// 6) wait 2 bus cycles     
// 7) LCD_CTRL = 0xF0;    // Set CS, WR, RS high
// ********************************************************
void LCD_WriteCommand(unsigned char command);

// ************** LCD_WriteData ***************************
// - Writes 16-bit data to the LCD controller
// - RS high during data write
// In LCD.s file, students implement this as part of Lab 7
// 16-bit data passed in R0
// 1) LCD_DATA = (data>>8);  // Write MSB to LCD data bus
// 2) LCD_CTRL = 0x50;       // Set CS, WR low
// 3) LCD_CTRL = 0x70;       // Set WR high
// 4) LCD_DATA = data;       // Write LSB to LCD data bus 
// 5) LCD_CTRL = 0x50;       // Set WR low
// 6) wait 2 bus cycles     
// 7) LCD_CTRL = 0xF0;       // Set CS, WR high
// ********************************************************
void LCD_WriteData(unsigned short data);

// ************** LCD_Init ********************************
// - Initializes the LCD
// - Command sequence verbatim from original driver
// ********************************************************
void LCD_Init(void);

// ************** convertColor ****************************
// - Converts 8-8-8 RGB values into 5-6-5 RGB
//   USE MACROS INSTEAD
// ********************************************************
unsigned short convertColor(unsigned char r, unsigned char g, unsigned char b);

// ************** LCD_ColorFill ***************************
// - Fills the screen with the specified color
// ********************************************************
void LCD_ColorFill(unsigned short color);



///////////////////////////////////////////////////////////////////////////////////////////////////
//                                 PRINTING FUNCTIONS                                            //
///////////////////////////////////////////////////////////////////////////////////////////////////

// ************** LCD_OutChar ***************************
// - Prints a character to the screen
// R0 is data, 8-bit ASCII 0x20 to 0x7F
// ********************************************************
void LCD_OutChar(unsigned char data);

// ************** LCD_OutString *************************
// - Prints a string to the screen
// R0 is pointer to null-terminated ASCII string
// ********************************************************
void LCD_OutString(char data[]);

// ************** LCD_SetCursor ***************************
// - Sets character printing cursor position
// xPos is R0 column pixel number 0 (left) to 319 (right)
// yPos is R1 row pixel number 0 (top) to 239 (bottom)
// ********************************************************
void LCD_SetCursor(unsigned short xPos, unsigned short yPos);

// ************** LCD_Goto ********************************
// - Sets character printing cursor position in terms of 
//   character positions rather than pixels. 
// - Ignores invalid position requests.
// x is R0 column number 0 (left) to 52 (right)
// y is R1 row number 0 (top) to 25 (bottom)
// ********************************************************
void LCD_Goto(unsigned char x, unsigned char y);

// ************** LCD_SetTextColor ************************
// - Sets the color that characters will be printed in
// R0 is r red
// R1 is g green
// R2 is b blue// ********************************************************
void LCD_SetTextColor(unsigned char r, unsigned char g, unsigned char b);

// ************** LCD_SetTextColorRGB ************************
// - Sets the color that characters will be printed in
// R0 is 16-bit color, 5-6-5 RGB
// ********************************************************
void LCD_SetTextColorRGB(unsigned short color);


///////////////////////////////////////////////////////////////////////////////////////////////////
//                                 DRAWING FUNCTIONS                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

// ************** LCD_DrawPixel ***************************
// - Draws a 16-bit pixel on the screen
// ********************************************************
void LCD_DrawPixel(unsigned short x, unsigned short y, unsigned short color);

// ************** LCD_DrawPixelRGB ************************
// - Draws a 16-bit representation of a 24-bit color pixel
// ********************************************************
void LCD_DrawPixelRGB(unsigned short x, unsigned short y, unsigned char r, unsigned char g, unsigned char b);

// ************** LCD_DrawLine ****************************
// - Draws a line using the Bresenham line algrorithm from
//   http://rosettacode.org/wiki/Bitmap/Bresenham%27s_line_algorithm
// ********************************************************
void LCD_DrawLine(unsigned short startX, unsigned short startY, unsigned short endX, unsigned short endY, unsigned short color);

// ************** LCD_DrawRect ****************************
// - Draws a rectangle, top left corner at (x,y)
// ********************************************************
void LCD_DrawRect(unsigned short x, unsigned short y, short width, short height, unsigned short color);

// ************** LCD_DrawFilledRect **********************
// - Draws a filled rectangle, top left corner at (x,y)
// ********************************************************
void LCD_DrawFilledRect(unsigned short x, unsigned short y, short width, short height, unsigned short color);

// ************** LCD_DrawCircle **************************
// - Draws a circle centered at (x0, y0)
// ********************************************************
void LCD_DrawCircle(unsigned short x0, unsigned short y0, unsigned short radius, short color);

// ************** LCD_DrawFilledCircle ********************
// - Draws a filled circle centered at (x0, y0)
// ********************************************************
void LCD_DrawFilledCircle(unsigned short x0, unsigned short y0, unsigned short radius, short color);

// ************** LCD_DrawImage ***************************
// - Draws an image from memory
// - Image format is a plain byte array (no metadata)
// - User must specify:
//   - pointer to image data
//   - x, y location to draw image
//   - width and height of image
//   - bpp (bits per pixel) of image
//     - currently supports 4 and 8 bpp image data
// ********************************************************
void LCD_DrawImage(const unsigned char imgPtr[], unsigned short x, unsigned short y, unsigned short width, unsigned short height, unsigned char bpp);

// ************** LCD_DrawBMP *****************************
// - Draws an image from memory
// - Image format is a BMP image stored in a byte array
// - Function attempts to resolve the following metadata
//   from the BMP format
//   - width 
//   - height
//   - bpp
//   - location of image data within bmp data
// - User must specify:
//   - pointer to image data
//   - x, y location to draw image
// ********************************************************
void LCD_DrawBMP(const unsigned char* imgPtr, unsigned short x, unsigned short y);

//-----------------------LCD_OutDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input:  32-bit unsigned number 
// Output: none
// Invariables: This function must not permanently modify registers R4 to R11
void LCD_OutDec(unsigned long data);


// -----------------------LCD_OutFix----------------------
// Output characters to LCD display in fixed-point format
// unsigned decimal, resolution 0.001, range 0.000 to 9.999 
// Inputs:  unsigned 32-bit number
// Outputs: none
// E.g., R0=0,    then output "0.000 " 
//       R0=3,    then output "0.003 " 
//       R0=89,   then output "0.089 " 
//       R0=123,  then output "0.123 " 
//       R0=9999, then output "9.999 " 
//       R0>9999, then output "*.*** "
// Invariables: This function must not permanently modify registers R4 to R11
void LCD_OutFix(unsigned long data);

#endif

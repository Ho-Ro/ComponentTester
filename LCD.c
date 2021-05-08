/* ************************************************************************
 *
 *   LCD functions
 *   - supporting a HD44780 compatible LCD module
 *     running in 4 bit data mode
 *   - if required change pin assignment in LCD.h
 *
 *   (c) 2012-2015 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define LCD_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "LCD.h"              /* LCD module specifics */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   low level functions
 * ************************************************************************ */


/*
 *  create enable pulse
 *  - LCD needs a pulse to take in data for processing
 */

void LCD_Enable(void)
{
   LCD_PORT |= (1 << LCD_EN1);     /* set enable bit */

   /* the LCD needs some time */
   /* if required adjust time according to LCD's datasheet */
   wait10us();

   LCD_PORT &= ~(1 << LCD_EN1);    /* unset enable bit */
}



/*
 *  send a byte (data or command) to the LCD
 *  - using 4 bit mode
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(unsigned char Byte)
{
  /* set upper nibble of byte */
  LCD_PORT = (LCD_PORT & 0xF0) | ((Byte >> 4) & 0x0F);

  /* give LCD some time */
  #if CPU_FREQ < 2000000
    _delay_us(5);
  #else
    wait5us();
  #endif

  LCD_Enable();          /* trigger LCD */

  /* set lower nibble of byte */ 
  LCD_PORT = (LCD_PORT & 0xF0) | (Byte & 0x0F);

  /* give LCD some time */
  #if CPU_FREQ < 2000000
    _delay_us(5);
  #else
    wait5us();
  #endif

  LCD_Enable();          /* trigger LCD */
  wait50us();            /* LCD needs some time for processing */
  LCD_PORT &= 0xF0;      /* clear data on port */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(unsigned char Cmd)
{
  LCD_PORT &= ~(1 << LCD_RS);    /* set RS to 0 (command mode) */
  LCD_Send(Cmd);                 /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(unsigned char Data)
{
  LCD_PORT |= (1 << LCD_RS);       /* set RS to 1 (data mode) */ 
  LCD_Send(Data);                  /* send data */
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  LCD_Cmd(CMD_CLEAR_DISPLAY);      /* send command */
  MilliSleep(2);                   /* LCD needs some time for processing */
}



#if 0
/*
 *  move cursor to the first position of a specified line
 *
 *  requires:
 *  - line number [1-2]
 */

void LCD_Line(unsigned char Line)
{
  uint8_t           Cmd;

  if (Line == 1)              /* line #1 */
  {
    Cmd = CMD_SET_DD_RAM_ADDR | 0x00;
  }
  else                        /* line #2 */
  {
    Cmd = CMD_SET_DD_RAM_ADDR | 0x40;
  }

  LCD_Cmd(Cmd);               /* send command */
}
#endif



/*
 *  move cursor to the first position of line #2
 */

void LCD_Line2(void)
{
  uint8_t           Cmd;

  /* command for moving cursor to start of line #2 */
  Cmd = CMD_SET_DD_RAM_ADDR | 0x40;

  LCD_Cmd(Cmd);               /* send command */
}



/*
 *  initialize LCD 
 */
 
void LCD_Init(void)
{
  /* set port pins to output mode */
  LCD_DDR = LCD_DDR | 0x0F | (1 << LCD_RS) | (1 << LCD_EN1);

  /*
   *  first we have to send three times:
   *  - RS and R/W unset
   *  - DB4 and DB5 set
   */

   /* round #1 */
   MilliSleep(30);
   LCD_PORT = (LCD_PORT & 0xF0 & ~(1 << LCD_RS)) | 0x03;
   LCD_Enable();

   /* round #2 */
   MilliSleep(5);
   LCD_Enable();

   /* round #3 */
   MilliSleep(1);
   LCD_Enable();


   /*
    *  set modes
    */

   /* init 4 bit mode  */
   MilliSleep(1);
   LCD_PORT = (LCD_PORT & 0xF0 & ~(1 << LCD_RS)) | 0x02;
   MilliSleep(1);
   LCD_Enable();
   MilliSleep(1);

   /* function set: 4 bit interface / 2 rows / font 5x7 */
   LCD_Cmd(CMD_FUNCTION_SET | 0x08);

   /* display: display on / cursor off / no blinking */
   LCD_Cmd(CMD_DISPLAY_CONTROL | 0x04);

   /* entry mode: increment cursor position / no scrolling */    
   LCD_Cmd(CMD_ENTRY_MODE_SET | 0x02);	

   /* and clear display */
   LCD_Clear();
}



/*
 *  load a custom character from EEPROM and upload it to the LCD
 *
 *  requires:
 *  - pointer of fixed character data
 *  - ID for custom character (0-7)
 */

void LCD_EELoadChar(const unsigned char *CharData, uint8_t ID)
{
  uint8_t      i;

  /*
   *  set CG RAM start address (for a 5x8 character)
   *  - lower 3 bits determine the row in a character
   *  - higher 3 bits determine the start of the character
   *  - so we have to shift the ID to the higher part
   *  - LCD module supports up to 8 custom characters for 5x8 font
   */

  LCD_Cmd(CMD_SET_CG_RAM_ADDR | (ID << 3));

  /* write custom character */
  for (i = 0; i < 8; i++)               /* do 8 times */
  {
    LCD_Data(eeprom_read_byte(CharData));    /* send byte */
    CharData++;                              /* next one */
  }
}



/* ************************************************************************
 *   high level output functions
 * ************************************************************************ */


#if 0
/*
 *  clear single line of display
 *  - by writing 20 spaces
 *  - cursor is set to first char of line
 */

void LCD_ClearLine((unsigned char Line)
{
  unsigned char     Pos;

  LCD_Line(Line);                  /* go to beginning of line */

  for (Pos = 0; Pos < 20; Pos++)   /* for 20 times */
  {
    LCD_Data(' ');                   /* send space */
  }

  LCD_Line(Line);                  /* go back to beginning of line */  
}
#endif



/*
 *  clear line #2 of display
 *  - by writing 20 spaces
 *  - cursor is set to first char of line
 */

void LCD_ClearLine2(void)
{
  unsigned char     Pos;

  LCD_Line2();                     /* go to beginning of line */

  for (Pos = 0; Pos < 20; Pos++)   /* for 20 times */
  {
    LCD_Data(' ');                   /* send space */
  }

  LCD_Line2();                     /* go back to beginning of line */  
}



/*
 *  write probe pin number to the LCD
 *  - pin TP1 -> '1'
 *  - pin TP2 -> '2'
 *  - pin TP3 -> '3'
 *
 *  requires:
 *  - testpin ID (0-2)
 */
 
void LCD_ProbeNumber(unsigned char Probe)
{
  /* since TP1 is 0 we simply add the value to '1' */
  LCD_Data('1' + Probe);           /* send data */
}



/*
 *  display a space
 */

void LCD_Space(void)
{
  LCD_Data(' ');
}



#if 0
/*
 *  display a string
 *
 *  requires:
 *  - pointer to string
 */ 
 
void LCD_String(char *String)
{
  while (*String)             /* loop until trailing 0 is reached */
  {
    LCD_Data(*String);          /* send character */
    String++;                   /* next one */
  }
}
#endif



/*
 *  display a fixed string stored in EEPROM
 *
 *  requires:
 *  - pointer to fixed string
 */

void LCD_EEString(const unsigned char *String)
{
  unsigned char     Char;

  while (1)
  {
    Char = eeprom_read_byte(String);    /* read character */

    /* check for end of string */
    if ((Char == 0) || (Char == 128)) break;

    LCD_Data(Char);                     /* send character */
    String++;                           /* next one */
  }
}



/* ************************************************************************
 *   convenience functions to save some bytes flash memory
 * ************************************************************************ */


/*
 *  display a fixed string stored in EEPROM followed by a space
 *
 *  requires:
 *  - pointer to fixed string
 */

void LCD_EEString2(const unsigned char *String)
{
  LCD_EEString(String);       /* display string */
  LCD_Space();                /* print space */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

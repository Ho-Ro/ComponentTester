/* ************************************************************************
 *
 *   LCD functions
 *   - supporting a HD44780 compatible LCD running in 4 bit data mode
 *   - if required change pin assignment in LCD.h
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

void lcd_enable(void)
{
   LCD_PORT |= (1 << LCD_EN1);     /* set enable bit */

   /* the LCD needs some time */
   /* if required adjust time according LCDs datasheet */
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

void lcd_send(unsigned char Byte)
{
  /* set upper nibble of byte */
  LCD_PORT = (LCD_PORT & 0xF0) | ((Byte >> 4) & 0x0F);

  /* give LCD some time */
  #if CPU_FREQ < 2000000
    _delay_us(5);
  #else
    wait5us();
  #endif

  lcd_enable();          /* trigger LCD */

  /* set lower nibble of byte */ 
  LCD_PORT = (LCD_PORT & 0xF0) | (Byte & 0x0F);

  /* give LCD some time */
  #if CPU_FREQ < 2000000
    _delay_us(5);
  #else
    wait5us();
  #endif

  lcd_enable();          /* trigger LCD */
  wait50us();            /* LCD needs some time for processing */
  LCD_PORT &= 0xF0;      /* clear data on port */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void lcd_command(unsigned char Cmd)
{
  LCD_PORT &= ~(1 << LCD_RS);    /* set RS to 0 (command mode) */
  lcd_send(Cmd);                 /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void lcd_data(unsigned char Data)
{
  LCD_PORT |= (1 << LCD_RS);       /* set RS to 1 (data mode) */ 
  lcd_send(Data);                  /* send data */
}



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  clear the display 
 */ 

void lcd_clear(void)
{
  lcd_command(CMD_CLEAR_DISPLAY);  /* send command */
  MilliSleep(2);                   /* LCD needs some time for processing */
}



/*
 *  move cursor to the first position of a specified line
 *
 *  requires:
 *  - line number [1-2]
 */

void lcd_line(unsigned char Line)
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

  lcd_command(Cmd);           /* send command */
}



/*
 *  clear single line of display
 *  - by writing 20 spaces
 *  - cursor is set to first char of line
 */

void lcd_clear_line(unsigned char Line)
{
  unsigned char     Pos;

  lcd_line(Line);                  /* go to beginning of line */

  for (Pos = 0; Pos < 20; Pos++)   /* for 20 times */
  {
    lcd_data(' ');                   /* send space */
  }

  lcd_line(Line);                  /* go back to beginning of line */  
}



/*
 *  initialize LCD 
 */
 
void lcd_init(void)
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
   lcd_enable();

   /* round #2 */
   MilliSleep(5);
   lcd_enable();

   /* round #3 */
   MilliSleep(1);
   lcd_enable();


   /*
    *  set modes
    */

   /* init 4 bit mode  */
   MilliSleep(1);
   LCD_PORT = (LCD_PORT & 0xF0 & ~(1 << LCD_RS)) | 0x02;
   MilliSleep(1);
   lcd_enable();
   MilliSleep(1);

   /* function set: 4 bit interface / 2 rows / font 5x7 */
   lcd_command(CMD_FUNCTION_SET | 0x08);

   /* display: display on / cursor off / no blinking */
   lcd_command(CMD_DISPLAY_CONTROL | 0x04);

   /* entry mode: increment cursor position / no scrolling */    
   lcd_command(CMD_ENTRY_MODE_SET | 0x02);	

   /* and clear display */
   lcd_clear();
}



/*
 *  load custom character from EEPROM and upload it to the LCD
 *
 *  requires:
 *  - pointer of fixed character data
 *  - ID for custom character (0-7)
 */

void lcd_fix_customchar(const unsigned char *CharData, uint8_t ID)
{
  uint8_t      i;

  /*
   *  set CG RAM start address (for a 5x8 character)
   *  - lower 3 bits determine the row in a character
   *  - higher 3 bits determine the start of the character
   *  - so we have to shift the ID to the higher part
   *  - LCD module supports up to 8 custom characters for 5x8 font
   */

  lcd_command(CMD_SET_CG_RAM_ADDR | (ID << 3));

  /* write custom character */
  for (i = 0; i < 8; i++)               /* do 8 times */
  {
    lcd_data(eeprom_read_byte(CharData));    /* send byte */
    CharData++;                              /* next one */
  }
}



/* ************************************************************************
 *   high level output functions
 * ************************************************************************ */


/*
 *  write probe pin number to the LCD
 *  - pin TP1 -> '1'
 *  - pin TP2 -> '2'
 *  - pin TP3 -> '3'
 *
 *  requires:
 *  - testpin ID (0-2)
 */
 
void lcd_testpin(unsigned char Probe)
{
  /* since TP1 is 0 we simply add the value to '1' */
  lcd_data('1' + Probe);           /* send data */
}



/*
 *  write a space to the LCD
 */

void lcd_space(void)
{
  lcd_data(' ');
}



#if 0
/*
 *  write a string to the LCD
 *
 *  requires:
 *  - pointer to string
 */ 
 
void lcd_string(char *String)
{
  while (*String)             /* loop until trailing 0 is reached */
  {
    lcd_data(*String);          /* send character */
    String++;                   /* next one */
  }
}
#endif



/*
 *  load string from EEPROM and send it to the LCD
 *
 *  requires:
 *  - pointer to fixed string
 */

void lcd_fix_string(const unsigned char *String)
{
  unsigned char     Char;

  while (1)
  {
    Char = eeprom_read_byte(String);    /* read character */

    /* check for end of string */
    if ((Char == 0) || (Char == 128)) break;

    lcd_data(Char);                     /* send character */
    String++;                           /* next one */
  }
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

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

void lcd_send(unsigned char byte)
{
  /* set upper nibble of byte */
  LCD_PORT = (LCD_PORT & 0xF0) | ((byte >> 4) & 0x0F);

  /* give LCD some time */
  #if CPU_FREQ < 2000000
    _delay_us(5);
  #else
    wait5us();
  #endif

  lcd_enable();          /* trigger LCD */

  /* set lower nibble of byte */ 
  LCD_PORT = (LCD_PORT & 0xF0) | (byte & 0x0F);

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
 
void lcd_command(unsigned char cmd)
{
  LCD_PORT &= ~(1 << LCD_RS);    /* set RS to 0 (command mode) */
  lcd_send(cmd);                 /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void lcd_data(unsigned char data)
{
 LCD_PORT |= (1 << LCD_RS);        /* set RS to 1 (data mode) */ 
 lcd_send(data);                   /* send data */
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
  wait2ms();                       /* LCD needs some time for processing */
}



/*
 *  move cursor to specified line
 *
 *  requires:
 *  - line number [1-2]
 */

void lcd_line(unsigned char Line)
{
  uint8_t           Cmd;

  /* set command and address 0x00 for line #1 */
  Cmd = CMD_SET_DD_RAM_ADDR;

  switch (Line)          /* set address based on line */
  {
    case 2:
      Cmd |= 0x40;
      break;
#if 0
    case 3:
      Cmd |= 0x14;
      break;

    case 4:
      Cmd |= 0x54;
      break;
#endif
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

  lcd_line(Line);                  /* go to beginning of line */  
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
   wait30ms();
   LCD_PORT = (LCD_PORT & 0xF0 & ~(1 << LCD_RS)) | 0x03;
   lcd_enable();

   /* round #2 */
   wait5ms();
   lcd_enable();

   /* round #3 */
   wait1ms();
   lcd_enable();


   /*
    *  set modes
    */

   /* init 4 bit mode  */
   wait1ms();
   LCD_PORT = (LCD_PORT & 0xF0 & ~(1 << LCD_RS)) | 0x02;
   wait1ms();
   lcd_enable();
   wait1ms();

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
 *  load custom character from PGM or EEPROM and upload it to the LCD
 *
 *  requires:
 *  - pointer of fixed character data
 *  - ID for custom character (0-7)
 */

void lcd_fix_customchar(const unsigned char *chardata, uint8_t ID)
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
    lcd_data(MEM_read_byte(chardata));    /* send byte */
    chardata++;                           /* next one */
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
 
void lcd_testpin(unsigned char pin)
{
  /* since TP1 is 0 we simply add the value to '1' */
  lcd_data('1' + pin);             /* send data */
}



/*
 *  write a space to the LCD
 */

void lcd_space(void)
{
  lcd_data(' ');
}



/*
 *  write a string to the LCD
 *
 *  requires:
 *  - pointer to string
 */ 
 
void lcd_string(char *string)
{
  while(*string)              /* loop until trailing 0 is reached */
  {
    lcd_data(*string);          /* send character */
    string++;                   /* next one */
  }
}



/*
 *  load string from PGM or EEPROM and send it to the LCD
 *
 *  requires:
 *  - pointer to fixed string
 */

void lcd_fix_string(const unsigned char *string)
{
  unsigned char cc;

  while(1)
  {
    cc = MEM_read_byte(string);         /* read charavter */
    if((cc == 0) || (cc == 128)) return;    /* check for end of string */
    lcd_data(cc);                       /* send character */
    string++;                           /* next one */
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

/* ************************************************************************
 *
 *   driver functions for ST7920 compatible graphic displays
 *   - 128 x 64 pixels
 *   - other resolutions are not supported because of different address
 *     to pixel mappings
 *     (64 x 32 pixels native, with ST7921s up to 256 x 64)
 *   - SPI interface (2-4 line)
 *   - 4 bit parallel interface
 *
 *   (c) 2017 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI
 *    /XRESET    LCD_RESET (optional)
 *    CS (RS)    LCD_CS (optional)
 *    SCLK (E)   LCD_SCLK
 *    SID (RW)   LCD_SID
 *    PSB        Gnd (enable serial mode)
 *    For hardware SPI LCD_SCLK and LCD_SID have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock: 2.5MHz at 4.5V / 1.6MHz at 2.7V 
 *  - write only when using SPI
 *  - doesn't allow other SPI chips on same bus (says the datasheet)
 *  - pin assignment for 4 bit parallel
 *    /XRESET    LCD_RESET (optional)
 *    E          LCD_EN
 *    RS         LCD_RS
 *    RW         Gnd or LCD_RW (optional)
 *    D4         LCD_DB4 (default: LCD_PORT Bit #0)
 *    D5         LCD_DB5 (default: LCD_PORT Bit #1)
 *    D6         LCD_DB6 (default: LCD_PORT Bit #2)
 *    D7         LCD_DB7 (default: LCD_PORT Bit #3)
 *    PSB        pull-up resistor to Vcc (enable parallel mode)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ST7920


/*
 *  local constants
 */

/* source management */
#define LCD_DRIVER_C


/*
 *  include header files
 */

/* local includes */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "ST7920.h"           /* ST7920 specifics */


/*
 *  fonts and symbols, horizontally aligned
 *  - Because of the horizontal addressing we're stuck with 8 pixels in x
 *    direction for the font. And we don't want to use a pixel screen buffer.
 */

#include "font_8x8_h.h"
#include "symbols_24x24_h.h"



/*
 *  derived constants
 */

/* number of 16 bit steps for x direction */
#define LCD_STEPS_X      (LCD_DOTS_X / 16)

/* number of lines and characters per line */
#define LCD_CHAR_X       (LCD_DOTS_X / FONT_SIZE_X)
#define LCD_CHAR_Y       (LCD_DOTS_Y / FONT_SIZE_Y)

/* component symbols */
#ifdef SW_SYMBOLS
  /* number of 16 bit steps for x direction */
  #define LCD_SYMBOL_STEPS_X  (SYMBOL_SIZE_X / 16)

  /* size in relation to a character */
  #define LCD_SYMBOL_CHAR_X   ((SYMBOL_SIZE_X + FONT_SIZE_X - 1) / FONT_SIZE_X)
  #define LCD_SYMBOL_CHAR_Y   ((SYMBOL_SIZE_Y + FONT_SIZE_Y - 1) / FONT_SIZE_Y)

  /* check x size: we need at least 3 chars */
  #if LCD_SYMBOL_CHAR_X < 3
    #error <<< Symbols too small! >>>
  #endif
#endif



/*
 *  local variables
 */

/* character matrix (copy of screen, horizontally aligned) */
unsigned char       Matrix[LCD_CHAR_X * LCD_CHAR_Y];   /* char matrix */

/* position management */
uint8_t             X_Start;       /* start position X (column in 16 bit steps) */
uint8_t             Y_Start;       /* start position Y (row) */

#ifdef SW_SYMBOLS
/* symbol positions (aligned to character positions) */
uint8_t             SymbolTop;     /* line above symbol */
uint8_t             SymbolBottom;  /* line below symbol */
uint8_t             SymbolLeft;    /* left side */
uint8_t             SymbolRight;   /* right side */
#endif



/*
 *  function declarations (prototypes)
 */

void LCD_Char(unsigned char Char);



/* ************************************************************************
 *   low level functions for 4 bit parallel interface
 * ************************************************************************ */


#ifdef LCD_PAR_4

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* bitmask */

  /*
   *  set control signals
   */

  Bits = LCD_DDR;                       /* get current directions */

  /* required pins */
  Bits |= (1 << LCD_RS) | (1 << LCD_EN) | 
          (1 << LCD_DB4) | (1 << LCD_DB5) | (1 << LCD_DB6) | (1 << LCD_DB7);

  /* optional output pins */
  #ifdef LCD_RESET
    Bits |= (1 << LCD_RESET);  
  #endif 
  #ifdef LCD_RW
    Bits |= (1 << LCD_RW);
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */

  /* LCD_EN should be low by default */

  #ifdef LCD_RESET
    /* disable reset */
    LCD_PORT |= (1 << LCD_RESET);       /* set /RESET high */
  #endif

  /* LCD_RW should be low by default (write mode) */
}



/*
 *  send a nibble to the LCD (4 bit mode)
 *
 *  requires:
 *  - nibble value to send
 */

void LCD_SendNibble(uint8_t Nibble)
{
  uint8_t           Data;

  Data = LCD_PORT;            /* get current bits */
  /* clear all 4 data lines */
  Data &= ~((1 << LCD_DB4) | (1 << LCD_DB5) | (1 << LCD_DB6) | (1 << LCD_DB7));

  #ifdef LCD_DB_STD
    /* standard pins: simply take nibble */
    Data |= Nibble;
  #else
    /* non-standard pins: set bits individually */
    if (Nibble & 0b00000001) Data |= (1 << LCD_DB4);
    if (Nibble & 0b00000010) Data |= (1 << LCD_DB5);
    if (Nibble & 0b00000100) Data |= (1 << LCD_DB6);
    if (Nibble & 0b00001000) Data |= (1 << LCD_DB7);
  #endif

  LCD_PORT = Data;            /* set nibble */

  /* enable pulse */
  LCD_PORT |= (1 << LCD_EN);       /* set EN high */

  wait1us();                       /* wait >= 140ns */

  LCD_PORT &= ~(1 << LCD_EN);      /* set EN low */

  /* data hold time >= 20ns */

  /* Enable cycle time >= 1200ns */
  wait1us();                       /* wait 1�s */
}



/*
 *  send a byte (data or command) to the LCD
 *  - send byte as two nibbles (MSB first, LSB last)
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(uint8_t Byte)
{
  uint8_t           Nibble;

  /* send upper nibble (bits 4-7) */
  Nibble = (Byte >> 4) & 0x0F;          /* get upper nibble */
  LCD_SendNibble(Nibble);

  /* send lower nibble (bits 0-3) */
  Nibble = Byte & 0x0F;                 /* get lower nibble */
  LCD_SendNibble(Nibble);
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Cmd(uint8_t Byte)
{
  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_RS);      /* set RS low */

  #ifdef LCD_RW
    /* write mode */
    LCD_PORT &= ~(1 << LCD_RW);    /* set RW low */
  #endif

  LCD_Send(Byte);                  /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Byte)
{
  /* indicate data mode */
  LCD_PORT |= (1 << LCD_RS);       /* set RS high */

  #ifdef LCD_RW
    /* write mode */
    LCD_PORT &= ~(1 << LCD_RW);    /* set RW low */
  #endif
 
  LCD_Send(Byte);                  /* send data */

  wait100us();                     /* 72�s processing delay */
}

#endif



/* ************************************************************************
 *   low level functions for SPI interface
 * ************************************************************************ */


#ifdef LCD_SPI

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* bitmask */

  /*
   *  set control signals
   */

  Bits = LCD_DDR;                       /* get current directions */

  /* optional output pins */
  #ifdef LCD_RESET
    Bits |= (1 << LCD_RESET);  
  #endif 
  #ifdef LCD_CS
    Bits |= (1 << LCD_CS);
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */

  /* LCD_CS should be low by default */

  #ifdef LCD_RESET
    /* disable reset */
    LCD_PORT |= (1 << LCD_RESET);       /* set /RESET high */
  #endif


  /*
   *  init SPI bus
   */

  #ifdef SPI_HARDWARE
  /*
   *  set SPI clock rate (max. 2.5MHz)
   */

  /* 1MHz -> f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 1
    SPI.ClockRate = SPI_CLOCK_2X;
  #endif

  /* 8MHz -> f_osc/4 (SPR1 = 0, SPR0 = 0, SPI2X = 0) */
  #if CPU_FREQ / 1000000 == 8
    SPI.ClockRate = 0
  #endif

  /* 16MHz -> f_osc/8 (SPR1 = 0, SPR0 = 1, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 16
    SPI.ClockRate = SPI_CLOCK_R0 | SPI_CLOCK_2X;
  #endif

  /* 20MHz -> f_osc/8 (SPR1 = 0, SPR0 = 1, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 20
    SPI.ClockRate = SPI_CLOCK_R0 | SPI_CLOCK_2X;
  #endif

  #endif

  SPI_Setup();                     /* set up SPI bus */
}



/*
 *  send a byte (command or data) to the LCD
 *  - hint: ST7920 doesn't take multiple bytes after the start byte.
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(uint8_t Byte)
{
  uint8_t           Nibble;

  /* upper nibble plus 4 zeros */
  Nibble = Byte & 0xF0;
  SPI_Write_Byte(Nibble);          /* write byte */

  /* lower nibble plus 4 zeros */
  Nibble = Byte << 4;
  SPI_Write_Byte(Nibble);          /* write byte */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Cmd(uint8_t Byte)
{
  uint8_t           Start;

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT |= (1 << LCD_CS);       /* set CS high */
  #endif

  /* start byte */
  Start = START_BYTE | FLAG_WRITE | FLAG_CMD;
  SPI_Write_Byte(Start);           /* write start byte */

  LCD_Send(Byte);                  /* write command */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set CS low */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Byte)
{
  uint8_t           Start;

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT |= (1 << LCD_CS);       /* set CS high */
  #endif

  /* start byte */
  Start = START_BYTE | FLAG_WRITE | FLAG_DATA;
  SPI_Write_Byte(Start);           /* write start byte */

  LCD_Send(Byte);                  /* write data */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set CS low */
  #endif

  /* todo: do we need a 72�s delay for LCD processing? */
  wait50us();                      /* 72�s processing delay */
}

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  set LCD dot position
 *  - horizontal position is based on 16 bit steps
 *  - top left: 0/0
 *
 *  requires:
 *  - x:  horizontal position (0-)
 *  - y:  vertical position (0-)
 */

void LCD_DotPos(uint8_t x, uint8_t y)
{
  uint8_t           Cmd_X;    /* command for setting x */
  uint8_t           Cmd_Y;    /* command for setting y */

  /* take care about address to pixel mapping */
  if (y < 32)          /* top LCD half (0-31) */
  {
    /* left address space is mapped to top LCD half */
    Cmd_Y = CMD_SET_GDRAM_ADDR | y;          /* vertical address */
    Cmd_X = CMD_SET_GDRAM_ADDR | x;          /* horizontal address */
  }
  else                 /* bottom LCD half (32-63) */
  {
    /* right address space is mapped to bottom LCD half */
    Cmd_Y = CMD_SET_GDRAM_ADDR | (y - 32);   /* vertical address */
    Cmd_X = CMD_SET_GDRAM_ADDR | 8 | x;      /* horizontal address */
  }

  LCD_Cmd(Cmd_Y);             /* set vertical address */
  wait100us();                /* 72�s processing delay */
  LCD_Cmd(Cmd_X);             /* set horizontal address */
  wait100us();                /* 72�s processing delay */

  /*
   *  The ST7920 seems to have a bug when setting the GDRAM address without
   *  sending any data afterwards. A second CMD_SET_GDRAM_ADDR causes a
   *  the output of 16 bits in a row. So we should call LCD_DotPos() only if
   *  we also send data.
   */
}



/*
 *  set LCD character position
 *  - top left: 1/1
 *
 *  requires:
 *  - x:  horizontal position (1-)
 *  - y:  vertical position (1-)
 */

void LCD_CharPos(uint8_t x, uint8_t y)
{
  /* update UI */
  UI.CharPos_X = x;
  UI.CharPos_Y = y;

  /* horizontal position (column in 16 bit steps) */
  x--;                             /* columns start at 0 */
  x *= FONT_SIZE_X;                /* offset for character */
  x /= 16;                         /* convert into 16 bit steps */
  X_Start = x;                     /* update start position */

  /* vertical position (row) */
  y--;                             /* rows start at 0 */
  y *= FONT_SIZE_Y;                /* offset for character */
  Y_Start = y;                     /* update start position */

  /*
   *  we don't call LCD_DotPos() here
   *  - see LCD_DotPos() for explaination
   */
}



/*
 *  clear one single character line
 *
 *  requires:
 *  - Line: line number (1-)
 *    special case line 0: clear remaining space in current line
 */ 

void LCD_ClearLine(uint8_t Line)
{
  uint8_t           n = 1;              /* counter */
  uint8_t           Temp;               /* temp. value */
  uint8_t           MatrixFlag = 1;     /* control flag */
  unsigned char     *Buffer;            /* char matrix */

  if (Line == 0)         /* special case: rest of current line */
  {
    Line = UI.CharPos_Y;      /* get current line */
    n = UI.CharPos_X;         /* get current character position */
  }

  /* check for row overflow */
  Temp = Line - 1;            /* rows start at 0 */
  Temp *= FONT_SIZE_Y;        /* offset for line */
  if (Temp > (LCD_DOTS_Y - 1)) return;  /* overflow */

  LCD_CharPos(n, Line);       /* set char position */

  if ((n % 2) == 0)           /* left neighbor */
  {
    LCD_Char(' ');            /* display space and keep neighbor */
    n++;                      /* one char done */
  }

  /* set start position in char matrix */
  Buffer = (unsigned char *)&Matrix;    /* start of matrix */
  Temp = (Line - 1) * LCD_CHAR_X;       /* offset for line */
  Temp += (n - 1);                      /* offset for column */
  Buffer += Temp;                       /* add offset for position */

  /* init row range */
  n = Y_Start;                     /* starting row */
  if (Line <= LCD_CHAR_Y)          /* within character lines */
  {
    /* clear character line */
    Line = n + FONT_SIZE_Y;        /* ending row + 1 */
  }
  else                             /* remaining rows */
  {
    /* clear up to last row */
    Line = LCD_DOTS_Y;             /* ending row + 1 */
    MatrixFlag = 0;                /* don't clear chars in matrix (overflow) */
  }

  /* clear line */
  while (n < Line)                 /* loop for rows */
  {
    LCD_DotPos(X_Start, n);        /* set new dot position */
    Temp = X_Start;                /* starting step for columns */

    while (Temp < LCD_STEPS_X)     /* loop for columns -*/
    {
      LCD_Data(0);                 /* clear 8 pixels */
      LCD_Data(0);                 /* clear another 8 pixels */
      Temp++;                      /* next 16 bit step */

      if (MatrixFlag)              /* clear char in char matrix */
      {
        *Buffer = ' ';             /* set space */
        Buffer++;                  /* next char */
        *Buffer = ' ';             /* set space */
        Buffer++;                  /* next char */
      }
    }

    MatrixFlag = 0;                /* reset flag */
    n++;                           /* next row */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           n = 1;    /* counter */

  /* we have to clear all dots manually :( */
  /* loop for all lines plus possible remaining rows */
  while (n <= (LCD_CHAR_Y + 1))
  {
    LCD_ClearLine(n);         /* clear line */
    n++;                      /* next line */
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  initialize LCD
 */
 
void LCD_Init(void)
{
  MilliSleep(40);             /* wait 40ms for Vcc to become stable */

  #ifdef LCD_RESET
  /* reset display */
  LCD_PORT &= ~(1 << LCD_RESET);        /* set /RES low */
  wait10us();                           /* wait 10�s */
  LCD_PORT |= (1 << LCD_RESET);         /* set /RES high */
  MilliSleep(1);                        /* wait 1ms */
  #endif

  /*
   *  init LCD module
   */

  #ifdef LCD_PAR_4
  /* setup for 4 bit parallel interface */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_4BIT);
  wait100us();                               /* 72�s processing delay */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_4BIT);
  wait100us();                               /* 72�s processing delay */
  #endif

  #ifdef LCD_SPI
  /* setup for SPI and 8 bit parallel interface */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_8BIT);
  wait100us();                               /* 72�s processing delay */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_8BIT);
  wait100us();                               /* 72�s processing delay */
  #endif

  /* switch display on */
  LCD_Cmd(CMD_DISPLAY | FLAG_DISPLAY_ON);
  wait100us();                               /* 72�s processing delay */

  /* clear display */
  LCD_Cmd(CMD_CLEAR);
  MilliSleep(2);                            /* 1.6ms processing delay */

  /* entry mode: left to right, no scrolling */
  LCD_Cmd(CMD_ENTRY_MODE | FLAG_INCREASE);
  wait100us();                               /* 72�s processing delay */


  /*
   *  enter graphics mode
   */

  #ifdef LCD_PAR_4
  /* enable extended instruction set */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_4BIT | FLAG_CMD_EXT);
  wait100us();                               /* 72�s processing delay */
  /* enable graphics mode */
  LCD_Cmd(CMD_EXT_FUNC_SET | FLAG_INTERFACE_4BIT | FLAG_CMD_EXT | FLAG_GFX_ON);
  wait100us();                               /* 72�s processing delay */
  #endif

  #ifdef LCD_SPI
  /* enable extended instruction set */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_INTERFACE_8BIT | FLAG_CMD_EXT);
  wait100us();                               /* 72�s processing delay */
  /* enable graphics mode */
  LCD_Cmd(CMD_EXT_FUNC_SET | FLAG_INTERFACE_8BIT | FLAG_CMD_EXT | FLAG_GFX_ON);
  wait100us();                               /* 72�s processing delay */
  #endif

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */

  LCD_Clear();                     /* clear screen */
}



/*
 *  display a single character
 *
 *  requires:
 *  - Char: character to display
 */

void LCD_Char(unsigned char Char)
{
  uint8_t           *Table1;       /* pointer to table */
  uint8_t           *Table2;       /* pointer to table */
  unsigned char     *Buffer;
  uint8_t           Index;         /* font index */
  uint16_t          Offset;        /* address offset */
  uint8_t           Neighbor;      /* neighboring character */
  uint8_t           StepFlag;      /* offset control flag */
  uint8_t           y;             /* bitmap y byte counter */
  uint8_t           Row;           /* screen row */

  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /*
   *  get bitmap of character
   */

  /* get font index number from lookup table */
  Table1 = (uint8_t *)&FontTable;       /* start address */
  Table1 += Char;                       /* add offset for character */
  Index = pgm_read_byte(Table1);        /* get index number */
  if (Index == 0xff) return;            /* no character bitmap available */

  /* calculate start address of character bitmap */
  Table1 = (uint8_t *)&FontData;        /* start address of font data */
  Offset = FONT_BYTES_N * Index;        /* offset for character */
  Table1 += Offset;                     /* address of character data */


  /*
   *  get bitmap of neighbor
   */

  /* get neighboring character from char matrix */
  StepFlag = 0;                              /* right by default */
  Index = (UI.CharPos_Y - 1) * LCD_CHAR_X;   /* offset for line */
  Index += UI.CharPos_X;                     /* offset for column (right neighbor) */

  if ((UI.CharPos_X % 2) == 0)     /* left neighbor */
  {
    StepFlag = 1;                  /* set flag */
    Index -= 2;                    /* adjust buffer offset */
  }

  Buffer = (unsigned char *)&Matrix;    /* start of matrix */
  Buffer += Index;                      /* add offset for position */
  Neighbor = *Buffer;                   /* get neighbor from matrix */

  /* get font index number from lookup table */
  Table2 = (uint8_t *)&FontTable;       /* start address */
  Table2 += Neighbor;                   /* add offset for character */
  Index = pgm_read_byte(Table2);        /* get index number */
  if (Index == 0xff) return;            /* no character bitmap available */  

  /* calculate start address of character bitmap */
  Table2 = (uint8_t *)&FontData;        /* start address of font data */
  Offset = FONT_BYTES_N * Index;        /* offset for character */
  Table2 += Offset;                     /* address of character data */


  /*
   *  display bitmaps of new and neighboring char
   */

  /* read character bitmap and send it to display */
  y = 1;
  Row = Y_Start;                        /* get start row for screen */

  while (y <= FONT_BYTES_Y)
  {
    LCD_DotPos(X_Start, Row);           /* set start position */

    /* new char */
    Index = pgm_read_byte(Table1);      /* read byte */
    Table1++;                           /* address for next byte */

    /* neighboring char */
    Neighbor = pgm_read_byte(Table2);   /* read byte */
    Table2++;                           /* address for next byte */

    /* send bytes */
    if (StepFlag)                       /* neighbor at the left */
    {
      LCD_Data(Neighbor);               /* left half */
      LCD_Data(Index);                  /* right half */
    }
    else                                /* neighbor at the right */
    {
      LCD_Data(Index);                  /* left half */
      LCD_Data(Neighbor);               /* right half */
    }

    y++;                                /* next row */
    Row++;                              /* next row */
  }

  /* update char matrix */
  if (StepFlag) Buffer++;          /* move one char to the right */
  else Buffer--;                   /* move one char to the left */
  *Buffer = Char;                  /* set new char */

  /* update character position */
  UI.CharPos_X++;                  /* next character in current line */
  if (StepFlag) X_Start++;         /* also update X dot position */
}



/*
 *  set cursor
 *
 *  required:
 *  - Mode: cursor mode
 *    0: cursor on
 *    1: cursor off
 */

void LCD_Cursor(uint8_t Mode)
{
  LCD_CharPos(LCD_CHAR_X, LCD_CHAR_Y);       /* move to bottom right */

  if (Mode)              /* cursor on */
  {
    LCD_Char('>');
  }
  else                   /* cursor off */
  {
    LCD_Char(' ');
  }
}



/* ************************************************************************
 *   fancy stuff
 * ************************************************************************ */


#ifdef SW_SYMBOLS

/*
 *  display a component symbol
 *
 *  requires:
 *  - ID: symbol to display
 */

void LCD_Symbol(uint8_t ID)
{
  uint8_t           *Table;        /* pointer to symbol table */
  uint8_t           Data;          /* symbol data */
  uint16_t          Offset;        /* address offset */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */
  uint8_t           Row;           /* screen row */
  uint8_t           StepFlag = 0;  /* offset control flag */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&SymbolData;       /* start address of symbol data */
  Offset = SYMBOL_BYTES_N * ID;         /* offset for symbol */
  Table += Offset;                      /* address of symbol data */

  Row = Y_Start;                        /* get start row for screen */

  if ((SYMBOL_BYTES_X % 2) != 0)        /* right byte of 16 bit step */
  {
    StepFlag = 1;                  /* set flag */
  }

  /* read character bitmap and send it to display */
  while (y <= SYMBOL_BYTES_Y)
  {
    LCD_DotPos(X_Start, Row);           /* set start position */    

    /* offset symbol to match 16 bit addressing step */
    if (StepFlag)
    {
      LCD_Data(0);                      /* send empty byte */
    }

    /* read and send all bytes for this row */
    x = 1;
    while (x <= SYMBOL_BYTES_X)
    {
      Data = pgm_read_byte(Table);      /* read byte */
      LCD_Data(Data);                   /* send byte */

      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    y++;                                /* next row */
    Row++;                              /* next row */
  }

  /* hint: we don't update the char position */
}



/*
 *  display fancy probe number
 *
 *  requires:
 *  - Probe: probe number
 *  - Table: pointer to pinout details
 */

void LCD_FancyProbeNumber(uint8_t Probe, uint8_t *Table)
{
  uint8_t           Data;          /* pinout data */
  uint8_t           x;             /* x position */
  uint8_t           y;             /* y position */

  Data = pgm_read_byte(Table);     /* read pinout details */

  if (Data != PIN_NONE)            /* show pin */
  {
    /* determine position based on pinout data */
    x = SymbolLeft;         /* set default positions */
    y = SymbolTop;
    if (Data & PIN_RIGHT) x = SymbolRight;
    if (Data & PIN_BOTTOM) y = SymbolBottom;

    /* show probe number */
    LCD_CharPos(x, y);           /* set position */
    LCD_ProbeNumber(Probe);      /* display probe number */
  }
}



/*
 *  show fancy pinout for semiconductors
 *  - display a nice component symbol
 *    starting in next line, aligned to right side
 *  - display pin numbers above and below the symbol
 *    because of the horizontal addressing in 16 bit steps
 *  - symbol ID (0-) in Check.Symbol
 */

void LCD_FancySemiPinout(void)
{
  uint8_t           Line;          /* line number */
  uint8_t           x, y;          /* position of char */
  uint8_t           *Table;        /* pointer to pin table */
  uint16_t          Offset;        /* address offset */

  /* save current char position */
  x = UI.CharPos_X;        /* column */
  y = UI.CharPos_Y;        /* line */

  /* check for sufficient screen size */
  Line = y + 1;                         /* next line */
  /* last line is reserved for cursor/touch bar */
  if (Line > (LCD_CHAR_Y - LCD_SYMBOL_CHAR_Y - 2)) return;  /* too few lines */

  /* determine positions */
  SymbolTop = Line;                          /* top is current line */
  SymbolBottom = Line;
  SymbolBottom += (LCD_SYMBOL_CHAR_Y + 1);   /* plus symbol's height */
  SymbolRight = LCD_CHAR_X;                  /* align to right side */
  /* minus symbol width */
  SymbolLeft = LCD_CHAR_X - LCD_SYMBOL_CHAR_X + 1;

  /* calculate start address of pinout details */
  Table = (uint8_t *)&PinTable;         /* start address of pin table */
  Offset = Check.Symbol * 3;            /* offset for pin details */
  Table += Offset;                      /* address of pin details */

  /* display probe numbers */
  LCD_FancyProbeNumber(Semi.A, Table);       /* A pin */
  Table++;                                   /* details for next pin */
  LCD_FancyProbeNumber(Semi.B, Table);       /* B pin */
  Table++;                                   /* details for next pin */
  LCD_FancyProbeNumber(Semi.C, Table);       /* C pin */

  /* display symbol */
  LCD_CharPos(SymbolLeft, SymbolTop + 1);    /* set top left position  */
  LCD_Symbol(Check.Symbol);                  /* display symbol */

  LCD_CharPos(x, y);          /* restore old char position */
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_DRIVER_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

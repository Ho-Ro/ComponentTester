/* ************************************************************************
 *
 *   driver functions for ST7565R compatible grafic displays
 *   - 128 x 64 (132 x 64) pixels
 *   - SPI interface (4 and 5 line)
 *
 *   (c) 2015-2017 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI
 *    /RES        LCD_RESET
 *    A0          LCD_A0
 *    SCL (DB6)   LCD_SCL
 *    SI (DB7)    LCD_SI
 *    /CS1        LCD_CS (optional)
 *    For hardware SPI LCD_SCL and LCD_SI have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock rate: 20MHz
 *  - write only
 *  - horizontal flip might require an offset of 4 dots
 *    (132 RAM dots - 128 real dots = 4)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ST7565R


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
#include "ST7565R.h"          /* ST7565R specifics */

/* fonts and symbols, vertically aligned, bank-wise grouping */
#include "font_8x8_v.h"
#include "symbols_24x24_vp.h"



/*
 *  derived constants
 */

/* pages/bytes required for character's height */
#define CHAR_PAGES       ((FONT_SIZE_Y + 7) / 8)

/* number of lines and characters per line */
#define LCD_CHAR_X       (LCD_DOTS_X / FONT_SIZE_X)
#define LCD_CHAR_Y       ((LCD_DOTS_Y / 8) / CHAR_PAGES)

/* component symbols */
#ifdef SW_SYMBOLS
  /* pages/bytes required for symbol's height */
  #define SYMBOL_PAGES        ((SYMBOL_SIZE_Y + 7) / 8)

  /* size in relation to a character */
  #define LCD_SYMBOL_CHAR_X   ((SYMBOL_SIZE_X + FONT_SIZE_X - 1) / FONT_SIZE_X)
  #define LCD_SYMBOL_CHAR_Y   ((SYMBOL_SIZE_Y + CHAR_PAGES * 8 - 1) / (CHAR_PAGES * 8))

  /* check y size: we need at least 2 lines */
  #if LCD_SYMBOL_CHAR_Y < 2
    #error <<< Symbols too small! >>>
  #endif
#endif

/* segment driver (ADC) direction */
#ifdef LCD_FLIP_X
  #define ADC_MODE       FLAG_ADC_REVERSE
#else
  #define ADC_MODE       FLAG_ADC_NORMAL
#endif

/* common driver direction */
#ifdef LCD_FLIP_Y
  #define COMMON_MODE    FLAG_COM_REVERSE
#else
  #define COMMON_MODE    FLAG_COM_NORMAL
#endif



/*
 *  local variables
 */

/* position management */
uint8_t             X_Start;       /* start position X (column) */
uint8_t             Y_Start;       /* start position Y (page) */

#ifdef SW_SYMBOLS
/* symbol positions (aligned to character positions) */
uint8_t             SymbolTop;     /* top line */
uint8_t             SymbolBottom;  /* bottom line */
uint8_t             SymbolLeft;    /* left of symbol */
uint8_t             SymbolRight;   /* right of symbol */
#endif



/* ************************************************************************
 *   low level functions for bit-bang SPI interface
 * ************************************************************************ */


#ifdef LCD_SPI_BITBANG

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  /* set port pins to output mode */
  #ifdef LCD_CS
    /* including /CS1 */
    LCD_DDR = LCD_DDR | (1 << LCD_RESET) | (1 << LCD_A0) | (1 << LCD_SCL) | (1 << LCD_SI) | (1 << LCD_CS);
  #else
    /* excluding /CS1 */
    LCD_DDR = LCD_DDR | (1 << LCD_RESET) | (1 << LCD_A0) | (1 << LCD_SCL) | (1 << LCD_SI);
  #endif

  /*  set default levels:
   *  - /CS1 high, if pin available
   *  - SCL high
   */

  #ifdef LCD_CS
    /* including /CS1 */
    LCD_PORT = LCD_PORT | (1 << LCD_CS) | (1 << LCD_SCL);
  #else
    /* excluding /CS1 */
    LCD_PORT = LCD_PORT | (1 << LCD_SCL);
  #endif

  /* disable reset */
  LCD_PORT = LCD_PORT | (1 << LCD_RESET);    /* set /RES high */
}



/*
 *  send a byte (data or command) to the LCD
 *  - bit-bang 8 bits, MSB first, LSB last
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(uint8_t Byte)
{
  uint8_t           n = 8;         /* counter */

  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT = LCD_PORT & ~(1 << LCD_CS);    /* set /CS1 low */
  #endif

  /* bit-bang 8 bits */
  while (n > 0)               /* 8 bits */
  {
    /* get current MSB and set SI */
    if (Byte & 0b10000000)    /* 1 */
    {
      /* set SI high */
      LCD_PORT = LCD_PORT | (1 << LCD_SI);
    }
    else                      /* 0 */
    {
      /* set SI low */
      LCD_PORT = LCD_PORT & ~(1 << LCD_SI);
    }

    /* start clock cycle (falling edge) */
    LCD_PORT = LCD_PORT & ~(1 << LCD_SCL);

    /* end clock cycle (rising edge takes bit) */
    LCD_PORT = LCD_PORT |(1 << LCD_SCL); 

    Byte <<= 1;               /* shift bits one step left */

    n--;                      /* next bit */
  }

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT = LCD_PORT | (1 << LCD_CS);     /* set /CS1 high */
  #endif
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT = LCD_PORT & ~(1 << LCD_A0);      /* set A0 low */

  LCD_Send(Cmd);              /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* indicate data mode */
  LCD_PORT = LCD_PORT | (1 << LCD_A0);       /* set A0 high */

  LCD_Send(Data);             /* send data */
}

#endif



/* ************************************************************************
 *   low level functions for hardware SPI interface
 * ************************************************************************ */


#ifdef LCD_SPI_HARDWARE


/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* bitmask */


  /*
   *  set port pin's data direction
   */

  Bits = LCD_DDR;                  /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_RESET) | (1 << LCD_A0) | (1 << LCD_SCL) | (1 << LCD_SI);

  /* optional output pins */
  #ifdef LCD_CS
    Bits |= (1 << LCD_CS);         /* /CS1 */
  #endif

  LCD_DDR = Bits;                       /* set new directions */


  /*  set default levels:
   *  - /CS1 high, if pin available
   */

  #ifdef LCD_CS
    /* disable chip */
    LCD_PORT |= (1 << LCD_CS);     /* set /CS1 high */
  #endif

  /* disable reset */
  LCD_PORT |= (1 << LCD_RESET);    /* set /RES high */


  /*
   *  set up hardware SPI
   *  - master mode
   *  - SPI mode 0 (CPOL = 0, CPHA = 0)
   *  - MSB first (DORD = 0)
   *  - polling mode (SPIE = 0)
   *  - SPI clock rate (max. 20MHz)
   *    max. MCU clock 20MHz / 2 = 10MHz
   *    f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1)
   */

  /* set mode and enable SPI */
  SPCR = (1 << SPE) | (1 << MSTR);

  /* set SPI2X for double SPI speed */
  SPSR = (1 << SPI2X);

  /* clear SPI interrupt flag, just in case */
  Bits = SPSR;           /* read flag */
  Bits = SPDR;           /* clear flag by reading data */
}



/*
 *  send a byte (data or command) to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Send(uint8_t Byte)
{
  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CS1 low */
  #endif

  /* send byte */
  SPDR = Byte;                     /* start transmission */
  while (!(SPSR & (1 << SPIF)));   /* wait for flag */
  Byte = SPDR;                     /* clear flag by reading data */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT |= (1 << LCD_CS);     /* set /CS1 high */
  #endif
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT = LCD_PORT & ~(1 << LCD_A0);      /* set A0 low */

  LCD_Send(Cmd);              /* send command */
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* indicate data mode */
  LCD_PORT = LCD_PORT | (1 << LCD_A0);       /* set A0 high */

  LCD_Send(Data);             /* send data */
}

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  set LCD dot position
 *  - since we can't read the LCD and don't use a RAM buffer
 *    we have to move page-wise in y direction
 *  - top left: 0/0
 *
 *  requires:
 *  - x:  horizontal position (0-)
 *  - y:  vertical position (0-)
 */

void LCD_DotPos(uint8_t x, uint8_t y)
{
  uint8_t           Temp;     /* temp. value */

  /* horizontal position (column) */
  Temp = x;
  Temp &= 0b00001111;              /* filter lower nibble */
  LCD_Cmd(CMD_COLUMN_L | Temp);    /* set lower nibble */
  Temp = x;
  Temp >>= 4;                      /* shift upper nibble to lower */
  Temp &= 0b00001111;              /* filter nibble */
  LCD_Cmd(CMD_COLUMN_H | Temp);    /* set upper nibble */

  /* vertical position (page) */
  LCD_Cmd(CMD_PAGE | y);           /* set page */  
}



/*
 *  set LCD character position
 *  - since we can't read the LCD and don't use a RAM buffer
 *    we have to move page-wise in y direction
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

  /* update display */

  /* horizontal position (column) */
  x--;                             /* columns starts at 0 */
  x *= FONT_SIZE_X;                /* offset for character */
  #ifdef LCD_OFFSET_X
  x += 4;                          /* x offset of 4 dots */
  #endif
  X_Start = x;                     /* update start position */

  /* vertical position (page) */
  y--;                             /* pages start at 0 */
  y *= CHAR_PAGES;                 /* offset for character */
  Y_Start = y;                     /* update start position */

  LCD_DotPos(x, y);                /* set dot position */
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
  uint8_t           MaxPage;            /* page limit */
  uint8_t           n = 1;              /* counter */

  if (Line == 0)         /* special case: rest of current line */
  {
    Line = UI.CharPos_Y;      /* get current line */
    n = UI.CharPos_X;         /* get current character position */
  }

  LCD_CharPos(n, Line);       /* set char position */

  /* calculate pages */
  Line = Y_Start;                       /* get start page */
  MaxPage = Line + CHAR_PAGES;          /* end page + 1 */

  /* clear line */
  while (Line < MaxPage)           /* loop through pages */
  {
    LCD_DotPos(X_Start, Line);     /* set dot position */

    /* clear page */
    n = X_Start;              /* reset counter */
    while (n < 132)           /* up to internal RAM size */
    {
      LCD_Data(0);            /* send empty byte */
      n++;                    /* next byte */
    }

    Line++;                   /* next page */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           n = 1;         /* counter */

  /* we have to clear all dots manually :-( */
  while (n <= LCD_CHAR_Y)          /* for all lines */
  {
    LCD_ClearLine(n);              /* clear line */
    n++;                           /* next line */
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  set contrast
 *  required:
 *  - value: 0-63
 */

void LCD_Contrast(uint8_t Contrast)
{

  if (Contrast <= 63)              /* limit value */
  {
    /* set contrast */
    LCD_Cmd(CMD_V0_MODE);
    LCD_Cmd(CMD_V0_REG | Contrast);

    NV.Contrast = Contrast;        /* update value */
  }
}



/*
 *  initialize LCD
 *  - for a single 3.3V supply
 */
 
void LCD_Init(void)
{
  /* reset display */
  LCD_PORT = LCD_PORT & ~(1 << LCD_RESET);   /* set /RES low */
  wait1us();                                 /* wait 1�s */
  LCD_PORT = LCD_PORT | (1 << LCD_RESET);    /* set /RES high */
  wait1us();                                 /* wait 1�s */

  /* set start line: user defined value (default 0) */
  LCD_Cmd(CMD_START_LINE | LCD_START_Y);

  /* set segment driver direction (ADC) */
  LCD_Cmd(CMD_SEGMENT_DIR | ADC_MODE);

  /* set common driver direction */
  LCD_Cmd(CMD_COMMON_DIR | COMMON_MODE);

  /* set LCD bias to 1/9 (duty 1/65) */
  LCD_Cmd(CMD_LCD_BIAS | FLAG_BIAS_19);

  /* set power mode: all on */
  LCD_Cmd(CMD_POWER_MODE | FLAG_FOLOWER_ON | FLAG_REGULATOR_ON | FLAG_BOOSTER_ON);

  /* set booster ratio to 4x */
  LCD_Cmd(CMD_BOOSTER_MODE);
  LCD_Cmd(CMD_BOOSTER_REG | FLAG_BOOSTER_234);

  /* set contrast: resistor ratio 6.5 */
  LCD_Cmd(CMD_V0_RATIO | FLAG_RATIO_65);

  /* set contrast: default value */
  LCD_Contrast(LCD_CONTRAST);

  /* no indicator */
  LCD_Cmd(CMD_INDICATOR_MODE);
  LCD_Cmd(CMD_INDICATOR_REG | FLAG_INDICATOR_OFF);

  /* switch display on */
  LCD_Cmd(CMD_DISPLAY | FLAG_DISPLAY_ON);

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  UI.MaxContrast = 63;             /* LCD contrast */

  LCD_Clear();                /* clear display */
}



/*
 *  display a single character
 *
 *  requires:
 *  - Char: character to display
 */

void LCD_Char(unsigned char Char)
{
  uint8_t           *Table;        /* pointer to table */
  uint8_t           Index;         /* font index */
  uint16_t          Offset;        /* address offset */
  uint8_t           Page;          /* page number */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */

  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /* get font index number from lookup table */
  Table = (uint8_t *)&FontTable;        /* start address */
  Table += Char;                        /* add offset for character */
  Index = pgm_read_byte(Table);         /* get index number */
  if (Index == 0xff) return;            /* no character bitmap available */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&FontData;        /* start address of font data */
  Offset = FONT_BYTES_N * Index;       /* offset for character */
  Table += Offset;                     /* address of character data */

  Page = Y_Start;                  /* get start page */

  /* read character bitmap and send it to display */
  while (y <= FONT_BYTES_Y)
  {
    LCD_DotPos(X_Start, Page);          /* set start position */

    /* read and send all column bytes for this row */
    x = 1;
    while (x <= FONT_BYTES_X)
    {
      Index = pgm_read_byte(Table);     /* read byte */
      LCD_Data(Index);                  /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    Page++;                             /* next page */
    y++;                                /* next row */
  }

  /* update character position */
  UI.CharPos_X++;                  /* next character in current line */
  X_Start += FONT_SIZE_X;          /* also update X dot position */
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
 *   special stuff
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
  uint8_t           Page;          /* page number */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&SymbolData;       /* start address of symbol data */
  Offset = SYMBOL_BYTES_N * ID;         /* offset for symbol */
  Table += Offset;                      /* address of symbol data */

  Page = Y_Start;                  /* get start page */

  /* read character bitmap and send it to display */
  while (y <= SYMBOL_BYTES_Y)
  {
    if (y > 1)                /* multi-page bitmap */
    {
      LCD_DotPos(X_Start, Page);        /* move to new page */
    }

    /* read and send all column bytes for this row */
    x = 1;
    while (x <= SYMBOL_BYTES_X)
    {
      Data = pgm_read_byte(Table);      /* read byte */
      LCD_Data(Data);                   /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    Page++;                             /* next page */
    y++;                                /* next row */
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
 *  - display pin numbers left and right of symbol
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
  Line = y + 1;               /* next line */
  /* last line is reserved for cursor/touch bar */
  if (Line > (LCD_CHAR_Y - LCD_SYMBOL_CHAR_Y)) return;  /* too few lines */

  /* determine positions */
  SymbolTop = Line;                          /* top is current line */
  SymbolBottom = Line;
  SymbolBottom += (LCD_SYMBOL_CHAR_Y - 1);   /* plus symbol's height */
  SymbolRight = LCD_CHAR_X;                  /* align to right side */
  /* minus symbol's width and pin IDs */
  SymbolLeft = LCD_CHAR_X - LCD_SYMBOL_CHAR_X - 1;

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
  LCD_CharPos(SymbolLeft + 1, SymbolTop);    /* set top left position  */
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

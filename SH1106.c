/* ************************************************************************
 *
 *   driver functions for SH1106 compatible OLED grafic displays
 *   - 128 x 64 (132 x 64) pixels
 *   - interfaces
 *      - 8 bit parallel in 6800 mode (not supported)
 *      - 8 bit parallel in 8080 mode (not supported)
 *      - 4 line SPI (untested)
 *      - 3 line SPI (untested)
 *      - I2C
 *
 *   (c) 2020-2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for 4 line SPI
 *    /CS         Gnd or LCD_CS (optional)
 *    /RES        Vcc or LCD_RESET (optional)
 *    A0          LCD_A0
 *    SCL (D0)    LCD_SCL / SPI_SCK
 *    SI (D1)     LCD_SI / SPI_MOSI
 *    For hardware SPI LCD_SCL and LCD_SI have to be the MCU's SCK and
 *    MOSI pins.
 *  - pin assignment for 3 line SPI
 *    /CS         Gnd or LCD_CS (optional)
 *    /RES        Vcc or LCD_RESET (optional)
 *    A0          Gnd
 *    SCL (D0)    LCD_SCL / SPI_SCK
 *    SI (D1)     LCD_SI / SPI_MOSI
 *    Bit-bang mode only!
 *  - max. SPI clock rate: 4MHz (V_DD1 > 2.4V)
 *  - pin assignment for I2C
 *    /RES            Vcc or LCD_RESET (optional)
 *    /CS             Gnd
 *    SA0 (A0)        slave address SA0 (Gnd for 0x3c / 3.3V for 0x3d)
 *    SCL (D0)        I2C_SCL
 *    SDA (D1)        I2C_SDA
 *  - max. I2C clock rate: 400kHz (fast mode)
 *  - max. clock rate for parallel bus: 3.3MHz (V_DD1 > 2.4V)
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_SH1106


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
#include "SH1106.h"           /* SH1106 specifics */

/* fonts and symbols */
/* vertically aligned, vertical bit order flipped, bank-wise grouping */
#include "font_6x8_vf.h"
#include "font_8x8_vf.h"
#include "font_8x16_vfp.h"
#include "font_6x8_iso8859-2_vf.h"
#include "font_8x8_iso8859-2_vf.h"
#include "font_8x12t_iso8859-2_vfp.h"
#include "font_8x16_iso8859-2_vfp.h"
#include "font_8x8_win1251_vf.h"
#include "font_8x8alt_win1251_vf.h"
#include "font_8x8t_win1251_vf.h"
#include "font_8x12t_win1251_vfp.h"
#include "font_8x16_win1251_vfp.h"
#include "symbols_24x24_vfp.h"

/* sanity check */
#ifndef FONT_SET
  #error <<< No font selected! >>>
#endif
#ifdef SW_SYMBOLS
  #ifndef SYMBOL_SET
    #error <<< No symbols selected! >>>
  #endif
#endif



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



/*
 *  local variables
 */

/* position management */
uint8_t             X_Start;       /* start position X (column) */
uint8_t             Y_Start;       /* start position Y (page) */



/* ************************************************************************
 *   low level functions for 4 wire SPI interface
 * ************************************************************************ */


/*
 *  protocol:
 *  - /CS -> A0 -> D7-0 with rising edge of SCLK
 *  - A0: high = data / low = command
 */

#if defined (LCD_SPI) && ! defined (SPI_9)

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set control signals
   */

  /* set directions */
  Bits = LCD_DDR;                  /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_A0);           /* A0 */

  /* optional output pins */
  #ifdef LCD_RESET
  Bits |= (1 << LCD_RESET);        /* /RES */
  #endif
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);           /* /CS */
  #endif

  LCD_DDR = Bits;                  /* set new directions */

  /* set default levels */
  #ifdef LCD_CS
  /* disable chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif

  #ifdef LCD_RESET
  /* disable reset */
  LCD_PORT |= (1 << LCD_RESET);    /* set /RES high */
  #endif


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */

  #ifdef SPI_HARDWARE

  /*
   *  set SPI clock rate (max. 4MHz)
   */

  /* 1MHz -> f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 1
  SPI.ClockRate = SPI_CLOCK_2X;
  #endif

  /* 8MHz -> f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 8
  SPI.ClockRate = SPI_CLOCK_2X;
  #endif

  /* 16MHz -> f_osc/4 (SPR1 = 0, SPR0 = 0, SPI2X = 0) */
  #if CPU_FREQ / 1000000 == 16
  SPI.ClockRate = 0;
  #endif

  /* 20MHz -> f_osc/8 (SPR1 = 0, SPR0 = 1, SPI2X = 1) */
  #if CPU_FREQ / 1000000 == 20
  SPI.ClockRate = SPI_CLOCK_R0 | SPI_CLOCK_2X;
  #endif

  SPI_Clock();                     /* update SPI clock */
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
  LCD_PORT &= ~(1 << LCD_A0);      /* set A0 low */

  /* select chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS low */
  #endif

  SPI_Write_Byte(Cmd);             /* write command byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif
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
  LCD_PORT |= (1 << LCD_A0);       /* set A0 high */

  /* select chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS low */
  #endif

  SPI_Write_Byte(Data);            /* write data byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif
}

#endif



/* ************************************************************************
 *   low level functions for 3 wire SPI interface
 * ************************************************************************ */


/*
 *  The 3 wire SPI interface ignores the D/C line and adds a D/C control
 *  bit to the SPI data resulting in a 9 bit frame:  
 *  - first bit for D/C, followed by D7 to D0
 *  - bitbang SPI only since the MCU's hardware SPI supports just bytes
 */

#if defined (LCD_SPI) && defined (SPI_BITBANG) && defined (SPI_9)

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set control signals
   */

  /* set directions */
  Bits = LCD_DDR;                  /* get current directions */

  /* optional output pins */
  #ifdef LCD_RESET
  Bits |= (1 << LCD_RESET);        /* /RES */
  #endif
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);           /* /CS */
  #endif

  LCD_DDR = Bits;                  /* set new directions */

  /* set default levels */
  #ifdef LCD_CS
  /* disable chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif

  #ifdef LCD_RESET
  /* disable reset */
  LCD_PORT |= (1 << LCD_RESET);    /* set /RES high */
  #endif


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* select chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS low */
  #endif

  SPI_Write_Bit(0);                /* indicate command (D/C=0) */
  SPI_Write_Byte(Cmd);             /* write command byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif
}


/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* select chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS low */
  #endif

  SPI_Write_Bit(1);                /* indicate data (D/C=1) */
  SPI_Write_Byte(Data);            /* write data byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
  LCD_PORT |= (1 << LCD_CS);       /* set /CS high */
  #endif
}

#endif



/* ************************************************************************
 *   low level functions for I2C interface
 * ************************************************************************ */


#ifdef LCD_I2C

/*
 *  local constants
 */

/* transfer mode */
#define CTRL_CMD         0b00000001     /* command */
#define CTRL_DATA        0b00000010     /* data */
#define CTRL_SINGLE      0b00000100     /* single byte */
#define CTRL_MULTI       0b00001000     /* multiple bytes */



/*
 *  local variables
 */

/* single/multi byte control */
uint8_t             MultiByte;     /* control flag */



/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  /* I2C is set up already in main() */

  /* optional control lines */
  #ifdef LCD_RESET
  /* set pin to output mode */
  LCD_DDR |= (1 << LCD_RESET);          /* /RES */

  /* disable reset */
  LCD_PORT |= (1 << LCD_RESET);         /* set /RES high */
  #endif

  /* set timing */
  I2C.Timeout = 1;            /* ACK timeout 10µs */

  /* init control flag */
  MultiByte = 0;              /* single byte mode */
}



/*
 *  start sending I2C data
 *  - set up I2C transfer
 *  - send control byte
 *  - manage single/multi byte mode
 *
 *  requires:
 *  - Mode:
 *    CTRL_CMD     command
 *    CTRL_DATA    data
 *    CTRL_SINGLE  single byte
 *    CTRL_MULTI   multiple bytes
 */

void LCD_StartTransfer(uint8_t Mode)
{
  uint8_t           Byte;          /* control byte */

  /* update control flag */
  if (Mode & CTRL_MULTI)           /* multi byte mode */
  {
    MultiByte = 1;                 /* set flag */
  }

  /* init control byte */
  Byte = LCD_CONTROL_BYTE;
  if (Mode & CTRL_SINGLE) Byte |= FLAG_CTRL_SINGLE;    /* single byte mode */
  if (Mode & CTRL_DATA) Byte |= FLAG_CTRL_DATA;        /* data mode */
  /* flags for multi byte mode and command mode are 0 */

  if (I2C_Start(I2C_START) == I2C_OK)             /* start */
  {
    I2C.Byte = LCD_I2C_ADDR << 1;       /* address (7 bit & write) */

    if (I2C_WriteByte(I2C_ADDRESS) == I2C_ACK)    /* address slave */
    {
      I2C.Byte = Byte;                  /* copy control byte */
      I2C_WriteByte(I2C_DATA);          /* send byte */
    }
  }

  /* todo: error handling? */
}



/*
 *  end sending I2C data
 *  - end I2C transfer
 *  - manage multi byte mode
 */

void LCD_EndTransfer(void)
{
  /* reset control flag anyway */
  MultiByte = 0;              /* single byte mode */

  /* end I2C transfer */
  I2C_Stop();                 /* stop */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  if (MultiByte == 0)         /* single byte mode */
  {
    /* init transfer */
    LCD_StartTransfer(CTRL_SINGLE | CTRL_CMD);
  }

  /* send command */
  I2C.Byte = Cmd;                  /* copy command */
  I2C_WriteByte(I2C_DATA);         /* send command */ 

  if (MultiByte == 0)         /* single byte mode */
  {
    LCD_EndTransfer();             /* end transfer */
  }
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - byte value to send
 */

void LCD_Data(uint8_t Data)
{
  if (MultiByte == 0)         /* single byte mode */
  {
    /* init transfer */
    LCD_StartTransfer(CTRL_SINGLE | CTRL_DATA);
  }

  /* send command */
  I2C.Byte = Data;                 /* copy data */
  I2C_WriteByte(I2C_DATA);         /* send data */ 

  if (MultiByte == 0)         /* single byte mode */
  {
    LCD_EndTransfer();             /* end transfer */
  }
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

  #ifdef LCD_I2C
  /* init transfer */
  LCD_StartTransfer(CTRL_MULTI | CTRL_CMD);
  #endif

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

  #ifdef LCD_I2C
  LCD_EndTransfer();               /* end transfer */
  #endif
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

  /*
   *  calculate dot position
   *  - top left of character
   */

  /* horizontal position (column) */
  x--;                             /* columns start at 0 */
  x *= FONT_SIZE_X;                /* offset for character */
  #ifdef LCD_OFFSET_X
  x += LCD_OFFSET_X;               /* add display offset */
  #endif
  X_Start = x;                     /* update start position */

  /* vertical position (page) */
  y--;                             /* pages start at 0 */
  y *= CHAR_PAGES;                 /* offset for character */
  Y_Start = y;                     /* update start position */

  /* update display */
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

  /* address limit for X */
  #ifdef LCD_OFFSET_X
    /* consider X offset */
    #define LCD_MAX_X          (LCD_DOTS_X + LCD_OFFSET_X)
  #else
    /* native X resolution */
    #define LCD_MAX_X          LCD_DOTS_X
  #endif

  /* calculate pages */
  Line = Y_Start;                       /* get start page */
  MaxPage = Line + CHAR_PAGES;          /* end page + 1 */

  /* clear line */
  while (Line < MaxPage)           /* loop through pages */
  {
    LCD_DotPos(X_Start, Line);     /* set dot position */

    #ifdef LCD_I2C
    /* init transfer */
    LCD_StartTransfer(CTRL_MULTI | CTRL_DATA);
    #endif

    /* clear page */
    n = X_Start;              /* reset counter */
    while (n < LCD_MAX_X)     /* for all columns */
    {
      LCD_Data(0);            /* send empty byte */
      n++;                    /* next byte */
    }

    #ifdef LCD_I2C
    LCD_EndTransfer();        /* end transfer */
    #endif

    Line++;                   /* next page */
  }

  /* clean up local constants */
  #undef LCD_MAX_X
}



/*
 *  clear the display 
 */

void LCD_Clear(void)
{
  uint8_t           n = 1;         /* counter */

  /* we have to clear all dots manually :( */
  while (n <= LCD_CHAR_Y)          /* for all lines */
  {
    LCD_ClearLine(n);              /* clear line */
    n++;                           /* next line */
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  set contrast
 *
 *  required:
 *  - Contrast: 0-255
 */

void LCD_Contrast(uint8_t Contrast)
{
  /* set contrast */
  LCD_Cmd(CMD_CONTRAST);           /* set command */
  LCD_Cmd(Contrast);               /* contrast value */

  NV.Contrast = Contrast;          /* update value */
}



/*
 *  initialize LCD
 */
 
void LCD_Init(void)
{
  #ifdef LCD_RESET
  /* reset display */
  LCD_PORT &= ~(1 << LCD_RESET);        /* set /RES low */
  wait10us();                           /* wait 10µs */
  LCD_PORT |= (1 << LCD_RESET);         /* set /RES high */
  wait2us();                            /* wait 2µs */
  #endif

  #if 0
  /* enable internal DC-DC converter */
  LCD_Cmd(CMD_DC_DC);
  LCD_Cmd(FLAG_DC_DC_ON);
  #endif

  /* segment mapping */
  #ifdef LCD_FLIP_X
  LCD_Cmd(CMD_SEGMENT_MAP | FLAG_SEG_131);   /* flip horizontally */
  #endif

  /* COM output scan direction */
  #ifdef LCD_FLIP_Y
  LCD_Cmd(CMD_COM_SCAN_DIR | FLAG_COM_63);   /* flip vertically */
  #endif

  /* set contrast: default value */
  LCD_Contrast(LCD_CONTRAST);

  /* switch display on */
  LCD_Cmd(CMD_DISPLAY | FLAG_DISPLAY_ON);

  /* display needs about 100ms */

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  UI.MaxContrast = 255;            /* maximum LCD contrast */
  #ifdef SW_SYMBOLS
  UI.SymbolSize_X = LCD_SYMBOL_CHAR_X;  /* x size in chars */
  UI.SymbolSize_Y = LCD_SYMBOL_CHAR_Y;  /* y size in chars */
  #endif

  LCD_Clear();                     /* clear display */
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

    #ifdef LCD_I2C
    /* init transfer */
    LCD_StartTransfer(CTRL_MULTI | CTRL_DATA);
    #endif

    /* read and send all column bytes for this row */
    x = 1;
    while (x <= FONT_BYTES_X)
    {
      Index = pgm_read_byte(Table);     /* read byte */
      LCD_Data(Index);                  /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    #ifdef LCD_I2C
    LCD_EndTransfer();        /* end transfer */
    #endif

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

    #ifdef LCD_I2C
    /* init transfer */
    LCD_StartTransfer(CTRL_MULTI | CTRL_DATA);
    #endif

    /* read and send all column bytes for this row */
    x = 1;
    while (x <= SYMBOL_BYTES_X)
    {
      Data = pgm_read_byte(Table);      /* read byte */
      LCD_Data(Data);                   /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    #ifdef LCD_I2C
    LCD_EndTransfer();        /* end transfer */
    #endif

    Page++;                             /* next page */
    y++;                                /* next row */
  }

  /* hint: we don't update the char position */
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

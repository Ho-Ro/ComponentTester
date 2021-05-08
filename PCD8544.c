/* ************************************************************************
 *
 *   driver functions for PCD8544 compatible grafic displays
 *   - aka Nokia 3310/5110 display (LPH7366)
 *   - 84 x 48 pixels
 *   - SPI interface (4 and 5 line)
 *
 *   (c) 2016-2017 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI
 *    /RES        LCD_RES (optional) 
 *    /SCE        LCD_SCE (optional)
 *    D/C         LCD_DC
 *    SCLK        LCD_SCLK
 *    SDIN        LCD_SDIN
 *    For hardware SPI LCD_SCLK and LCD_SDIN have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock rate: 4MHz
 *  - write only
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_PCD8544


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
#include "PCD8544.h"          /* PCD8544 specifics */

/* fonts and symbols */
/* vertically aligned, vertical bit order flipped, bank-wise grouping */
#include "font_6x8_vf.h"


/*
 *  derived constants
 */

/* banks/bytes required for character's height */
#define CHAR_BANKS       ((FONT_SIZE_Y + 7) / 8)

/* number of lines and characters per line */
#define LCD_CHAR_X       (LCD_DOTS_X / FONT_SIZE_X)
#define LCD_CHAR_Y       ((LCD_DOTS_Y / 8) / CHAR_BANKS)


/*
 *  local variables
 */

/* position management */
uint8_t             X_Start;       /* start position X (column) */
uint8_t             Y_Start;       /* start position Y (bank) */



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

  /* basic output pins */
  Bits |= (1 << LCD_DC);                /* D/C */

  /* optional output pins */
  #ifdef LCD_RES
    Bits |= (1 << LCD_RES);             /* /RES */
  #endif
  #ifdef LCD_SCE
    Bits |= (1 << LCD_SCE);             /* /SCE */
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */
  #ifdef LCD_SCE
    /* disable chip */
    LCD_PORT |= (1 << LCD_SCE);         /* set /SCE high */
  #endif
  #ifdef LCD_RES
    /* disable reset */
    LCD_PORT |= (1 << LCD_RES);         /* set /RES high */
  #endif

  /*
   *  todo:
   *  - Do we have the reset the display here?
   *  - time window: 30 - 100ms after V_DD?
   */

  /*
   *  init SPI bus
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

  #endif

  SPI_Setup();                     /* set up SPI bus */
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
  LCD_PORT &= ~(1 << LCD_DC);                /* set D/C low */

  /* select chip, if pin available */
  #ifdef LCD_SCE
    LCD_PORT &= ~(1 << LCD_SCE);             /* set /SCE low */
  #endif

  SPI_Write_Byte(Cmd);                       /* write command byte */

  /* deselect chip, if pin available */
  #ifdef LCD_SCE
    LCD_PORT |= (1 << LCD_SCE);              /* set /SCE high */
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
  LCD_PORT |= (1 << LCD_DC);                 /* set D/C high */

  /* select chip, if pin available */
  #ifdef LCD_SCE
    LCD_PORT &= ~(1 << LCD_SCE);             /* set /SCE low */
  #endif

  SPI_Write_Byte(Data);                      /* write data byte */

  /* deselect chip, if pin available */
  #ifdef LCD_SCE
    LCD_PORT |= (1 << LCD_SCE);              /* set /SCE high */
  #endif
}

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */


/*
 *  set LCD dot position
 *  - since we can't read the LCD and don't use a RAM buffer
 *    we have to move bank-wise in y direction
 *  - top left: 0/0
 *
 *  requires:
 *  - x:  horizontal position (0-)
 *  - y:  vertical position (0-)
 */

void LCD_DotPos(uint8_t x, uint8_t y)
{
  LCD_Cmd(CMD_ADDR_X | x);         /* set column */
  LCD_Cmd(CMD_ADDR_Y | y);         /* set bank */
}



/*
 *  set LCD character position
 *  - since we can't read the LCD and don't use a RAM buffer
 *    we have to move bank-wise in y direction
 *  - top left: 1/1
 *
 *  requires:
 *  - x:  horizontal position (1-)
 *  - y:  vertical position (1-)
 */

void LCD_CharPos(uint8_t x, uint8_t y)
{
  /* update UI (virtual position) */
  UI.CharPos_X = x;
  UI.CharPos_Y = y;

  /* update display */

  /* horizontal position (column) */
  x--;                             /* columns starts at 0 */
  x *= FONT_SIZE_X;                /* offset for character */
  X_Start = x;                     /* update start position */
  LCD_Cmd(CMD_ADDR_X | x);         /* set column */

  /* vertical position (bank) */
  y--;                             /* banks start at 0 */
  y *= CHAR_BANKS;                 /* offset for character */
  Y_Start = y;                     /* update start position */
  LCD_Cmd(CMD_ADDR_Y | y);         /* set bank */
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
  uint8_t           MaxBank;            /* bank limit */
  uint8_t           n = 1;              /* counter */  

  if (Line == 0)         /* special case: rest of current line */
  {
    Line = UI.CharPos_Y;      /* current line */
    n = UI.CharPos_X;         /* current character position */
  }

  LCD_CharPos(n, Line);       /* set char position */

  /* calculate banks */
  Line = Y_Start;                  /* get start bank */
  MaxBank = Line + CHAR_BANKS;     /* end bank + 1 */

  /* clear line */
  while (Line < MaxBank)           /* loop through banks */
  {
    LCD_DotPos(X_Start, Line);     /* set dot position */

    /* clear bank */
    n = X_Start;              /* reset counter */
    while (n < 84)            /* up to internal RAM size */
    {
      LCD_Data(0);            /* send empty byte */
      n++;                    /* next byte */
    }

    Line++;                   /* next bank */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           Bank = 0;      /* bank counter */
  uint8_t           Pos;           /* column counter */

  /* we have to clear all dots manually :-( */

  LCD_CharPos(0, 0);          /* set start address */

  while (Bank < 6)            /* loop through all 6 banks */
  {
    Pos = 0;

    while (Pos < 84)               /* for all 84 columns */
    {
      LCD_Data(0);                 /* send empty byte */
      Pos++;                       /* next column */
    }

    Bank++;                        /* next bank */           
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  set contrast
 *  required:
 *  - value: 1-127
 */

void LCD_Contrast(uint8_t Contrast)
{
  /* prevent charge pump being switched off by 0 */
  if (Contrast == 0)
  {
    Contrast++;
  }

  if (Contrast <= 127)             /* limit value */
  {
    /* set V_OP (contrast) */
    LCD_Cmd(CMD_SET_VOP | Contrast);

    NV.Contrast = Contrast;        /* update value */
  }
}



/*
 *  initialize LCD
 */
 
void LCD_Init(void)
{
  #ifdef LCD_RES
  /* reset display */
  /* max. delay for /RES after V_DD: 30ms or 100ms? */
  LCD_PORT = LCD_PORT & ~(1 << LCD_RES);     /* set /RES low */
  wait1us();                                 /* wait 1�s (needs just 100ns) */
  LCD_PORT = LCD_PORT | (1 << LCD_RES);      /* set /RES high */
  wait10us();                                /* wait 10�s */
  #endif

  /*
   *  Unfortunataly the datasheet doesn't tell anything about instruction
   *  execution times, but a source says it's 100ns for most commands.
   */ 

  /* default: display off */
  /* default: horizontal addressing mode */

  /* select extended instruction set and power on */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_CMD_EXTENDED | FLAG_DISPLAY_ON);  

  /* default: temperature coefficient 0 */
  /* default: bias 7 */

  /* set bias mode: 1:48 */
  LCD_Cmd(CMD_BIAS_SYSTEM | FLAG_BIAS_4);

  /* set contrast: default value */
  LCD_Contrast(LCD_CONTRAST);

  /* select normal instruction set */
  LCD_Cmd(CMD_FUNCTION_SET | FLAG_CMD_NORMAL | FLAG_DISPLAY_ON);

  /* set normal display mode */
  LCD_Cmd(CMD_DISP_CONTROL | FLAG_NORMAL_MODE);

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  UI.MaxContrast = 127;            /* LCD contrast */

  LCD_Clear();                /* clear display to set char position */
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
  uint8_t           Bank;          /* bank number */
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
  Table = (uint8_t *)&FontData;         /* start address of font data */
  Offset = FONT_BYTES_N * Index;        /* offset for character */
  Table += Offset;                      /* address of character data */

  Bank = Y_Start;                  /* get start bank */

  /* read character bitmap and send it to display */
  while (y <= FONT_BYTES_Y)             /* loop for Y */
  {
    LCD_DotPos(X_Start, Bank);          /* set start position */

    /* read and send all column bytes for this bank */
    x = 1;
    while (x <= FONT_BYTES_X)           /* loop for X */
    {
      Index = pgm_read_byte(Table);     /* read byte */
      LCD_Data(Index);                  /* send byte */
      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    Bank++;                             /* next bank */
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


/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef LCD_DRIVER_C

#endif

/* ************************************************************************
 *   EOF
 * ************************************************************************ */

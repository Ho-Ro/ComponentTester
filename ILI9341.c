/* ************************************************************************
 *
 *   driver functions for ILI9341 and ILI9342 compatible
 *   color graphic displays
 *   - ILI9341: 240 x 320 pixels
 *     ILI9342: 320 x 240 pixels
 *   - interfaces
 *     - 8 bit parallel in 8080-I and 8080-II mode
 *     - 9 bit parallel in 8080-I and 8080-II mode (not supported)
 *     - 16 bit parallel in 8080-I and 8080-II mode (not supported)
 *     - 18 bit parallel in 8080-I and 8080-II mode (not supported)
 *     - 4 line SPI
 *     - 3 line SPI (not supported)
 *
 *   (c) 2015-2020 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI
 *    /RES         LCD_RES (optional)
 *    /CS          LCD_CS (optional)
 *    D/C (WRX)    LCD_DC
 *    SCK (DC/X)   LCD_SCK
 *    SDI          LCD_SDI
 *    SDO          LCD_SDO (ILI9341 only, not used yet)
 *    For hardware SPI LCD_SCK and LCD_SDI have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock: 10MHz write and 6.6MHz read
 *    there are reports about 36, 48, and even 78MHz for the write mode
 *  - pin assignment for 8 bit parallel interface in 8080-I mode
 *             LCD_PORT/LCD_DDR:
 *    /RESX    LCD_RES (optional)
 *    /CSX     LCD_CS (optional)
 *    D/CX     LCD_DC
 *    WRX      LCD_WR
 *    RDX      LCD_RD
 *             LCD_PORT2/LCD_DDR2/LCD_PIN2:
 *    D0       LCD_DB0 (LCD_PORT2 pin #0)
 *    D1       LCD_DB1 (LCD_PORT2 pin #1)
 *    D2       LCD_DB2 (LCD_PORT2 pin #2)
 *    D3       LCD_DB3 (LCD_PORT2 pin #3)
 *    D4       LCD_DB4 (LCD_PORT2 pin #4)
 *    D5       LCD_DB5 (LCD_PORT2 pin #5)
 *    D6       LCD_DB6 (LCD_PORT2 pin #6)
 *    D7       LCD_DB7 (LCD_PORT2 pin #7)
 *  - max. clock rate for parallel bus
 *    15MHz write
 *    6.25MHz read register data
 *    2.2MHz read frame memory
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ILI9341


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
#include "colors.h"           /* color definitions */
#include "ILI9341.h"          /* ILI9341 specifics */

/* fonts and symbols */
/* horizontally aligned, horizontal bit order flipped */
#include "font_8x8_hf.h"
#include "font_12x16_hf.h"
#include "font_16x26_hf.h"
#include "font_10x16_iso8859-2_hf.h"
#include "font_12x16_iso8859-2_hf.h"
#include "font_16x26_iso8859-2_hf.h"
#include "font_16x26_win1251_hf.h"
#include "symbols_24x24_hf.h"
#include "symbols_32x32_hf.h"

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

/* maximum number of pixels for X and Y direction */
#ifdef LCD_ROTATE
  #define LCD_PIXELS_X        LCD_DOTS_Y
  #define LCD_PIXELS_Y        LCD_DOTS_X
#else
  #define LCD_PIXELS_X        LCD_DOTS_X
  #define LCD_PIXELS_Y        LCD_DOTS_Y
#endif

/* number of lines and characters per line */
#define LCD_CHAR_X            (LCD_PIXELS_X / FONT_SIZE_X)
#define LCD_CHAR_Y            (LCD_PIXELS_Y / FONT_SIZE_Y)

/* component symbols */
#ifdef SW_SYMBOLS
  /* resize symbols by a factor of 2 */
  #define SYMBOL_RESIZE         2

  /* size in relation to a character */
  #define LCD_SYMBOL_CHAR_X   (((SYMBOL_SIZE_X * SYMBOL_RESIZE) + FONT_SIZE_X - 1) / FONT_SIZE_X)
  #define LCD_SYMBOL_CHAR_Y   (((SYMBOL_SIZE_Y * SYMBOL_RESIZE) + FONT_SIZE_Y - 1) / FONT_SIZE_Y)

  /* check y size: we need at least 2 lines */
  #if LCD_SYMBOL_CHAR_Y < 2
    #error <<< Symbols too small! >>>
  #endif
#endif



/*
 *  local variables
 */

/* address window */
uint16_t            X_Start;       /* start position X (column) */
uint16_t            X_End;         /* end position X (column) */
uint16_t            Y_Start;       /* start position Y (page/row) */
uint16_t            Y_End;         /* end position Y (page/row) */

/* text line management */
uint16_t            LineFlags;     /* bitfield for up to 16 lines */



/* ************************************************************************
 *   low level functions for 4 line SPI interface
 * ************************************************************************ */


/*
 *  protocol:
 *  - CSX -> D/CX -> D7-0 with rising edge of SCL
 *  - D/CX: high = data / low = command
 *
 *  hints:
 *  - RGB565 and RGB666 are supported when using 4-line SPI
 */


#ifdef LCD_SPI

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

  Bits = LCD_DDR;                       /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC);                /* D/C */

  /* optional output pins */
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RES */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CS */
  #endif

  LCD_DDR = Bits;                       /* set new directions */


  /* set default levels */
  #ifdef LCD_CS
  /* disable chip */
  LCD_PORT |= (1 << LCD_CS);            /* set /CS high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  LCD_PORT |= (1 << LCD_RES);           /* set /RES high */
  #endif


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */

  #ifdef SPI_HARDWARE

  /*
   *  set SPI clock rate (10MHz worst case)
   *  - max. MCU clock 20MHz / 2 = 10MHz
   *  - f_osc/2 (SPR1 = 0, SPR0 = 0, SPI2X = 1)
   */

  SPI.ClockRate = SPI_CLOCK_2X;    /* set clock rate flags */

  SPI_Clock();                     /* update SPI clock */
  #endif
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/C low */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS1 low */
  #endif

  SPI_Write_Byte(Cmd);             /* write command byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CS1 high */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS1 low */
  #endif

  SPI_Write_Byte(Data);            /* write data byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CS1 high */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/C high */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CS1 low */
  #endif

  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */

  SPI_Write_Byte((uint8_t)Data);   /* write MSB of data */
  SPI_Write_Byte(Byte);            /* write LSB of data */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CS1 high */
  #endif
}

#endif



/* ************************************************************************
 *   low level functions for 8 bit parallel interface
 *   - LCD_PORT (LCD_DDR) for control signals
 *   - LCD_PORT2 (LCD_DDR2/LCD_PIN2) for data signals 0-7
 * ************************************************************************ */


/*
 *  protocol:
 *  - CSX -> D/CX -> D7-0 with rising edge of WRX
 *  - D/CX: high = data / low = command
 *
 *  hints:
 *  - RGB565 and RGB666 are supported when using the 8-bit parallel interface
 */


#ifdef LCD_PAR_8

/*
 *  set up interface bus
 *  - should be called at firmware startup
 */

void LCD_BusSetup(void)
{
  uint8_t           Bits;          /* register bits */


  /*
   *  set data signals
   *  - LCD_PORT2
   */

  /* all data pins are in output mode by default */
  LCD_DDR2 = 0b11111111;                /* D0-7 */


  /*
   *  set control signals
   *  - LCD_PORT
   */

  Bits = LCD_DDR;                       /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC) | (1 << LCD_WR) | (1 << LCD_RD);    /* D/CX, WRX, RDX */

  /* optional output pins */
  #ifdef LCD_RES
  Bits |= (1 << LCD_RES);               /* /RES */
  #endif 
  #ifdef LCD_CS
  Bits |= (1 << LCD_CS);                /* /CS */
  #endif

  LCD_DDR = Bits;                       /* set new directions */

  /* set default levels */
  Bits = LCD_PORT;                      /* get current levels */

  /* set WRX and RDX high */
  Bits |= (1 << LCD_WR) | (1 << LCD_RD);

  /* optional output pins */
  #ifdef LCD_CS
  /* disable chip */
  Bits |= (1 << LCD_CS);                /* set /CSX high */
  #endif
  #ifdef LCD_RES
  /* disable reset */
  Bits |= (1 << LCD_RES);               /* set /RESX high */
  #endif

  LCD_PORT = Bits;                      /* set new levels */
}



/*
 *  send a byte (data or command) to the LCD
 *
 *  requires:
 *  - Byte: byte value to send
 */

void LCD_SendByte(uint8_t Byte)
{
  /* set data signals */
  LCD_PORT2 = Byte;                /* D0-7 */

  /* create write strobe (rising edge takes data in) */
  LCD_PORT &= ~(1 << LCD_WR);      /* set WRX low */
                                   /* wait 15ns */
  LCD_PORT |= (1 << LCD_WR);       /* set WRX high */

  /* data hold time 10ns */
  /* next write cycle after 15ns WRX being high */
}



/*
 *  send a command to the LCD
 *
 *  requires:
 *  - Cmd: byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/CX low */

  /* send command */
  LCD_SendByte(Cmd);               /* send command byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: byte value to send
 */

void LCD_Data(uint8_t Data)
{
  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* send data */
  LCD_SendByte(Data);              /* send data byte */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - Data: 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CS);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* send data */
  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */
  LCD_SendByte((uint8_t)Data);     /* send MSB */
  LCD_SendByte(Byte);              /* send LSB */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif
}



#if 0

/*
 *  read byte from LCD
 *  - timing for register data
 *  - not suitable for frame memory
 *
 *  returns:
 *  - byte
 */

uint8_t LCD_ReadByte(void)
{
  uint8_t           Byte;     /* return value */

  /* set data pins to input mode */
  LCD_DDR2 = 0b00000000;           /* D0-7 */

  #ifdef LCD_CS
  /* select chip */
  LCD_PORT &= ~(1 << LCD_CD);      /* set /CSX low */
  #endif

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* start read cycle (RDX low for min. 45ns) */
  LCD_PORT &= ~(1 << LCD_RD);      /* set RDX low */

  /* wait for LCD to fetch data: max. 40ns */
  asm volatile("nop\n\t"::);       /* burn a clock cycle */

  /* read data */
  Byte = LCD_PIN2;

  /* end read cycle */
  LCD_PORT |= (1 << LCD_RD);       /* set RDX high */

  /* wait for LCD to release data lines: max. 80ns */
  #ifndef LCD_CS
  /* burn two clock cycles */
  asm volatile(
    "nop\n\t"
    "nop\n\t"
    ::
  );
  #endif

  /* next read cycle after 90ns RDX being high */

  #ifdef LCD_CS
  /* deselect chip */
  LCD_PORT |= (1 << LCD_CS);       /* set /CSX high */
  #endif

  /* set data pins back to output mode */
  LCD_DDR2 = 0b11111111;           /* D0-7 */

  return Byte;
}

#endif

#endif



/* ************************************************************************
 *   high level functions
 * ************************************************************************ */



/*
 *  set address window
 *  - 0 up to (max - 1)
 */

void LCD_AddressWindow(void)
{
  /* X -> column */
  LCD_Cmd(CMD_COL_ADDR_SET);
  LCD_Data2(X_Start);               /* start column */
  LCD_Data2(X_End);                 /* end column */

  /* Y -> page */
  LCD_Cmd(CMD_PAGE_ADDR_SET);
  LCD_Data2(Y_Start);               /* start page */
  LCD_Data2(Y_End);                 /* end page */
}



/*
 *  set LCD character position
 *
 *  requires:
 *  - x:  horizontal position (1-)
 *  - y:  vertical position (1-)
 */

void LCD_CharPos(uint8_t x, uint8_t y)
{
  uint16_t          Mask = 1;

  /* update UI */
  UI.CharPos_X = x;
  UI.CharPos_Y = y;

  y--;                        /* start at zero */

  /* mark text line as used */
  if (y < 16)                 /* prevent overflow */
  {
    Mask <<= y;               /* shift to bit position for line */
    LineFlags |= Mask;        /* set bit for line */
  }

  /*
   *  calculate dot position
   *  - top left of character
   */

  /* horizontal position (column) */
  x--;                        /* columns start at 0 */
  Mask = x;                   /* expand to 16 bit */
  Mask *= FONT_SIZE_X;        /* offset for character */
  X_Start = Mask;             /* update start position */

  /* vertical position (page) */
  Mask = y;                   /* expand to 16 bit */
  Mask *= FONT_SIZE_Y;        /* offset for character */
  Y_Start = Mask;             /* update start position */
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
  uint16_t          x;             /* x position */
  uint8_t           y;             /* y position */
  uint8_t           Pos = 1;       /* character position */

  wdt_reset();                /* reset watchdog */

  if (Line == 0)         /* special case: rest of current line */
  {
    Line = UI.CharPos_Y;           /* get current line */
    Pos = UI.CharPos_X;            /* get current character position */
  }

  /* text line optimization */
  if (Line <= 16)                  /* prevent overflow */
  {
    y = Line - 1;                  /* bitfield starts at zero */
    x = 1;                         /* set start bit */
    x <<= y;                       /* bit for this line */

    if (! (LineFlags & x))         /* bit not set */
    {
      /* empty text line, already cleared */
      return;                      /* nothing do to */
    }
    else if (Pos == 1)             /* bit set and complete line */
    {
      /* we'll clear this line completely */
      LineFlags &= ~x;             /* clear bit */
    }
  }

  /* manage address window */
  LCD_CharPos(Pos, Line);         /* update character position */
                                  /* also updates X_Start and Y_Start */
  X_End = LCD_PIXELS_X - 1;             /* last column */
  Y_End = Y_Start + FONT_SIZE_Y - 1;    /* last row */
  y = FONT_SIZE_Y;                      /* set default */

  /* partial text line at bottom of display */
  if (Y_End > (LCD_PIXELS_Y - 1))       /* row overflow */
  {
    x = Y_End - (LCD_PIXELS_Y - 1);     /* difference */
    y -= (uint8_t)x;                    /* adjust number of rows */
    Y_End = LCD_PIXELS_Y - 1;           /* set last row */    
  }

  LCD_AddressWindow();                  /* set window */

  /* clear all pixels in window */
  LCD_Cmd(CMD_MEM_WRITE);          /* start writing */

  while (y > 0)                    /* character height (pages) */
  {
    x = X_Start;                   /* reset start position */
    while (x < LCD_PIXELS_X)       /* all columns */
    {
      /* send background color */
      LCD_Data2(COLOR_BACKGROUND);

      x++;                         /* next column */
    }

    y--;                           /* next page */
  }
}



/*
 *  clear the display 
 */ 

void LCD_Clear(void)
{
  uint8_t           n = 1;         /* counter */

  /* we have to clear all dots manually :-( */
  while (n <= (LCD_CHAR_Y + 1))    /* for all text lines */
  {
    /* +1 is for a possible partial line at the bottom */

    LCD_ClearLine(n);              /* clear line */
    n++;                           /* next line */
  }

  LCD_CharPos(1, 1);          /* reset character position */
}



/*
 *  initialize LCD
 */
 
void LCD_Init(void)
{
  uint8_t           Bits;

  /* hardware reset */
  #ifdef LCD_RES
  LCD_PORT &= ~(1 << LCD_RES);               /* set /RES low */
  wait10us();                                /* wait 10µs */
  LCD_PORT |= (1 << LCD_RES);                /* set /RES high */
  /* blanking sequence needs up to 120ms */
  /* but we may send command after 5ms */
  MilliSleep(5);                             /* wait 5ms */
  #endif


  /* 
   *  set registers
   */

  #ifndef LCD_EXT_CMD_OFF
  /* power control A */
  LCD_Cmd(CMD_POWER_CTRL_A);
  LCD_Data(MASK_POWER_CTRL_A_1);        /* fixed value */
  LCD_Data(MASK_POWER_CTRL_A_2);        /* fixed value */
  LCD_Data(MASK_POWER_CTRL_A_3);        /* fixed value */
  LCD_Data(MASK_POWER_CTRL_A_4 | FLAG_REG_VD_160);     /* Vcore 1.6V */
  LCD_Data(FLAG_VBC_56);                               /* DDVDH 5.6V */

  /* power control B */  
  LCD_Cmd(CMD_POWER_CTRL_B);
  LCD_Data(MASK_POWER_CTRL_B_1);        /* fixed value */
  LCD_Data(MASK_POWER_CTRL_B_2 | FLAG_POWER_CTRL_B);
  LCD_Data(MASK_POWER_CTRL_B_3 | FLAG_DC_ON);          /* ESD protection on */

  /* pump ratio control */
  LCD_Cmd(CMD_PUMP_RATIO_CTRL);
  LCD_Data(FLAG_PUMP_RATIO_2);     /* 2xVCI */

  /* power control 1 */
  LCD_Cmd(CMD_POWER_CTRL_1);
  LCD_Data(FLAG_VRH_460);          /* GVDD 4.60V */

  /* power control 2 */
  LCD_Cmd(CMD_POWER_CTRL_2);
  LCD_Data(FLAG_BT_3);             /* DDVDH=2*VCI, VGH=6xVCI, VGL=-3*VCI */

  /* VCOM control 1 */
  LCD_Cmd(CMD_VCOM_CTRL_1);
  LCD_Data(FLAG_VMH_5000);         /* 5.000V, could be used to adjust contrast */
  LCD_Data(FLAG_VML_0600);         /* -0.600V */

  /* VCOM control 2 */
  LCD_Cmd(CMD_VCOM_CTRL_2);
  LCD_Data(FLAG_VMF_M44 | FLAG_NVM_1);       /* -44 */

  /* driver timing control A */
  LCD_Cmd(CMD_TIME_CTRL_A);
  LCD_Data(MASK_TIME_CTRL_A_1 | FLAG_NOW_1);      /* default + 1 unit */
  LCD_Data(FLAG_CR_1);                            /* default CR timing */
  LCD_Data(MASK_TIME_CTRL_A_3 | FLAG_PC_0);       /* default - 2 units */

  /* driver timing control B */
  LCD_Cmd(CMD_TIME_CTRL_B);
  LCD_Data(FLAG_VG_SW_T1_0 | FLAG_VG_SW_T2_0 | FLAG_VG_SW_T3_0 | FLAG_VG_SW_T4_0);
  LCD_Data(MASK_TIME_CTRL_B_2);                   /* fixed value */
  #endif

  /* set pixel format for RGB image data */
  LCD_Cmd(CMD_SET_PIX_FORMAT);
  LCD_Data(FLAG_DBI_16);           /* 16 Bits / RGB565 */

  #ifndef LCD_EXT_CMD_OFF
  /* frame control for normal display mode */
  LCD_Cmd(CMD_FRAME_CTRL_NORM);
  LCD_Data(FLAG_DIVA_1);           /* f_OSC */
  LCD_Data(FLAG_RTNA_24);          /* 24 clocks */
  #endif

  /* display function control */
  LCD_Cmd(CMD_FUNC_CTRL);
  LCD_Data(FLAG_PT_0);                  /* V63 / V0 - VCOML / VCOMH */
  LCD_Data(FLAG_REV_1 | FLAG_ISC_01);   /* white, 1 frame */
  LCD_Data(FLAG_NL_320);                /* 320 lines */
  LCD_Data(0x00);                       /* DOTCLK / 2 */

  #if 0
  /* gamma set */
  LCD_Cmd(CMD_GAMMA_SET);
  LCD_Data(FLAG_GC_1);             /* gamma curve 1 */
  #endif

  /* memory access control */
  LCD_Cmd(CMD_MEM_CTRL);
  #ifdef LCD_BGR
  /* reverse red and blue color channels */
  Bits = FLAG_COLOR_BGR;           /* color bit order: BGR */
  #else
  Bits = FLAG_COLOR_RGB;           /* color bit order: RGB */
  #endif
  #ifdef LCD_ROTATE
  Bits |= FLAG_XY_REV;             /* swap x and y */
  #endif
  #ifdef LCD_FLIP_X 
  Bits |= FLAG_COL_REV;            /* flip x */
  #endif
  #ifdef LCD_FLIP_Y
  Bits |= FLAG_PAGE_REV;           /* flip y */
  #endif
  LCD_Data(Bits);

  /* address window */
  X_Start = 0;
  X_End = LCD_PIXELS_X - 1;
  Y_Start = 0;
  Y_End = LCD_PIXELS_Y - 1;
  LCD_AddressWindow();

  /* power on */
  MilliSleep(120);                 /* pause for 120ms */
  LCD_Cmd(CMD_SLEEP_OUT);          /* exit sleep mode */
  MilliSleep(60);                  /* pause for 60ms */
  LCD_Cmd(CMD_DISPLAY_ON);         /* enable display output */
  MilliSleep(80);                  /* pause for 80ms */

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  #ifdef SW_SYMBOLS
  UI.SymbolSize_X = LCD_SYMBOL_CHAR_X;  /* x size in chars */
  UI.SymbolSize_Y = LCD_SYMBOL_CHAR_Y;  /* y size in chars */
  #endif

  /* init character stuff */
  LineFlags = 0xffff;           /* clear all lines by default */
  LCD_CharPos(1, 1);            /* reset character position */

  #if defined (LCD_SPI) && defined (SPI_HARDWARE)
  /* For bit-bang SPI we don't clear the display now, because it's quite slow */
  LCD_Clear();
  #endif
  #if defined (LCD_PAR_8)
  LCD_Clear();
  #endif
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
  uint8_t           Pixels;        /* pixels in y direction */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */
  uint8_t           Bits;          /* number of bits to be sent */
  uint8_t           n;             /* bitmap bit counter */

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

  /* LCD's address window */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);   /* update character position */
                                             /* also updates X_Start and Y_Start */
  X_End = X_Start + FONT_SIZE_X - 1;   /* offset for end */
  Y_End = Y_Start + FONT_SIZE_Y - 1;   /* offset for end */
  LCD_AddressWindow();                 /* set address window */

  #ifdef LCD_COLOR
  /* color feature enabled */
  Offset = UI.PenColor;                 /* get pen color */
  #else
  /* color feature disabled */
  Offset = COLOR_PEN;                   /* use default pen color */
  #endif

  LCD_Cmd(CMD_MEM_WRITE);              /* start writing */

  /* read character bitmap and send it to display */
  while (y <= FONT_BYTES_Y)
  {
    Pixels = FONT_SIZE_X;               /* track x bits to be sent */
    x = 1;                              /* reset counter */

    /* read and send all bytes for this row */
    while (x <= FONT_BYTES_X)
    {
      /* track x bits */
      if (Pixels >= 8)             /* a byte or more left */
      {
        Bits = 8;                  /* send full byte */
      }
      else                         /* less than a byte left */
      {
        Bits = Pixels;             /* send remaining bits */
      }
      Pixels -= Bits;              /* update counter */

      Index = pgm_read_byte(Table);     /* read byte */

      /* send color for each bit */
      n = Bits;
      while (n > 0)
      {
        if (Index & 0b00000001)         /* bit set */
        {
          LCD_Data2(Offset);            /* foreground color */
        }
        else                            /* bit unset */
        {
          LCD_Data2(COLOR_BACKGROUND);  /* background color */
        }

        Index >>= 1;                      /* shift byte for next bit */
        n--;                              /* next bit */
      }

      Table++;                          /* address for next byte */
      x++;                              /* next byte */
    }

    y++;                                /* next row */
  }

  UI.CharPos_X++;             /* update character position */
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
  LCD_CharPos(LCD_CHAR_X, LCD_CHAR_Y);     /* move to bottom right */

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
  uint8_t           *Table2;       /* pointer */
  uint8_t           Data;          /* symbol data */
  uint16_t          Offset;        /* address offset */
  uint8_t           Pixels;        /* pixels in x direction */
  uint8_t           x;             /* bitmap x byte counter */
  uint8_t           y = 1;         /* bitmap y byte counter */
  uint8_t           Bits;          /* number of bits to be sent */
  uint8_t           n;             /* bitmap bit counter */
  uint8_t           Factor = SYMBOL_RESIZE;  /* resize factor */

  /* calculate start address of character bitmap */
  Table = (uint8_t *)&SymbolData;       /* start address of symbol data */
  Offset = SYMBOL_BYTES_N * ID;         /* offset for symbol */
  Table += Offset;                      /* address of symbol data */

  /* LCD's address window */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);   /* update character position */
                                             /* also updates X_Start and Y_Start */
  X_End = X_Start + (SYMBOL_SIZE_X * SYMBOL_RESIZE) - 1;  /* offset for end */
  Y_End = Y_Start + (SYMBOL_SIZE_Y * SYMBOL_RESIZE) - 1;  /* offset for end */
  LCD_AddressWindow();                  /* set address window */

  #ifdef LCD_COLOR
  /* color feature enabled */
  Offset = UI.PenColor;                 /* get pen color */
  #else
  /* color feature disabled */
  Offset = COLOR_PEN;                   /* use default pen color */
  #endif

  LCD_Cmd(CMD_MEM_WRITE);               /* start writing */

  /* read character bitmap and send it to display */
  while (y <= SYMBOL_BYTES_Y)
  {
    Table2 = Table;           /* save current pointer */

    while (Factor > 0)        /* resize symbol */
    {
      Table = Table2;                   /* reset start pointer */

      Pixels = SYMBOL_SIZE_X;           /* x bits to be sent */
      x = 1;                            /* reset counter */

      /* read and send all bytes for this row */
      while (x <= SYMBOL_BYTES_X)
      {
        /* track x bits */
        if (Pixels >= 8)           /* a byte or more left */
        {
          Bits = 8;                /* send full byte */
        }
        else                       /* less than a byte left */
        {
          Bits = Pixels;           /* send remaining bits */
        }
        Pixels -= Bits;            /* update counter */

        Data = pgm_read_byte(Table);    /* read byte */

        /* send color for each bit */
        n = Bits;                       /* reset counter */
        n *= SYMBOL_RESIZE;             /* and consider resize factor */

        while (n > 0)                   /* x pixels */
        {
          if (Data & 0b00000001)             /* bit set */
          {
            LCD_Data2(Offset);               /* foreground color */
          }
          else                               /* bit unset */
          {
            LCD_Data2(COLOR_BACKGROUND);     /* background color */
          }

          n--;                          /* next pixel */

          if (n % SYMBOL_RESIZE == 0)   /* for every resize step */
          {
            Data >>= 1;                 /* shift byte for next bit */
          }
        }

        Table++;                        /* address for next byte */
        x++;                            /* next byte */
      }

      Factor--;                    /* one y resizing step done */
    }

    /* manage y direction */
    if (Factor == 0)               /* all y resizing steps done */
    {
      Factor = SYMBOL_RESIZE;      /* reset resize factor */
      y++;                         /* next row */
    }              
  }

  /* mark text lines as used */
  n = LCD_SYMBOL_CHAR_Y;           /* set line counter */
  x = UI.SymbolPos_Y;              /* start line */
  while (n > 1)                    /* first line already set */
  {
    x++;                           /* next line */
    LCD_CharPos(1, x);             /* mark line */
    n--;                           /* next line */
  }
}

#endif



#ifdef FUNC_COLORCODE

/*
 *  draw filled box
 *  - takes X_start, X_End, Y_Start and Y_End as coordinates 
 *
 *  requires:
 *  - Color: RGB565 color code
 */

void LCD_Box(uint16_t Color)
{
  uint16_t          x_Size;             /* x size */
  uint16_t          x;                  /* x counter */
  uint16_t          y_Size;             /* y size/counter */

  LCD_AddressWindow();             /* set address window */

  /* calculate sizes */
  x_Size = X_End - X_Start + 1;
  y_Size = Y_End - Y_Start + 1;

  LCD_Cmd(CMD_MEM_WRITE);          /* start writing */

  while (y_Size > 0)               /* loop trough rows */
  {
    x = x_Size;                    /* reset counter */    

    while (x > 0)                  /* loop through columns */
    {
      LCD_Data2(Color);            /* send color code for dot */
      x--;                         /* next one */
    }

    y_Size--;                      /* next one */
  }
}



/*
 *  display color band of a component color code
 *  - aligned to charactor position
 *  - size: 2x1 chars
 *
 *  requires:
 *  - Color: RGB565 color code
 *  - Align: ALIGN_LEFT   align band left
 *           ALIGN_RIGHT  align band right
 */

void LCD_Band(uint16_t Color, uint8_t Align)
{
  /* prevent x overflow */
  if (UI.CharPos_X > LCD_CHAR_X) return;

  /* update character position, also updates X_Start and Y_Start */
  LCD_CharPos(UI.CharPos_X, UI.CharPos_Y);

  /*
   *  box for component body
   *  - height: nearly one char
   *    top and bottom margin: 1/8 char heigth
   *  - width: two chars
   */
 
  X_End = X_Start + (2 * FONT_SIZE_X) - 1;   /* offset for end */
  /* offset for end & bottom margin */
  Y_End = Y_Start + FONT_SIZE_Y - 1 - (FONT_SIZE_Y / 8);   
  Y_Start += (FONT_SIZE_Y / 8);              /* top margin */

  /* draw body using the component's body color */
  LCD_Box(COLOR_CODE_NONE);


  /*
   *  box for band
   *  - heigth: same as body but -1 dot at top and bottom
   *  - width: 1 char
   *    left and right margin: 1/3 char width
   */

  /* create thin outline */
  Y_Start += 1;
  Y_End -= 1;

  if (Align == ALIGN_LEFT)    /* align band left */
  {
    X_Start += (FONT_SIZE_X / 3);       /* left margin */
    X_End = X_Start + FONT_SIZE_X - 1;  /* offset for end */
  }
  else                        /* align band right */
  {
    X_End -= (FONT_SIZE_X / 3);         /* right margin */
    X_Start = X_End - FONT_SIZE_X + 1;  /* offset for start */
  }

  /* draw band */
  LCD_Box(Color);

  UI.CharPos_X += 2;          /* update character position */
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

/* ************************************************************************
 *
 *   driver functions for ST7735 compatible color graphic displays
 *   - 128 x 160 (132 x 162) pixels
 *   - interfaces
 *     - 8, 9, 16 and 18 bit parallel (not supported)
 *     - 3 line SPI (not supported)
 *     - 4 line SPI
 *
 *   (c) 2016-2021 by Markus Reschke
 *
 * ************************************************************************ */

/*
 *  hints:
 *  - pin assignment for SPI (4 wire)
 *    /RESX    LCD_RES (optional)
 *    /CSX     LCD_CS (optional)
 *    D/CX     LCD_DC
 *    SCL      LCD_SCL / SPI_SCK
 *    SDA      LCD_SDA / SPI_MOSI
 *    For hardware SPI LCD_SCL and LCD_SDA have to be the MCU's SCK and
 *    MOSI pins.
 *  - max. SPI clock: 15.1MHz write, 6.6MHz read 
 */


/* local includes */
#include "config.h"           /* global configuration */

#ifdef LCD_ST7735


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
#include "ST7735.h"           /* ST7735 specifics */

/* fonts and symbols */
/* horizontally aligned, horizontal bit order flipped */
#include "font_8x8_hf.h"
#include "font_10x16_hf.h"
#include "font_6x8_iso8859-2_hf.h"
#include "font_8x8_iso8859-2_hf.h"
#include "font_10x16_iso8859-2_hf.h"
#include "font_8x16_win1251_hf.h"
#include "font_8x16alt_win1251_hf.h"
#include "symbols_24x24_hf.h"
#include "symbols_30x32_hf.h"

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

/* number of pixels for X and Y direction, and display offsets */
#ifdef LCD_ROTATE
  /* swap X and Y */
  #define LCD_PIXELS_X        LCD_DOTS_Y
  #define LCD_PIXELS_Y        LCD_DOTS_X
  #ifdef LCD_OFFSET_X
    #define LCD_SHIFT_Y       LCD_OFFSET_X   /* shift y by 2 or 4 dots */
  #endif
  #ifdef LCD_OFFSET_Y
    #define LCD_SHIFT_X       LCD_OFFSET_Y   /* shift x by 1 or 2 dots */
  #endif
#else
  /* keep X and Y */
  #define LCD_PIXELS_X        LCD_DOTS_X
  #define LCD_PIXELS_Y        LCD_DOTS_Y
  #ifdef LCD_OFFSET_X
    #define LCD_SHIFT_X       LCD_OFFSET_X   /* shift x by 2 or 4 dots */
  #endif
  #ifdef LCD_OFFSET_Y
    #define LCD_SHIFT_Y       LCD_OFFSET_Y   /* shift y by 1 or 2 dots */
  #endif
#endif

/* number of lines and characters per line */
#define LCD_CHAR_X            (LCD_PIXELS_X / FONT_SIZE_X)
#define LCD_CHAR_Y            (LCD_PIXELS_Y / FONT_SIZE_Y)

/* component symbols */
#ifdef SW_SYMBOLS
  /* resize symbols by a factor of 1 */
  #define SYMBOL_RESIZE         1

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
uint16_t            Y_Start;       /* start position Y (row) */
uint16_t            Y_End;         /* end position Y (row) */

/* text line management */
uint16_t            LineFlags;     /* bitfield for up to 16 lines */



/* ************************************************************************
 *   low level functions for 4 wire SPI interface
 * ************************************************************************ */


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

  /* set directions */
  Bits = LCD_DDR;                  /* get current directions */

  /* basic output pins */
  Bits |= (1 << LCD_DC);           /* D/C */

  /* optional output pins */
  #ifdef LCD_RES
    Bits |= (1 << LCD_RES);        /* /RESX */
  #endif 
  #ifdef LCD_CS
    Bits |= (1 << LCD_CS);         /* /CSX */
  #endif

  LCD_DDR = Bits;                  /* set new directions */


  /* set default levels */
  #ifdef LCD_CS
    /* disable chip */
    LCD_PORT |= (1 << LCD_CS);          /* set /CSX high */
  #endif
  #ifdef LCD_RES
    /* disable reset */
    LCD_PORT |= (1 << LCD_RES);         /* set /RESX high */
  #endif


  /*
   *  init SPI bus
   *  - SPI bus is set up already in main()
   */

  #ifdef SPI_HARDWARE

  /*
   *  set SPI clock rate (max. 15 MHz)
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
 *  - byte value to send
 */
 
void LCD_Cmd(uint8_t Cmd)
{
  /* indicate command mode */
  LCD_PORT &= ~(1 << LCD_DC);      /* set D/CX low */

  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CSX low */
  #endif

  SPI_Write_Byte(Cmd);             /* write command byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT |= (1 << LCD_CS);     /* set /CSX high */
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
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CSX low */
  #endif

  SPI_Write_Byte(Data);            /* write data byte */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT |= (1 << LCD_CS);     /* set /CSX high */
  #endif
}



/*
 *  send data to the LCD
 *
 *  requires:
 *  - 2-byte value to send
 */

void LCD_Data2(uint16_t Data)
{
  uint8_t           Byte;     /* data byte */

  /* indicate data mode */
  LCD_PORT |= (1 << LCD_DC);       /* set D/CX high */

  /* select chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT &= ~(1 << LCD_CS);    /* set /CSX low */
  #endif

  Byte = (uint8_t)Data;            /* save LSB */
  Data >>= 8;                      /* get MSB */

  SPI_Write_Byte((uint8_t)Data);   /* write MSB of data */
  SPI_Write_Byte(Byte);            /* write LSB of data */

  /* deselect chip, if pin available */
  #ifdef LCD_CS
    LCD_PORT |= (1 << LCD_CS);     /* set /CSX high */
  #endif
}

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

  /* Y -> row */
  LCD_Cmd(CMD_ROW_ADDR_SET);
  LCD_Data2(Y_Start);               /* start row */
  LCD_Data2(Y_End);                 /* end row */
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

  y--;                        /* rows start at zero */

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
  #ifdef LCD_SHIFT_X
  Mask += LCD_SHIFT_X;        /* add display offset */
  #endif
  X_Start = Mask;             /* update start position */

  /* vertical position (row) */
  Mask = y;                   /* expand to 16 bit */
  Mask *= FONT_SIZE_Y;        /* offset for character */
  #ifdef LCD_SHIFT_Y
  Mask += LCD_SHIFT_Y;        /* add display offset */
  #endif
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
  /* address limit for X */
  #ifdef LCD_SHIFT_X
    /* consider X offset */
    #define LCD_MAX_X          (LCD_PIXELS_X + LCD_SHIFT_X)
  #else
    /* native X resolution */
    #define LCD_MAX_X          LCD_PIXELS_X
  #endif

  /* address limit for Y */
  #ifdef LCD_SHIFT_Y
    /* consider Y offset */
    #define LCD_MAX_Y          (LCD_PIXELS_Y + LCD_SHIFT_Y)
  #else
    /* native Y resolution */
    #define LCD_MAX_Y          LCD_PIXELS_Y
  #endif

  X_End = LCD_MAX_X - 1;                /* last column */
  Y_End = Y_Start + FONT_SIZE_Y - 1;    /* last row */
  y = FONT_SIZE_Y;                      /* set default */

  /* partial text line at bottom of display */
  if (Y_End > (LCD_MAX_Y - 1))          /* row overflow */
  {
    x = Y_End - (LCD_MAX_Y - 1);        /* difference */
    y -= (uint8_t)x;                    /* adjust number of rows */
    Y_End = LCD_MAX_Y - 1;              /* set last row */    
  }

  LCD_AddressWindow();                  /* set window */

  /* clear all pixels in window */
  LCD_Cmd(CMD_MEM_WRITE);          /* start writing */

  while (y > 0)                    /* character height (pages) */
  {
    x = X_Start;                   /* reset start position */
    while (x < LCD_MAX_X)          /* all columns */
    {
      /* send background color */
      LCD_Data2(COLOR_BACKGROUND);

      x++;                         /* next column */
    }

    y--;                           /* next page */
  }

  /* clean up local constants */
  #undef LCD_MAX_X
  #undef LCD_MAX_Y
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
  LCD_PORT = LCD_PORT & ~(1 << LCD_RES);     /* set /RESX low */
  wait10us();                                /* wait 10µs */
  LCD_PORT = LCD_PORT | (1 << LCD_RES);      /* set /RESX high */
  /* blanking sequence needs up to 120ms */
  /* but we may send command after 5ms */
  MilliSleep(5);                             /* wait 5ms */
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
    Bits |= FLAG_XY_REV;           /* swap x and y */
  #endif
  #ifdef LCD_FLIP_X 
    Bits |= FLAG_COL_REV;          /* flip x */
  #endif
  #ifdef LCD_FLIP_Y
    Bits |= FLAG_ROW_REV;          /* flip y */
  #endif
  LCD_Data(Bits);

  /* set pixel format for RGB image data */
  LCD_Cmd(CMD_PIX_FORMAT);
  LCD_Data(FLAG_IFPF_16);          /* 16 Bits per pixel */

  /* address window */
  X_Start = 0;
  X_End = LCD_PIXELS_X - 1;
  Y_Start = 0;
  Y_End = LCD_PIXELS_Y - 1;
  LCD_AddressWindow();

  /* power on */
  MilliSleep(115);                 /* pause for 120ms (blanking sequence) */
  LCD_Cmd(CMD_SLEEP_OUT);          /* leave sleep mode */
  MilliSleep(120);                 /* pause for 120ms (booster & clocks) */
  #ifndef LCD_LATE_ON
  /* turn on display early as visual feedback */
  LCD_Cmd(CMD_DISPLAY_ON);         /* enable display output */
  #endif

  /* update maximums */
  UI.CharMax_X = LCD_CHAR_X;       /* characters per line */
  UI.CharMax_Y = LCD_CHAR_Y;       /* lines */
  #ifdef SW_SYMBOLS
  UI.SymbolSize_X = LCD_SYMBOL_CHAR_X;  /* x size in chars */
  UI.SymbolSize_Y = LCD_SYMBOL_CHAR_Y;  /* y size in chars */
  #endif

  LineFlags = 0xffff;              /* clear all lines by default */
  LCD_Clear();                     /* clear display */
  #ifdef LCD_LATE_ON
  /* turn on display after clearing it */
  LCD_Cmd(CMD_DISPLAY_ON);         /* enable display output */  
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

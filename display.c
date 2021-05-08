/* ************************************************************************
 *
 *   common display functions for LCD modules
 *
 *   (c) 2015-2018 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define DISPLAY_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   display functions
 * ************************************************************************ */


/*
 *  next line automation
 *  - move to next line
 *  - clear display if no lines are left
 *  - controlled by global variable UI.LineMode
 *
 *  Flags:
 *  - LINE_STD   move to next line,
 *               clear display when last line is exceeded
 *  - LINE_KEY   same as LINE_STD,
 *               but also wait for test key/timeout
 *  - LINE_KEEP  keep first line when clearing the display
 */

void LCD_NextLine(void)
{
  uint8_t           Mode;          /* line mode */
  uint8_t           Line;          /* line number */

  Mode = UI.LineMode;              /* get current mode */
  Line = UI.CharPos_Y;             /* get current line number */

  /* check if we reached the last line */
  if (Line == UI.CharMax_Y)
  {
    if (Mode & LINE_KEY) WaitKey();     /* wait for key press */

    /* clear screen */
    if (Mode & LINE_KEEP)          /* keep first line */
    {
      Line = UI.CharMax_Y;         /* start at the last line */
      while (Line > 1)
      {
        LCD_ClearLine(Line);       /* clear line */
        Line--;                    /* next line */
      }

      LCD_CharPos(1, 2);           /* move to second line */
    }
    else                           /* clear complete screen */
    {
      LCD_Clear();                 /* clear screen */
    }
  }
  else
  {
    /* simply move to the next line */
    Line++;                        /* add one line */
    LCD_CharPos(1, Line);          /* move to new line */
  }

  #ifdef UI_SERIAL_COPY
  if (UI.OP_Mode & OP_SER_COPY)    /* copy to serial enabled */
  {
    Serial_NewLine();              /* serial: new line */
  }
  #endif
}



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
    if (Char == 0) break;

    LCD_Char(Char);                     /* send character */
    String++;                           /* next one */
  }
}



/* ************************************************************************
 *   convenience functions to save some bytes of flash memory
 * ************************************************************************ */


/*
 *  display probe number
 *  - probe-1 -> '1'
 *  - probe-2 -> '2'
 *  - probe-3 -> '3'
 *
 *  requires:
 *  - probe/testpin ID (0-2)
 */
 
void LCD_ProbeNumber(uint8_t Probe)
{
  #ifdef SW_PROBE_COLORS
  uint16_t          Color;         /* color value */

  Color = UI.PenColor;               /* save current color */
  UI.PenColor = ProbeColors[Probe];  /* set probe color */
  #endif

  /* since probe-1 is 0 we simply add the value to ASCII '1' */
  LCD_Char('1' + Probe);           /* send char */

  #ifdef SW_PROBE_COLORS
  UI.PenColor = Color;             /* restore old color */
  #endif
}



/*
 *  clear line #2 of display
 *  - cursor is set to first char of line
 */

void LCD_ClearLine2(void)
{
  LCD_ClearLine(2);           /* clear line #2 */
  LCD_CharPos(1, 2);          /* move to beginning of line #2 */
}



/*
 *  display a space
 */

void LCD_Space(void)
{
  LCD_Char(' ');         /* print a space */
}



/*
 *  display a fixed string stored in EEPROM followed by a space
 *
 *  requires:
 *  - pointer to fixed string
 */

void LCD_EEString_Space(const unsigned char *String)
{
  LCD_EEString(String);       /* display string */
  LCD_Space();                /* print space */
}



/*
 *  set line mode
 *
 *  requires:
 *  - Mode: mode flags
 */

void LCD_NextLine_Mode(uint8_t Mode)
{
  UI.LineMode = Mode;
}



/*
 *  move to the next line and
 *  display a fixed string stored in EEPROM
 *
 *
 *  requires:
 *  - pointer to fixed string
 */

void LCD_NextLine_EEString(const unsigned char *String)
{
  LCD_NextLine();
  LCD_EEString(String);       /* display string */
}



/*
 *  move to the next line and
 *  display a fixed string stored in EEPROM followed by a space
 *
 *  requires:
 *  - pointer to fixed string
 */

void LCD_NextLine_EEString_Space(const unsigned char *String)
{
  LCD_NextLine();
  LCD_EEString(String);       /* display string */
  LCD_Space();                /* print space */
}



/* ************************************************************************
 *   fancy display functions
 * ************************************************************************ */


#ifdef SW_SYMBOLS

/*
 *  external variables
 */

/* pin position lookup table from symbol_xy.h */
extern const uint8_t PinTable[];



/*
 *  display fancy probe number
 *  - display pin numbers left and right of symbol
 *
 *  requires:
 *  - Probe: probe number
 *  - Index: pin index (0-2)
 */

void LCD_FancyProbeNumber(uint8_t Probe, uint8_t Index)
{
  uint8_t           *Table;        /* pointer to pin table */
  uint8_t           Data;          /* pinout data */
  uint8_t           x;             /* x position */
  uint8_t           y;             /* y position */

  /* calculate start address of pinout details */
  Table = (uint8_t *)&PinTable;    /* start address of pin table */
  x = Check.Symbol * 3;            /* offset for symbol's pin details */
  Table += x;                      /* start address of symbol's pin details */
  Table += Index;                  /* address of specific pin's details */

  Data = pgm_read_byte(Table);     /* read pinout details */

  if (Data != PIN_NONE)            /* show pin */
  {
    /* determine position based on pinout data */
    /* set default position (top left) */
    x = UI.SymbolPos_X - 1;        /* left of symbol */
    y = UI.SymbolPos_Y;            /* top line of symbol */
    if (Data & PIN_RIGHT)          /* right */
    {
      x += UI.SymbolSize_X + 1;    /* right of symbol */
    }
    if (Data & PIN_BOTTOM)         /* bottom */
    {
      y += UI.SymbolSize_Y - 1;    /* bottom line of symbol */
    }

    /* show probe number */
    LCD_CharPos(x, y);           /* set position */
    LCD_ProbeNumber(Probe);      /* display probe number */
  }
}



/*
 *  show fancy pinout for semiconductors
 *  - display a nice component symbol
 *    standard case: aligned to right side
 *  - display pin numbers left and right of symbol
 *  - symbol ID (0-) in Check.Symbol
 *
 *  requires:
 *  - Line: starting line of symbol (top, 1-)
 */

void LCD_FancySemiPinout(uint8_t Line)
{
  uint8_t           Pos;           /* position of symbol */
  uint8_t           MaxLine;       /* maximum line number */
  #ifdef LCD_COLOR
  uint16_t          Color;         /* pen color */ 
  #endif

  Pos = UI.SymbolSize_Y;           /* get symbol height */
  MaxLine = UI.CharMax_Y;          /* get screen heigth */

  /* check for sufficient screen size */
  if (MaxLine > Pos)               /* sanity check for size */
  {
    /*
     *  check if we got enough lines left on the screen
     *  - standard case: last line is reserved for cursor
     */

    Pos += Line;                   /* line below symbol */

    /* check for narrow displays */
    if (UI.CharMax_X < 16)         /* less than 16 chars per line */
    {
      Pos = MaxLine;               /* trigger output on new screen */
    }

    if (Pos >= MaxLine)       /* doesn't fit on current screen */
    {
      /* output on a new screen */
      UI.CharPos_Y = MaxLine;      /* simulate last line */
      LCD_NextLine();              /* trigger test key & clear screen */
      Line = 2;                    /* simply line #2 */
      Pos = 3;                     /* align to left side with tiny offset */
    }
    else                      /* fit's on current screen */
    {
      /* default x position */
      Pos = UI.CharMax_X - UI.SymbolSize_X;  /* align to right side */
    }

    /*
     *  display symbol with pinout
     */

    /* determine start position (top left of symbol) */
    UI.SymbolPos_X = Pos;               /* x position */
    UI.SymbolPos_Y = Line;              /* y position */

    /* display probe numbers */
    LCD_FancyProbeNumber(Semi.A, 0);    /* A pin */
    LCD_FancyProbeNumber(Semi.B, 1);    /* B pin */
    LCD_FancyProbeNumber(Semi.C, 2);    /* C pin */

    /* display symbol */
    #ifdef LCD_COLOR
      Color = UI.PenColor;              /* save color */
      UI.PenColor = COLOR_SYMBOL;       /* set pen color */
    #endif

    LCD_CharPos(UI.SymbolPos_X, UI.SymbolPos_Y);  /* set position */
    LCD_Symbol(Check.Symbol);                     /* display symbol */

    #ifdef LCD_COLOR
      UI.PenColor = Color;              /* restore pen color */
    #endif

    /* hint: we don't restore the old char position */
  }
}

#endif



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef DISPLAY_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   common display functions for LCD modules
 *
 *   (c) 2015-2017 by Markus Reschke
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
 *   convenience functions to save some bytes flash memory
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
 *   clean-up of local constants
 * ************************************************************************ */

/* source management */
#undef DISPLAY_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

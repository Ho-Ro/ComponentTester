/* ************************************************************************
 *
 *   common display functions and common functions for LCD modules
 *
 *   (c) 2015-2021 by Markus Reschke
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
 *   display of characters and strings
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

void Display_NextLine(void)
{
  uint8_t           Mode;          /* line mode */
  uint8_t           Line;          /* line number */

  /*
   *  display module
   */

  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  if (Cfg.OP_Control & OP_OUT_LCD)      /* copy to LCD enabled */
  {
  #endif

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

  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  }
  #endif


  /*
   *  TTL serial
   */

  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  if (Cfg.OP_Control & OP_OUT_SER)      /* copy to serial enabled */
  {
    Serial_NewLine();                   /* serial: new line */
  }
  #endif
}



#ifdef UI_KEY_HINTS

/*
 *  last line automation for key hint
 */

void Display_LastLine(void)
{
  uint8_t           Line;          /* line number */

  /*
   *  display module
   */

  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  if (Cfg.OP_Control & OP_OUT_LCD)      /* copy to LCD enabled */
  {
  #endif

  Line = UI.CharPos_Y;             /* get current line number */

  /* check if we reached the last line */
  if (Line == UI.CharMax_Y)         
  {
    WaitKey();                     /* wait for key press */
    LCD_ClearLine(Line);           /* clear last line */
    LCD_CharPos(1, Line);          /* move to start of last line */
    MilliSleep(500);               /* smooth UI */
  }

  #if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)
  }
  #endif
}

#endif



/*
 *  display a single character
 *  - wrapper for outputting a character to different channels
 *  - if we have only the display we simply create an alias for
 *    LCD_Char() in functions.h to save a few bytes
 *
 *  requires:
 *  - Char: character
 */

#if defined (UI_SERIAL_COPY) || defined (UI_SERIAL_COMMANDS)

void Display_Char(unsigned char Char)
{
  /*
   *  display module
   */

  if (Cfg.OP_Control & OP_OUT_LCD)      /* copy to LCD enabled */
  {
    LCD_Char(Char);                     /* send char to display */
  }


  /*
   *  TTL serial
   */

  if (Cfg.OP_Control & OP_OUT_SER)      /* copy to serial enabled */
  {
    Serial_Char(Char);                  /* send char to serial */
  }
}

#endif



/*
 *  display a fixed string stored in EEPROM/Flash
 *
 *  requires:
 *  - pointer to fixed string
 */

void Display_EEString(const unsigned char *String)
{
  unsigned char     Char;

  /* read characters until we get the terminating 0 */
  while ((Char = DATA_read_byte(String)))
  {
    Display_Char(Char);                 /* send character */
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
 
void Display_ProbeNumber(uint8_t Probe)
{
  #ifdef UI_PROBE_COLORS
  uint16_t          Color;         /* color value */

  Color = UI.PenColor;               /* save current color */
  UI.PenColor = ProbeColors[Probe];  /* set probe color */
  #endif

  /* since probe-1 is 0 we simply add the value to ASCII '1' */
  Display_Char('1' + Probe);       /* send char */

  #ifdef UI_PROBE_COLORS
  UI.PenColor = Color;             /* restore old color */
  #endif
}



/*
 *  display a space
 */

void Display_Space(void)
{
  Display_Char(' ');          /* print a space */
}



/*
 *  display a minus sign
 */

void Display_Minus(void)
{
  Display_Char('-');          /* print a minus */
}



/*
 *  display a colon
 */

void Display_Colon(void)
{
  Display_Char(':');          /* print a minus */
}



/*
 *  display a fixed string stored in EEPROM followed by a space
 *
 *  requires:
 *  - pointer to fixed string
 */

void Display_EEString_Space(const unsigned char *String)
{
  Display_EEString(String);   /* display string */
  Display_Space();            /* print space */
}



/*
 *  move to the next line and
 *  display a fixed string stored in EEPROM
 *
 *  requires:
 *  - pointer to fixed string
 */

void Display_NL_EEString(const unsigned char *String)
{
  Display_NextLine();
  Display_EEString(String);   /* display string */
}



/*
 *  move to the next line and
 *  display a fixed string stored in EEPROM followed by a space
 *
 *  requires:
 *  - pointer to fixed string
 */

void Display_NL_EEString_Space(const unsigned char *String)
{
  Display_NextLine();
  Display_EEString(String);   /* display string */
  Display_Space();            /* print space */
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



#ifdef UI_SERIAL_COPY

/*
 *  enable output to TTL serial and send newline
 *  - output to display and serial
 */

void Display_Serial_On(void)
{
  Serial_NewLine();                /* serial: new line */
  Cfg.OP_Control |= OP_OUT_SER;    /* enable output to serial */
}



/*
 *  disable output to TTL serial and send newline
 *  - keep output to display enabled
 */

void Display_Serial_Off(void)
{
  Cfg.OP_Control &= ~OP_OUT_SER;   /* disable output to serial */
  Serial_NewLine();                /* serial: new line */
}

#endif



#if defined (UI_SERIAL_COMMANDS) || defined (SW_DISPLAY_REG)

/*
 *  set output to TTL serial
 *  - and disable output to display
 */

void Display_Serial_Only(void)
{
  Cfg.OP_Control &= ~OP_OUT_LCD;   /* disable display output */
  Cfg.OP_Control |= OP_OUT_SER;    /* enable serial output */
}



/*
 *  set output to display
 *  - and disable output to TTL serial
 */

void Display_LCD_Only(void)
{
  Cfg.OP_Control &= ~OP_OUT_SER;   /* disable serial output */
  Cfg.OP_Control |= OP_OUT_LCD;    /* enable display output */
}

#endif



#ifdef UI_SERIAL_COMMANDS

/*
 *  display a fixed string stored in EEPROM
 *  and move to next line
 *
 *  requires:
 *  - pointer to fixed string
 */

void Display_EEString_NL(const unsigned char *String)
{
  Display_EEString(String);   /* display string */
  Display_NextLine();         /* new line */
}

#endif



#ifdef UI_COLORED_TITLES

/*
 *  display a fixed string stored in EEPROM using a specific color
 *
 *  requires:
 *  - pointer to fixed string
 *  - color
 */

void Display_ColoredEEString(const unsigned char *String, uint16_t Color)
{
  uint16_t               OldColor;

  OldColor = UI.PenColor;     /* get current color */
  UI.PenColor = Color;        /* set new color */

  Display_EEString(String);   /* display string */

  UI.PenColor = OldColor;     /* reset color to old one */
}



/*
 *  display a fixed string stored in EEPROM using a specific color and
 *  folowed by a space
 *
 *  requires:
 *  - pointer to fixed string
 *  - color
 */

void Display_ColoredEEString_Space(const unsigned char *String, uint16_t Color)
{
  Display_ColoredEEString(String, Color);    /* display string */
  Display_Space();                           /* print space */
}


/*
 *  set pen color to COLOR_TITLE
 */

void Display_UseTitleColor(void)
{
  UI.PenColor = COLOR_TITLE;       /* set pen color */
}



/*
 *  set pen color to COLOR_PEN
 */

void Display_UsePenColor(void)
{
  UI.PenColor = COLOR_PEN;         /* reset pen color */
}

#endif



/* ************************************************************************
 *   display of values and units
 * ************************************************************************ */


#if defined (FUNC_DISPLAY_HEXBYTE) || defined (FUNC_DISPLAY_HEXVALUE)

/*
 *  display single hexadecimal digit
 *
 *  requires:
 *  - value to display (0-15)
 */

void Display_HexDigit(uint8_t Digit)
{
  /*
   *  convert value into hex digit:
   *  - 0-9: ascii 48-57
   *  - A-F: ascii 65-70
   *    a-f: ascii 97-102
   */

  if (Digit < 10)             /* 0-9 */
  {
    /* char '0' is ASCII 48 */
    Digit += 48;                   /* char 0-9 */
  }
  else                        /* a-f */
  {
    #ifdef UI_HEX_UPPERCASE
      /* char 'A' is ASCII 65 */
      Digit += (65 - 10);          /* char A-F */
    #else
      /* char 'a' is ASCII 97 */
      Digit += (97 - 10);          /* char a-f */
    #endif
  }

  Display_Char(Digit);        /* display digit */
}

#endif



#ifdef FUNC_DISPLAY_HEXBYTE

/*
 *  display byte as hexadecimal number
 *
 *  requires:
 *  - value to display (0-255)
 */

void Display_HexByte(uint8_t Value)
{
  uint8_t           Digit;

  /* first digit */
  Digit = Value / 16;
  Display_HexDigit(Digit);

  /* second digit */
  Digit = Value % 16;
  Display_HexDigit(Digit);
}

#endif



#ifdef FUNC_DISPLAY_HEXVALUE

/*
 *  display value as hexadecimal number
 *
 *  requires:
 *  - Value: value to display
 *  - Bits: max. bit depth of value (1-16)
 */

void Display_HexValue(uint16_t Value, uint8_t Bits)
{
  uint8_t           Nibble[4];     /* nibbles */
  uint8_t           n;             /* counter */

  /* calculate number of nibbles/digits */
  Bits += 3;                  /* for a partial nibble */
  Bits /= 4;                  /* number of nibbles */

  /* get nibbles (from LSB to MSB) */
  n = 0;
  while (n < Bits)
  {
    Nibble[n] = Value & 0x000F;    /* get LSB nibble */
    Value >>= 4;                   /* shift one nibble */
    n++;                           /* next nibble */
  }

  /* output nibbles (from MSB to LSB) */
  n = Bits;
  while (n > 0)
  {
    Display_HexDigit(Nibble[n - 1]);
    n--;                           /* next nibble */
  }
}

#endif



#if defined (FUNC_DISPLAY_FULLVALUE) || defined (FUNC_DISPLAY_SIGNEDFULLVALUE)

/*
 *  display unsigned value plus unit
 *  - outputs all digits
 *
 *  requires:
 *  - Value: unsigned value
 *  - DecPlaces: decimal places (0 = none)
 *  - Unit: character for unit (0 = none)
 */

void Display_FullValue(uint32_t Value, uint8_t DecPlaces, unsigned char Unit)
{
  uint8_t           n;                  /* counter */
  uint8_t           Length;             /* string length */
  uint8_t           Pos = 0;            /* position of dot */

  /* convert value into string */
  ultoa(Value, OutBuffer, 10);          /* radix 10: max. 10 chars + /0 */
  Length = strlen(OutBuffer);           /* get string length */

  /* determine position of dot */
  if (DecPlaces == 0)                   /* no dot requested */
  {
    Pos = 100;                          /* disable dot */
  }
  else if (Length >= DecPlaces)         /* position within string */
  {
    Pos = Length - DecPlaces;           /* position of dot */
    DecPlaces = 0;                      /* no additional zeros needed */
  }
  else                                  /* position outside string */
  {
    DecPlaces = DecPlaces - Length;     /* number of zeros after dot */
  }

  /* leading zero (followed by dot) */
  if (Pos == 0) Display_Char('0');      /* display: 0 */

  /* display digits */
  n = 0;
  while (n < Length)                    /* process string */
  {
    if (n == Pos)                       /* at position of dot */
    {
      #ifdef UI_COMMA
      Display_Char(',');                /* display: , */
      #else
      Display_Char('.');                /* display: . */
      #endif

      while (DecPlaces > 0)             /* fill in more zeros if needed */
      {
        Display_Char('0');              /* display: 0 */
        DecPlaces--;
      }
    }

    Display_Char(OutBuffer[n]);         /* display digit */
    n++;                                /* next digit */
  }

  /* display unit */
  if (Unit) Display_Char(Unit);
}

#endif



#ifdef FUNC_DISPLAY_SIGNEDFULLVALUE

/*
 *  display signed value plus unit
 *  - outputs all digits
 *
 *  requires:
 *  - Value: signed value
 *  - DecPlaces: decimal places (0 = none)
 *  - Unit: character for unit (0 = none)
 */

void Display_SignedFullValue(int32_t Value, uint8_t DecPlaces, unsigned char Unit)
{
  /* take care about sign */
  if (Value < 0)              /* negative value */
  {
    Display_Minus();          /* display: "-" */
    Value = -Value;           /* make value positive */
  }

  /* and display unsigned value */
  Display_FullValue((uint32_t)Value, DecPlaces, Unit);
}

#endif



/*
 *  display unsigned value plus unit (character)
 *  - scales value to max. 4 digits excluding "." and unit
 *
 *  requires:
 *  - unsigned value
 *  - exponent of factor related to base unit (value * 10^x)
 *    e.g: p = 10^-12 -> -12
 *  - unit character (0 = none)
 */

void Display_Value(uint32_t Value, int8_t Exponent, unsigned char Unit)
{
  unsigned char     Prefix = 0;         /* prefix character */
  uint8_t           Offset = 0;         /* exponent offset to lower 10^3 step */
  uint8_t           Index;              /* index ID */
  uint8_t           Length;             /* string length */

  /* scale value down to 4 digits */
  while (Value >= 10000)
  {
    Value += 5;                       /* for automagic rounding */
    Value = Value / 10;               /* scale down by 10^1 */
    Exponent++;                       /* increase exponent by 1 */
  } 


  /*
   *  determine prefix and offset (= number of digits right of dot)
   */

  if (Exponent >= -15)                  /* prevent index underflow */
  {
    Exponent += 15;                     /* shift exponent to be >= 0 */
    Index = Exponent / 3;               /* number of 10^3 steps */
    Offset = Exponent % 3;              /* offset to lower 10^3 step */

    if (Offset > 0)                     /* dot required */
    {
      Index++;                          /* upscale prefix */ 
      Offset = 3 - Offset;              /* reverse value (1 or 2) */
    }    

    /* look up prefix in table */
    if (Index < NUM_PREFIXES)           /* prevent array overflow */
    {
      Prefix = DATA_read_byte(&Prefix_table[Index]);
    }
  }


  /*
   *  display value
   */

  /* convert value into string */
  utoa((uint16_t)Value, OutBuffer, 10);   /* radix 10: max. 5 chars + /0 */
  Length = strlen(OutBuffer);             /* get string length */

  /* we misuse Exponent for the dot position */
  Exponent = Length - Offset;           /* calculate position */

  if (Exponent <= 0)                    /* we have to prepend "0." */
  {
    /* 0: factor 10 / -1: factor 100 */
    Display_Char('0');                  /* display: 0 */
    #ifdef UI_COMMA
    Display_Char(',');                  /* display: , */
    #else
    Display_Char('.');                  /* display: . */
    #endif
    if (Exponent < 0)                   /* factor 100 */
    {
      Display_Char('0');                /* additional 0 */
    }
  }

  if (Offset == 0)                 /* no dot needed */
  {
    Exponent = -1;                 /* disable dot */
  }

  /* adjust position to match array or disable dot if set to 0 */ 
  Exponent--;

  /* display value and add dot if requested */
  Index = 0;
  while (Index < Length)                /* loop through string */
  {
    Display_Char(OutBuffer[Index]);     /* display char/digit */

    if (Index == Exponent)              /* starting point of decimal fraction */
    {
      #ifdef UI_COMMA
      Display_Char(',');                /* display: , */
      #else
      Display_Char('.');                /* display: . */
      #endif
    }

    Index++;                            /* next one */
  }

  /* display prefix and unit */
  if (Prefix) Display_Char(Prefix);
  if (Unit) Display_Char(Unit);
}



/*
 *  display signed value and unit
 *  - max. 4 digits excluding sign, "." and unit
 *
 *  requires:
 *  - signed value
 *  - exponent of factor related to base unit (value * 10^x)
 *    e.g: p = 10^-12 -> -12
 *  - unit character (0 = none)
 */

void Display_SignedValue(int32_t Value, int8_t Exponent, unsigned char Unit)
{
  /* take care about sign */
  if (Value < 0)              /* negative value */
  {
    Display_Minus();          /* display: "-" */
    Value = -Value;           /* make value positive */
  }

  /* and display unsigned value */
  Display_Value((uint32_t)Value, Exponent, Unit);
}



#ifdef FUNC_EVALUE

/*
 *  display E norm value
 *
 *  requires:
 *  - Value: unsigned value (2 or 3 digits)
 *  - Scale: exponent/multiplier (10^n with n >= -12)
 *  - Unit: unit character
 */

void Display_EValue(uint16_t Value, int8_t Scale, unsigned char Unit)
{
  uint8_t           Offset = 0;         /* exponent offset to lower 10^3 step */
  int8_t            Temp;               /* temporary value */

  /* calculate exponent offset to lower 10^3 step */
  Temp = Scale;                    /* copy exponent */
  Temp += 12;                      /* shift exponent to be >= 0 */
  Offset = Temp % 3;               /* offset to lower 10^3 step */

  /*
   *  Since Display_Value() scales values to the next higher 10^3 step
   *  we have to beautify some special cases to prevent things like
   *  0.15M (150k looks much nicer).
   */

  if ((Offset == 1) && (Value < 100))      /* 0.01u and 0.11u */  
  {
    /* scale value to lower 10^3 step */
    Value *= 10;              /* *10 */
    Scale--;                  /* /10 */
  }
  #if 0
  else if ((Offset == 2) && (Value < 10))  /* 0.1u */
  {
    /* scale value to lower 10^3 step */
    Value *= 100;             /* *100 */
    Scale -= 2;               /* /100 */
  }
  #endif

  Display_Value(Value, Scale, Unit);       /* display value */
}

#endif



#ifdef FUNC_EIA96

/*
 *  display EIA-96 code
 *
 *  requires:
 *  - Index: index number of norm value (1-96)
 *  - Scale: exponent/multiplier (10^n with n >= -12)
 */

void Display_EIA96(uint8_t Index, int8_t Scale)
{
  uint8_t           n;             /* counter */
  unsigned char     MultCode;      /* multiplier code */

  /*
   *  value code (index number of value)
   *  - 2 digits
   *  - prepend 0 for single digit number 
   */

  if (Index < 10)                  /* single digit */
  {
    Display_Char('0');             /* display: 0 */
  }
  Display_FullValue(Index, 0, 0);  /* display index number */


  /*
   *  multiplier code
   *  - 0.001  0.01  0.1  1  10   100  1k  10k  100k
   *    Z      Y/R   X/S  A  B/H  C    D   E    F
   */

  n = 0;                           /* reset index counter */

  /* get table index for multiplier */
  while (Scale > -3)               /* 10^-3 / 0.001 */
  {
    Scale--;                       /* decrease multiplier */
    n++;                           /* increase index */
  }

  /* read multiplier code */
  MultCode = DATA_read_byte(&EIA96_Mult_table[n]);

  Display_Char(MultCode);          /* display multiplier code */
}

#endif



/* ************************************************************************
 *   fancy pinout
 * ************************************************************************ */


#ifdef SW_SYMBOLS

/*
 *  external variables
 */

/* pin position lookup table from symbols_<size>_<format>.h */
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
    Display_ProbeNumber(Probe);  /* display probe number */
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
      Display_NextLine();          /* trigger test key & clear screen */
      Line = 2;                    /* simply line #2 */
      Pos = 3;                     /* align to left side with tiny offset */
    }
    else                      /* fits on current screen */
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
 *   display related menu functions
 * ************************************************************************ */


#ifdef SW_CONTRAST

/*
 *  change LCD contrast
 *  - takes maximum value into account (UI.MaxContrast)
 */

void ChangeContrast(void)
{
  uint8_t          Flag = 1;            /* loop control */
  uint8_t          Contrast;            /* contrast value */
  uint8_t          Max;                 /* contrast maximum */
  

  /*
   *  increase: short key press / right turn 
   *  decrease: long key press / left turn
   *  done:     two brief key presses          
   */

  LCD_Clear();
  Display_EEString_Space(Contrast_str);      /* display: Contrast */

  Contrast = NV.Contrast;          /* get current value */
  Max = UI.MaxContrast;            /* get maximum value */

  while (Flag)
  {
    LCD_ClearLine2();
    Display_Value(Contrast, 0, 0);

    #ifdef HW_KEYS
    if (Flag < KEY_RIGHT)               /* just for test button usage */
    #endif
    MilliSleep(300);                    /* smooth UI */

    /* wait for user feedback */
    Flag = TestKey(0, CHECK_KEY_TWICE | CHECK_BAT);

    if (Flag == KEY_SHORT)              /* short key press */
    {
      if (Contrast < Max) Contrast++;   /* increase value */
    }
    else if (Flag == KEY_TWICE)         /* two short key presses */
    {
      Flag = 0;                         /* end loop */
    }
    #ifdef HW_KEYS
    else if (Flag == KEY_RIGHT)         /* rotary encoder: right turn */
    {
      if (Contrast < Max) Contrast++;   /* increase value */
    }
    #endif
    else                                /* long key press / left turn */
    {
      if (Contrast > 0) Contrast--;       /* decrease */
    }

    LCD_Contrast(Contrast);        /* change contrast */
  }
}

#endif



#ifdef SW_FONT_TEST

/*
 *  display font for test purposes
 */

void FontTest(void)
{
  uint8_t           Run = 1;       /* loop control */
  uint8_t           n = 0;         /* char counter */
  uint8_t           i;             /* loop counter */
  uint8_t           Pos;           /* char X position */
  uint8_t           Max;           /* max char lines */

  /* show info */
  LCD_Clear();
  #ifdef UI_COLORED_TITLES
    /* display: Rotary Encoder */
    Display_ColoredEEString(FontTest_str, COLOR_TITLE);
  #else
    Display_EEString(FontTest_str);     /* display: Rotary Encoder */
  #endif
  UI.LineMode = LINE_STD | LINE_KEEP;   /* next-line mode: keep first line */

  Max = UI.CharMax_Y;         /* get maximum number of lines */
  Max--;                      /* - first line */

  while (Run)
  {
    Display_NextLine();            /* move to next line */

    /* display start value in hexadecimal */
    Display_HexByte(n);
    Display_Space();

    /* display next 8 chars */
    i = 0;
    while (i < 8)                  /* 8 chars */
    {
      Pos = UI.CharPos_X;          /* get currect X position */
      Display_Char(n);             /* display char */

      if (UI.CharPos_X == Pos)     /* no char available */
      {
        UI.CharPos_X++;            /* move right by one char */
      }

      i++;                         /* next char */
      n++;
    }

    /* loop management */
    Pos = 0;
    if (n == 0)               /* overflow to zero */
    {
      /* all 256 chars done */
      Run = 0;                /* end loop */
      Pos = 1;                /* request user feedback */
    }
    else                      /* not done yet */
    {
      if (Run == Max)         /* screen full */
      {
        Run = 1;              /* reset counter */
        Pos = 1;              /* request user feedback */
      }
      else                    /* some space left */
      {
        Run++;                /* another line */ 
      }
    }

    if (Pos)                  /* requested user feedback */
    {
      /* wait for user input */
      i = TestKey(0, CURSOR_BLINK | CHECK_KEY_TWICE | CHECK_BAT);

      if (i == KEY_TWICE)     /* two short key presses */
      {
        Run = 0;              /* end loop */
      }
    }
  }
}

#endif



/* ************************************************************************
 *   color code
 * ************************************************************************ */


#ifdef FUNC_COLORCODE

/*
 *  display color code
 *
 *  requires:
 *  - Value: unsigned value (2 or 3 digits)
 *  - Scale: exponent/multiplier (10^n)
 *  - TolBand: color of tolerance band
 */

void Display_ColorCode(uint16_t Value, int8_t Scale, uint16_t TolBand)
{
  uint8_t           Length;             /* string length */
  uint8_t           n;                  /* counter */
  uint8_t           Digit;              /* digit */
  uint16_t          *Table;             /* pointer to table */
  uint16_t          Color = 0;          /* display color */

  /* convert value into string */
  utoa(Value, OutBuffer, 10);           /* radix 10: max. 3 chars + /0 */
  Length = strlen(OutBuffer);           /* get string length */


  /*
   *  value
   */

  n = 0;                           /* reset counter */
  while (n < Length)               /* loop through string */
  {
    /* get digit */
    Digit = OutBuffer[n];          /* get char */
    Digit -= 48;                   /* convert to number */

    /* read color value from table */
    Table = (uint16_t *)&ColorCode_table[0];      /* pointer to table */
    Table += Digit;                               /* add index */
    Color = DATA_read_word(Table);                /* read color */

    /* display band (left-aligned) */
    LCD_Band(Color, ALIGN_LEFT);

    n++;                           /* next one */
  }


  /*
   *  multiplier
   */

  if (Scale >= 0)             /* use color code table */
  {
    /* read color value from table */
    Table = (uint16_t *)&ColorCode_table[0];      /* pointer to table */
    Table += Scale;                               /* add index */
    Color = DATA_read_word(Table);                /* read color */
  }
  else                        /* assign color manually */
  {
    if (Scale == -1)          /* -1 / 0.1 */
    {
      Color = COLOR_CODE_SILVER;
    }
    else if (Scale == -2)     /* -2 / 0.01 */
    {
      Color = COLOR_CODE_GOLD;
    }
  }

  /* display band (left-aligned) */
  LCD_Band(Color, ALIGN_LEFT);


  /*
   *  tolerance
   */

  /* display band (right-aligned) */
  LCD_Band(TolBand, ALIGN_RIGHT);
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

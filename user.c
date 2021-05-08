/* ************************************************************************
 *
 *   user interface functions
 *
 *   (c) 2012-2017 by Markus Reschke
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define USER_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */


/*
 *  local constants
 */

/* rotary encoder */
#define DIR_NONE         0b00000000     /* no turn or error */
#define DIR_RESET        0b00000001     /* reset state */
#define DIR_RIGHT        KEY_TURN_RIGHT /* turn to the right */
#define DIR_LEFT         KEY_TURN_LEFT  /* turn to the left */



/* ************************************************************************
 *   values and scales
 * ************************************************************************ */


/*
 *  get number of digits of a value
 */

uint8_t NumberOfDigits(uint32_t Value)
{
  uint8_t           Counter = 1;   /* return value */

  while (Value >= 10)
  {
    Value /= 10;
    Counter++;
  }

  return Counter;
}



/*
 *  compare two scaled values
 *
 *  requires:
 *  - first pair of value and scale
 *  - second pait of value and scale
 *
 *  returns:
 *  - -1 if first value is smaller than seconds one
 *  - 0 if equal
 *  - 1 if first value is larger than second one
 */

int8_t CmpValue(uint32_t Value1, int8_t Scale1, uint32_t Value2, int8_t Scale2)
{
  int8_t            Flag;               /* return value */
  int8_t            Len1, Len2;         /* length */

  /* determine virtual length */
  Len1 = NumberOfDigits(Value1) + Scale1;
  Len2 = NumberOfDigits(Value2) + Scale2;

  if ((Value1 == 0) || (Value2 == 0))    /* special case */
  {
    Flag = 10;                /* perform direct comparison */
  }
  else if (Len1 > Len2)       /* more digits -> larger */
  {
    Flag = 1;
  }
  else if (Len1 == Len2)      /* same length */
  {
    /* re-scale to longer value */
    Len1 -= Scale1;
    Len2 -= Scale2;

    while (Len1 > Len2)       /* up-scale Value #2 */
    {
      Value2 *= 10;
      Len2++;
      /* Scale2-- */
    }

    while (Len2 > Len1)       /* up-scale Value #1 */
    {
      Value1 *= 10;
      Len1++;
      /* Scale1-- */
    }   

    Flag = 10;                /* perform direct comparison */
  }
  else                        /* less digits -> smaller */
  {
    Flag = -1;
  }

  if (Flag == 10)             /* perform direct comparison */
  {
    if (Value1 > Value2) Flag = 1;
    else if (Value1 < Value2) Flag = -1;
    else Flag = 0;
  }

  return Flag;
}



/*
 *  rescale value
 *
 *  requires:
 *  - value and scale
 *  - new scale
 */

uint32_t RescaleValue(uint32_t Value, int8_t Scale, int8_t NewScale)
{
  uint32_t          NewValue;      /* return value */

  NewValue = Value;           /* take old value */

  while (Scale != NewScale)   /* processing loop */
  {
    if (NewScale > Scale)     /* upscale */
    {
      NewValue /= 10;
      Scale++;
    }
    else                      /* downscale */
    {
      NewValue *= 10;
      Scale--;
    }
  }

  return NewValue;
}



/* ************************************************************************
 *   display of values and units
 * ************************************************************************ */


#if defined (SW_SQUAREWAVE) || defined (SW_PWM_PLUS)

/*
 *  display unsigned value
 *
 *  requires:
 *  - unsigned value
 *  - decimal places
 *  - unit character (0 = none)
 */

void DisplayFullValue(uint32_t Value, uint8_t DecPlaces, unsigned char Unit)
{
  uint8_t           n;                  /* counter */
  uint8_t           Length;             /* string length */
  uint8_t           Pos = 0;            /* position of dot */

  /* convert value into string */
  ultoa(Value, OutBuffer, 10);
  Length = strlen(OutBuffer);

  /* determine position of dot */
  if (DecPlaces == 0)                   /* no dot needed */
  {
    Pos = 100;                          /* disable dot */
  }
  else if (Length >= DecPlaces)         /* position within string */
  {
    Pos = Length - DecPlaces;           /* position od dot */
    DecPlaces = 0;                      /* no additional zeros needed */
  }
  else                                  /* position outside string */
  {
    DecPlaces = DecPlaces - Length;     /* number of zeros after dot */
  }

  /* leading zero */
  if (Pos == 0) LCD_Char('0');          /* display: 0 */

  /* display digits */
  n = 0;
  while (n < Length)                    /* process string */
  {
    if (n == Pos)                       /* at position of dot */
    {
      LCD_Char('.');                    /* display: . */
      while (DecPlaces > 0)             /* fill in more zeros if needed */
      {
        LCD_Char('0');                  /* display: 0 */
        DecPlaces--;
      }
    }

    LCD_Char(OutBuffer[n]);             /* display digit */
    n++;                                /* next digit */
  }

  /* display unit */
  if (Unit) LCD_Char(Unit);
}

#endif



/*
 *  display unsigned value and unit
 *  - max. 4 digits excluding "." and unit
 *
 *  requires:
 *  - unsigned value
 *  - exponent of factor related to base unit (value * 10^x)
 *    e.g: p = 10^-12 -> -12
 *  - unit character (0 = none)
 */

void DisplayValue(uint32_t Value, int8_t Exponent, unsigned char Unit)
{
  unsigned char     Prefix = 0;         /* prefix character */
  uint8_t           Offset = 0;         /* exponent offset to next 10^3 step */
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

  if (Exponent >= -12)                  /* prevent index underflow */
  {
    Exponent += 12;                     /* shift exponent to be >= 0 */ 
    Index = Exponent / 3;               /* number of 10^3 steps */
    Offset = Exponent % 3;              /* offset to lower 10^3 step */

    if (Offset > 0)                     /* dot required */
    {
      Index++;                          /* upscale prefix */ 
      Offset = 3 - Offset;              /* reverse value (1 or 2) */
    }    

    /* look up prefix in table (also prevent array overflow) */
    if (Index <= 6) Prefix = eeprom_read_byte(&Prefix_table[Index]);
  }


  /*
   *  display value
   */

  /* convert value into string */
  utoa((uint16_t)Value, OutBuffer, 10);
  Length = strlen(OutBuffer);

  /* we misuse Exponent for the dot position */
  Exponent = Length - Offset;           /* calculate position */

  if (Exponent <= 0)                    /* we have to prepend "0." */
  {
    /* 0: factor 10 / -1: factor 100 */
    LCD_Char('0');
    LCD_Char('.');
    if (Exponent < 0) LCD_Char('0');    /* extra 0 for factor 100 */
  }

  if (Offset == 0) Exponent = -1;       /* disable dot if not needed */

  /* adjust position to match array or disable dot if set to 0 */ 
  Exponent--;

  /* display value and add dot if requested */
  Index = 0;
  while (Index < Length)                /* loop through string */
  {
    LCD_Char(OutBuffer[Index]);              /* display char */
    if (Index == Exponent) LCD_Char('.');    /* display dot */
    Index++;                                 /* next one */
  }

  /* display prefix and unit */
  if (Prefix) LCD_Char(Prefix);
  if (Unit) LCD_Char(Unit);
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

void DisplaySignedValue(int32_t Value, int8_t Exponent, unsigned char Unit)
{
  /* take care about sign */
  if (Value < 0)              /* negative value */
  {
    LCD_Char('-');            /* display: "-" */
    Value = -Value;           /* make value positive */
  }

  /* and display unsigned value */
  DisplayValue((int32_t)Value, Exponent, Unit);
}



/* ************************************************************************
 *   user input (testkey / rotary encoder)
 * ************************************************************************ */


#ifdef HW_ENCODER 

/*
 *  read rotary encoder
 *
 *  returns user action:
 *  - DIR_NONE for no turn or invalid signal
 *  - DIR_RIGHT for right/clockwise turn
 *  - DIR_LEFT for left/counter-clockwise turn
 */

uint8_t ReadEncoder(void)
{
  uint8_t           Action = DIR_NONE;       /* return value */
  uint8_t           Temp;                    /* temporary value */
  uint8_t           AB = 0;                  /* new AB state */
  uint8_t           Old_AB;                  /* old AB state */

  /* set encoder's A & B pins to input */
  Old_AB = ENCODER_DDR;                 /* save current settings */
  ENCODER_DDR &= ~(1 << ENCODER_A);     /* A signal pin */
  ENCODER_DDR &= ~(1 << ENCODER_B);     /* B signal pin */
  wait500us();                          /* settle time */

  /* get A & B signals */
  Temp = ENCODER_PIN;
  if (Temp & (1 << ENCODER_A)) AB = 0b00000010;
  if (Temp & (1 << ENCODER_B)) AB |= 0b00000001;

  /* restore port/pin settings */
  ENCODER_DDR = Old_AB;                 /* restore old settings */

  /* update state history */
  if (Enc.Dir == DIR_RESET)             /* first scan */
  {
    Enc.History = AB;                   /* set as last state */
    Enc.Dir = DIR_NONE;                 /* reset direction */
  }

  Old_AB = Enc.History;                 /* save last state */
  Enc.History = AB;                     /* and save new state */

  /* process signals */
  if (Old_AB != AB)        /* signals changed */
  {
    /* check if only one bit has changed (Gray code) */
    Temp = AB ^ Old_AB;                 /* get bit difference */
    if (!(Temp & 0b00000001)) Temp >>= 1;
    if (Temp == 1)                      /* valid change */
    {
      /* determine direction */
      /* Gray code: 00 01 11 10 */
      Temp = 0b10001101;                /* expected values for a right turn */
      Temp >>= (Old_AB * 2);            /* get expected value by shifting */
      Temp &= 0b00000011;               /* select value */
      if (Temp == AB)                   /* value matches */
        Temp = DIR_RIGHT;               /* turn to the right */
      else                              /* value mismatches */
        Temp = DIR_LEFT;                /* turn to the left */

      /* step/detent logic */
      Enc.Pulses++;                     /* got a new pulse */

      if (Temp != Enc.Dir)              /* direction has changed */
      {     
        Enc.Pulses = 1;                 /* first pulse for new direction */
      }

      Enc.Dir = Temp;         /* update direction */

      if (Enc.Pulses >= ENCODER_PULSES) /* reached step */
      {
        Enc.Pulses = 0;                 /* reset pulses */
        Action = Temp;                  /* signal valid step */
      }
    }
    else                                /* invalid change */
    {
      Enc.Dir = DIR_RESET;              /* reset direction state */
    }
  }

  return Action;
}

#endif



/*
 *  detect keypress of the test push button and any turn of an optional
 *  rotary encoder while tracking the turning velocity
 *
 *  requires:
 *  - Timeout in ms 
 *    0 = no timeout, wait for key press or rotary encoder
 *  - Mode:
 *    0 = no cursor
 *    1 = steady cursor
 *    2 = blinking cursor
 *    11 = steady cursor considering tester operation mode (UI.TesterMode)
 *    12 = blinking cursor considering tester operation mode (UI.TesterMode)
 *
 *  returns:
 *  - KEY_TIMEOUT    0 if timeout was reached
 *  - KEY_SHORT      1 if key was pressed short
 *  - KEY_LONG       2 if key was pressed long
 *  - KEY_TURN_RIGHT 3 if rotary encoder was turned right
 *  - KEY_TURN_LEFT  4 if rotary encoder was turned left
 *  The turning velocity is returned via Enc.Velocity.
 */

uint8_t TestKey(uint16_t Timeout, uint8_t Mode)
{
  uint8_t           Key = 0;       /* return value */
  uint8_t           Run = 1;       /* loop control */
  uint8_t           Counter = 0;   /* time counter */
  #ifdef HW_ENCODER
  uint8_t           Test;          /* temp. value */
  uint8_t           Counter2 = 0;  /* time counter #2 */
  uint8_t           Timeout2;      /* step timeout */
  uint8_t           Step = 0;      /* control flag */
  #endif


  /*
   *  sampling delay for rotary encoder
   *  - most got a bounce period of max. 3ms
   */

  #if ENCODER_PULSES < 4
    /* default delay */
    #define DELAY_TICK        5        /* 5ms */
    #define DELAY_500       100        /* ticks for 500ms */
  #else
    /* for 4 pulses/step we have to decrease the delay */
    #define DELAY_TICK        4        /* 4ms */
    #define DELAY_500       125        /* ticks for 500ms */
  #endif


  /*
   *  init
   */

  #ifdef HW_ENCODER
  /* init control for rotary encoder */
  Enc.History = 0;
  Enc.Dir = DIR_RESET;
  Enc.Pulses = 0;
  Enc.Velocity = 1;           /* default velocity (level #1) */

  Timeout2 = 6 + (2 * ENCODER_PULSES);
  #endif

  if (Mode > 10)              /* consider operation mode */
  {
    if (UI.TesterMode == MODE_AUTOHOLD)      /* auto hold mode */
    {
      Timeout = 0;                 /* disable timeout */
      Mode -= 10;                  /* set cursor mode */
    }
    else                                     /* continous mode */
    {
      Mode = 0;                    /* disable cursor */
    }
  }

  if (Mode > 0)               /* cursor enabled */
  {
    LCD_Cursor(1);            /* enable cursor */
  }


  /*
   *  wait for key press or timeout
   */
 
  while (Run)
  {
    /* take care about timeout */
    if (Timeout > 0)                    /* timeout enabled */
    {
      if (Timeout > 5) Timeout -= 5;    /* decrease timeout by 5ms */
      else Run = 0;                     /* end loop on timeout */
    }

    /* check for key press */
    /* push button is low active */
    if (!(CONTROL_PIN & (1 << TEST_BUTTON)))      /* if key is pressed */
    {
      Counter = 0;            /* reset counter */
      MilliSleep(30);         /* time to debounce */

      while (Run)             /* detect how long key is pressed */
      {
        if (!(CONTROL_PIN & (1 << TEST_BUTTON)))  /* key still pressed */
        {
          Counter++;                        /* increase counter */
          if (Counter > 26) Run = 0;        /* end loop if 300ms are reached */
          else MilliSleep(10);              /* otherwise wait 10ms */

        }
        else                                      /* key released */
        {
          Run = 0;                          /* end loop */
        }
      }

      /* determine key press type */
      if (Counter > 26) Key = KEY_LONG;   /* long (>= 300ms) */
      else Key = KEY_SHORT;               /* short (< 300ms) */
    }
    else                                          /* no key press */
    {
      #ifdef HW_ENCODER
      /* rotary encoder */
      if (Key) Counter2++;         /* increase counter if we had a step */

      Test = ReadEncoder();        /* read rotary encoder */
      if (Test)                    /* got user input */
      {
        if (Key == 0)              /* no step yet */
        {
          Key = Test;              /* save direction */
          Step = 1;                /* got girst step */
        }
        else if (Test == Key)      /* second step (same direction) */
        {
          /*
           *  determine turning velocity
           *  - use elapsed time ticks for second step
           *  - consider pulses/step to cope with various encoder types
           *  - create speed levels 2-7 (default: 1 for single step)
           *
           *  pulses/  timeout  timeout  max time/  seen time/  
           *   step    1. step  2. step  pulse      pulse
           *     1        8        8       8          ?
           *     2       10       14       7          1-5
           *     4       14       22       5          2-4(5)
           */

          Counter2 /= ENCODER_PULSES;        /* time ticks per pulse */
          if (Counter2 >= 6) Counter2 = 5;   /* limit value */
          Enc.Velocity = 7 - Counter2;       /* convert delay into speed */

          Counter2 = Timeout2;          /* end loop */
        }
      }
      else          /* no step detected */
      {
        if (Step == 1)             /* got first step */
        {
          // Enc.Dir == Key
          if (Enc.Pulses == 1)     /* first pulse of second step */
          {
            Step = 2;              /* set flag */
            /* increase timeout based on pulses/step */
            Timeout2 += (ENCODER_PULSES * 2);
          }
        }
      }

      if (Counter2 == Timeout2)    /* timeout for velocity detection */
      {      
        break;                     /* leave loop */
      }
      #endif

      MilliSleep(DELAY_TICK);      /* wait a little bit (4 or 5ms) */

      /* blinking cursor */
      /* HD44780's built-in blinking cursor is ugly anyway :) */
      
      if (Mode == 2)                    /* blinking cursor */
      {
        Counter++;                        /* increase counter */

        if (Counter == DELAY_500)         /* every 500ms (1Hz) */
        {
          Counter = 0;                    /* reset counter */

          /* we misuse Run as toggle switch */
          if (Run == 1)                   /* turn off */
          {
            LCD_Cursor(0);                /* disable cursor */
            Run = 2;                      /* toggle flag */
          }
          else                            /* turn on */
          {
            LCD_Cursor(1);                /* enable cursor */
            Run = 1;                      /* toggle flag */
          }
        }
      }
    }
  }


  /*
   *  clean up
   */

  if (Mode > 0)               /* cursor enabled */
  {
    LCD_Cursor(0);            /* disable cursor */
  }

  #undef DELAY_500
  #undef DELAY_TICK

  return Key;
}



/*
 *  convenience version of TestKey() to wait for any user input or timeout
 *  based on the tester's operation mode
 */

void WaitKey(void)
{
  /* wait for key press or 3000ms timeout */
  TestKey(3000, 11);
}



/* ************************************************************************
 *   extra UI stuff
 * ************************************************************************ */


/*
 *  Tell user to create or remove short-circuit of all three probes 
 *  and wait until he really does. But allow user to abort creating
 *  a short circuit by a key press.
 *
 *  requires:
 *  - mode:
 *    0 = remove short-circuit
 *    1 = create short circuit
 *
 *  returns:
 *  - 0 on any problem
 *  - 1 on success
 */

uint8_t ShortCircuit(uint8_t Mode)
{
  uint8_t           Flag = 2;           /* return value / control flag */
  uint8_t           Test;               /* test feedback */
  uint8_t           Comp;               /* expected result */
  unsigned char     *String;            /* display string pointer */

  /* init */
  if (Mode == 0)         /* remove short */
  {
    String = (unsigned char *)Remove_str;
    Comp = 0;
  }
  else                   /* create short */
  {
    String = (unsigned char *)Create_str;
    Comp = 3;
  } 

  /* check if already done */
  Test = AllProbesShorted();            /* get current status */
  if (Test == Comp) Flag = 1;           /* skip loop if job already done */

  /* if necessary tell user what to do */
  if (Flag == 2)
  {
    LCD_Clear();
    LCD_EEString(String);               /* display: Remove/Create */
    LCD_NextLine();
    LCD_EEString(ShortCircuit_str);     /* display: short circuit! */
  }  

  /* wait until all probes are dis/connected */
  while (Flag == 2)
  {
    Test = AllProbesShorted();     /* check for shorted probes */

    if (Test == Comp)         /* job done */
    {
       Flag = 1;              /* end loop */
       MilliSleep(200);       /* time to debounce */
    }
    else                      /* job not done yet */
    {
      Test = TestKey(100, 0);      /* wait 100ms or detect key press */
      if (Mode == 0) Test = 0;     /* ignore key for un-short mode */
      if (Test > KEY_TIMEOUT) Flag = 0;  /* abort on key press */
    }
  }

  return Flag;
}



#ifdef SW_CONTRAST

/*
 *  change LCD contrast
 *  - takes maximum value into account
 */

void ChangeContrast(void)
{
  uint8_t          Flag = 1;            /* loop control */
  uint8_t          Test = 1;            /* loop control */
  uint8_t          Contrast;            /* contrast value */
  uint8_t          Max;                 /* contrast maximum */
  

  /*
   *  increase: short key press / right turn 
   *  decrease: long key press / left turn
   *  done:     two brief key presses          
   */

  LCD_Clear();
  LCD_EEString_Space(Contrast_str);     /* display: Contrast */

  Contrast = NV.Contrast;          /* get current value */
  Max = UI.MaxContrast;

  while (Flag)
  {
    LCD_ClearLine2();
    DisplayValue(Contrast, 0, 0);

    #ifdef HW_ENCODER
    if (Flag < 3)                       /* just for test button usage */
    #endif
    MilliSleep(300);                    /* smooth UI */

    Flag = TestKey(0, 0);               /* wait for user feedback */
    if (Flag == KEY_SHORT)              /* short key press */
    {
      MilliSleep(50);                   /* debounce button a little bit longer */
      Test = TestKey(200, 0);           /* check for second key press */
      if (Test > KEY_TIMEOUT)           /* second key press */
      {
        Flag = 0;                         /* end loop */
      }
      else                              /* single key press */
      {
        if (Contrast < Max) Contrast++;   /* increase value */
      }
    }
    #ifdef HW_ENCODER
    else if (Flag == 3)                 /* rotary encoder: right turn */
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



/* ************************************************************************
 *   menues
 * ************************************************************************ */


/*
 *  menu tool
 *
 *  requires:
 *  - Items: number of menu items
 *  - Type: type of menu items
 *      1  pointer (in RAM) to fixed string stored in EEPROM
 *      2  uint16_t stored in EEPROM
 *  - Menu: address of array with menu items
 *  - Unit: optional fixed string stored in EEPROM
 *
 *  returns:
 *  - ID of selected item
 */

uint8_t MenuTool(uint8_t Items, uint8_t Type, void *Menu[], unsigned char *Unit)
{
  uint8_t           Selected = 0;       /* return value / ID of selected item */
  uint8_t           First = 0;          /* first item listed */
  uint8_t           Run = 2;            /* loop control flag */
  uint8_t           Lines;              /* line number */
  uint8_t           n;                  /* temp value */
  void              *Address;           /* address of menu element */
  uint16_t          Value;              /* temp. value */

  Items--;                    /* to match array counter */
  Lines = UI.CharMax_Y;       /* max. number of lines */
  Lines--;                    /* adjust to match item counter */
  LCD_Char(':');              /* whatever: */


  while (Run)
  {
    if (Lines == 1)           /* 2 line display */
    {
      First = Selected;       /* just one line for items */
      Run++;                  /* set flag for changed list */
    }


    /*
     *  display item(s)
     */

    Address = &Menu[First];        /* get address of first item */
    n = 0;

    while (n < Lines)
    {
      LCD_CharPos(1, n + 2);       /* move to start of line */

      /* display indicator for multiline displays */
      if (Lines > 1)
      {
        if (Selected == (First + n))    /* selected item */
        {
          LCD_Char('*');
        }
        else                            /* not selected */
        {
          LCD_Space();
        }
      }

      if (Run > 1)            /* list changed */
      {
        /* display item or value */
        if (Type == 1)                  /* fixed string */
        {
          LCD_EEString(*(unsigned char **)Address);
        }
        else                            /* uint16_t in EEPROM */
        {
          Value = eeprom_read_word(Address); /* read value at eeprom address */
          DisplayValue(Value, 0, 0);
        }     

        /* display optional fixed string */
        if (Unit)
        {
          LCD_EEString(Unit);
        }  

        LCD_ClearLine(0);     /* clear rest of this line */
      }

      Address += 2;                /* next address (2 byte steps) */
      n++;                         /* next item */

      if (n > Items) n = Lines;    /* end loop for a short list */
    }

    Run = 1;             /* reset loop flag (changed list) */

    /* show navigation help for 2 line displays */
    if (Lines == 1)
    {
      LCD_CharPos(UI.CharMax_X, UI.CharMax_Y);    /* set position to bottom right */
      if (Selected < Items) n = '>';      /* another item follows */
      else n = '<';                       /* last item */
      LCD_Char(n);
    }

    #ifndef HW_ENCODER
      MilliSleep(100);        /* smooth UI */
    #endif


    /*
     *  process user feedback
     */
 
    n = TestKey(0, 0);             /* wait for testkey */

    #ifdef HW_ENCODER
    /* processing for rotary encoder */
    if (n == KEY_SHORT)            /* short key press: select item */
    {
      n = KEY_LONG;                     /* trigger item selection */
    }
    else if (n == KEY_TURN_RIGHT)  /* rotary encoder: right turn */
    {
      n = KEY_SHORT;                    /* trigger next item */
    }
    else if (n == KEY_TURN_LEFT)   /* rotary encoder: left turn */
    {
      if (Selected == 0)                /* first item */
      {
        Selected = Items;               /* roll over to last item */

        if (Items >= Lines)             /* large list */
        {
          First = Items - Lines + 1;    /* update first item listed */
          Run++;                        /* set flag for changed list */
        }
      }
      else                              /* not first item */
      {
        Selected--;                     /* move to previous item */

        if (Selected == First)          /* item would be the first one listed */
        {
          if (Selected > 0)             /* more items in list */
          {
            First--;                    /* scroll one item down */
            Run++;                      /* set flag for changed list */
          }
        }
      }
    }
    #endif

    /* processing for testkey */
    if (n == KEY_SHORT)            /* short key press: move to next item */
    {
      if (Selected == Items)       /* last item */
      {
        Selected = 0;              /* roll over to first one */
        First = 0;                 /* also reset first item listed */

        if (Items >= Lines)        /* large list */
        {
          Run++;                   /* set flag for changed list */
        }
      }
      else                         /* more items follow */
      {
        Selected++;                /* move to next item */        

        n = First + Lines - 1;     /* last item on screen */
        if (Selected == n)         /* item would be the last one listed */
        {
          if (Items > Selected)    /* more items follow in list */
          {
            First++;               /* scroll one item up */
            Run++;                 /* set flag for changed list */
          }
        }
      }
    }
    else if (n == KEY_LONG)        /* long key press: select current item */
    {
      Run = 0;                     /* end loop */
    }
  }

  LCD_Clear();                /* feedback for user */
  MilliSleep(500);            /* smooth UI */

  return Selected;
}



/*
 *  adjustment menu
 */

void AdjustmentMenu(uint8_t Mode)
{
  #define MENU_ITEMS  3

  uint8_t           Item = 0;           /* item number */
  void              *MenuItem[MENU_ITEMS];   /* menu item strings */
  uint8_t           MenuID[MENU_ITEMS];      /* menu item IDs */
  uint8_t           ID;            /* profile ID */

  /* set up menu */
  MenuItem[Item] = (void *)Profile1_str;     /* profile #1 */
  MenuID[Item] = 1;
  Item++;  
  MenuItem[Item] = (void *)Profile2_str;     /* profile #2 */
  MenuID[Item] = 2;
  Item++;
  MenuItem[Item] = (void *)Exit_str;         /* exit menu */
  MenuID[Item] = 0;
  Item++;                                    /* add 1 for item #0 */

  /* display mode */
  LCD_Clear();
  if (Mode == MODE_SAVE)           /* write mode */
  {
    LCD_EEString(Save_str);
  }
  else                             /* read mode */
  {
    LCD_EEString(Load_str);
  }

  /* run menu */
  ID = MenuTool(Item, 1, MenuItem, NULL);    /* menu dialog */
  ID = MenuID[ID];                           /* get item ID */

  if (ID > 0)                 /* valid profile ID */
  {
    ManageAdjust(Mode, ID);
  }

  #undef MENU_ITEMS
}


/*
 *  create main menu and return ID of selected item  
 */

uint8_t PresentMainMenu(void)
{
  #define MENU_ITEMS  16

  uint8_t           Item = 0;           /* item number */
  uint8_t           ID;                 /* ID of selected item */
  void              *MenuItem[MENU_ITEMS];   /* menu item strings */
  uint8_t           MenuID[MENU_ITEMS];      /* menu item IDs */


  /*
   *  set up menu
   */

  /* extra items */
  #if defined (SW_PWM_SIMPLE) || defined (SW_PWM_PLUS)
  MenuItem[Item] = (void *)PWM_str;          /* PWM tool */
  MenuID[Item] = 6;
  Item++;
  #endif
  #ifdef SW_SQUAREWAVE
  MenuItem[Item] = (void *)SquareWave_str;   /* Square Wave Signal Generator */
  MenuID[Item] = 7;
  Item++;
  #endif
  #ifdef HW_ZENER
  MenuItem[Item] = (void *)Zener_str;        /* Zener tool */
  MenuID[Item] = 8;  
  Item++;
  #endif
  #ifdef SW_ESR
  MenuItem[Item] = (void *)ESR_str;          /* in-circuit ESR */
  MenuID[Item] = 9;
  Item++;
  #endif
  #ifdef HW_FREQ_COUNTER
  MenuItem[Item] = (void *)FreqCounter_str;  /* frequency counter */
  MenuID[Item] = 10;
  Item++;
  #endif
  #ifdef SW_ENCODER
  MenuItem[Item] = (void *)Encoder_str;      /* rotary encoder check */
  MenuID[Item] = 11;
  Item++;
  #endif
  #ifdef SW_CONTRAST
  MenuItem[Item] = (void *)Contrast_str;     /* change LCD contrast */
  MenuID[Item] = 12;
  Item++;
  #endif
  #ifdef SW_IR_RECEIVER
  MenuItem[Item] = (void *)IR_Detector_str;  /* IR RC detection */
  MenuID[Item] = 13;
  Item++;
  #endif
  #ifdef SW_OPTO_COUPLER
  MenuItem[Item] = (void *)OptoCoupler_str;  /* opto coupler tool */
  MenuID[Item] = 14;
  Item++;
  #endif
  #ifdef SW_SERVO
  MenuItem[Item] = (void *)Servo_str;        /* servo check */
  MenuID[Item] = 15;
  Item++;
  #endif

  /* standard items */
  MenuItem[Item] = (void *)Selftest_str;    /* selftest */
  MenuID[Item] = 1;
  Item++;
  MenuItem[Item] = (void *)Adjustment_str;  /* self-adjustment */
  MenuID[Item] = 2;
  Item++;  
  MenuItem[Item] = (void *)Save_str;        /* store self-adjustment values */
  MenuID[Item] = 3;
  Item++;
  MenuItem[Item] = (void *)Load_str;        /* load self-adjustment values */
  MenuID[Item] = 4;
  Item++;
  MenuItem[Item] = (void *)Show_str;        /* show self-adjustment values */
  MenuID[Item] = 5;
  Item++;
  MenuItem[Item] = (void *)Exit_str;        /* exit menu */
  MenuID[Item] = 0;


  /*
   *  run menu
   */

  LCD_Clear();
  LCD_EEString(Select_str);                  /* display "Select" */
  Item++;                                    /* add 1 for item #0 */
  ID = MenuTool(Item, 1, MenuItem, NULL);    /* menu dialog */
  ID = MenuID[ID];                           /* get item ID */

  return(ID);                 /* return item ID */

  #undef MENU_ITEMS
}



/*
 *  main menu
 */

void MainMenu(void)
{
  uint8_t           ID;                 /* ID of selected item */
  uint8_t           Flag = 1;           /* control flag */
  #ifdef SW_PWM_SIMPLE
  uint16_t          Frequency;          /* PWM frequency */  
  #endif

  ID = PresentMainMenu();     /* create menu and get user feedback */

  /* run selected item */
  switch (ID)
  {
    /* case 0:              exit menu */

    case 1:              /* self-test */
      Flag = SelfTest();
      break;

    case 2:              /* self-adjustment */
      Flag = SelfAdjust();
      break;

    case 3:              /* save self-adjustment values */
      AdjustmentMenu(MODE_SAVE);
      break;

    case 4:              /* load self-adjustment values */
      AdjustmentMenu(MODE_LOAD);
      break;

    case 5:              /* show self-adjustment values */
      ShowAdjust();
      break;

    #ifdef SW_PWM_SIMPLE
    case 6:              /* PWM tool with simple UI */
      /* run PWM menu */
      LCD_Clear();
      LCD_EEString(PWM_str);
      ID = MenuTool(8, 2, (void *)PWM_Freq_table, (unsigned char *)Hertz_str);
      Frequency = eeprom_read_word(&PWM_Freq_table[ID]);    /* get selected frequency */
      PWM_Tool(Frequency);                                  /* and run PWM tool */
      break;
    #endif

    #ifdef SW_PWM_PLUS
    case 6:              /* PWM tool with improved UI */
      PWM_Tool();
      break;
    #endif

    #ifdef SW_SQUAREWAVE
    case 7:              /* square wave signal generator */
      SquareWave_SignalGenerator();
      break;
    #endif

    #ifdef HW_ZENER
    case 8:              /* Zener tool */
      Zener_Tool();
      break;
    #endif

    #ifdef SW_ESR
    case 9:              /* ESR tool */
      ESR_Tool();
      break;
    #endif

    #ifdef HW_FREQ_COUNTER
    case 10:             /* frequency counter */
      FrequencyCounter();
      break;
    #endif

    #ifdef SW_ENCODER
    case 11:             /* rotary encoder check */
      Encoder_Tool();
      break;
    #endif

    #ifdef SW_CONTRAST
    case 12:             /* change contrast */
      ChangeContrast();
      break;
    #endif

    #ifdef SW_IR_RECEIVER
    case 13:             /* IR RC detection */
      IR_Detector();
      break;
    #endif

    #ifdef SW_OPTO_COUPLER
    case 14:             /* opto coupler tool */
      OptoCoupler_Tool();
      break;
    #endif

    #ifdef SW_SERVO
    case 15:             /* servo check */
      Servo_Check();
      break;
    #endif
  }

  /* display result */
  LCD_Clear();
  if (Flag == 0)
    LCD_EEString(Error_str);       /* display: error! */
  else
    LCD_EEString(Done_str);        /* display: done! */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef USER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

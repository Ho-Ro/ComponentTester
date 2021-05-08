/* ************************************************************************
 *
 *   user interface functions
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
#include "LCD.h"              /* LCD module */
#include "functions.h"        /* external functions */


/*
 *  local functions
 */

void PWM_Menu(void);



/* ************************************************************************
 *   values and scales
 * ************************************************************************ */


/*
 *  get number of digits of a value
 */

uint8_t NumberOfDigits(unsigned long Value)
{
  uint8_t           Counter = 1;

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
 *  returns:
 *  - -1 if first value is smaller than seconds one
 *  - 0 if equal
 *  - 1 if first value is larger than second one
 */

int8_t CmpValue(unsigned long Value1, int8_t Scale1, unsigned long Value2, int8_t Scale2)
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



/* ************************************************************************
 *   display of values and units
 * ************************************************************************ */


/*
 *  display value and unit
 *  - max. 4 digits excluding "." and unit
 *
 *  requires:
 *  - value
 *  - exponent of factor related to base unit (value * 10^x)
 *    e.g: p = 10^-12 -> -12
 *  - unit character (0 = none)
 */

void DisplayValue(unsigned long Value, int8_t Exponent, unsigned char Unit)
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
  utoa((unsigned int)Value, OutBuffer, 10);
  Length = strlen(OutBuffer);

  /* we misuse Exponent for the dot position */
  Exponent = Length - Offset;           /* calculate position */

  if (Exponent <= 0)                    /* we have to prepend "0." */
  {
    /* 0: factor 10 / -1: factor 100 */
    lcd_data('0');
    lcd_data('.');
    if (Exponent < 0) lcd_data('0');    /* extra 0 for factor 100 */
  }

  if (Offset == 0) Exponent = -1;       /* disable dot if not needed */

  /* adjust position to match array or disable dot if set to 0 */ 
  Exponent--;

  /* display value and add dot if requested */
  Index = 0;
  while (Index < Length)                /* loop through string */
  {
    lcd_data(OutBuffer[Index]);              /* display char */
    if (Index == Exponent) lcd_data('.');    /* display dot */
    Index++;                                 /* next one */
  }

  /* display prefix and unit */
  if (Prefix) lcd_data(Prefix);
  if (Unit) lcd_data(Unit);
}



/*
 *  display signed value and unit
 *  - max. 4 digits excluding sign, "." and unit
 *
 *  requires:
 *  - value
 *  - exponent of factor related to base unit (value * 10^x)
 *    e.g: p = 10^-12 -> -12
 *  - unit character (0 = none)
 */

void DisplaySignedValue(signed long Value, int8_t Exponent, unsigned char Unit)
{
  /* take care about sign */
  if (Value < 0)              /* negative value */
  {
    lcd_data('-');            /* display: "-" */
    Value = -Value;           /* make value positive */
  }

  /* and display unsigned value */
  DisplayValue((signed long)Value, Exponent, Unit);
}



/* ************************************************************************
 *   user interface for push buttons
 * ************************************************************************ */


/*
 *  Tell user to create or remove short-circuit of all three probes 
 *  and wait until he realy does.
 *
 *  requires:
 *  - mode:
 *    0 = remove short-circuit
 *    1 = create short circuit
 */

void ShortCircuit(uint8_t Mode)
{
  uint8_t           Run = 0;            /* loop control */
  uint8_t           Test;               /* test feedback */
  unsigned char     *String = NULL;     /* display string pointer */

  Test = AllProbesShorted();            /* get current status */

  if (Mode == 0)         /* remove short */
  {
    if (Test != 0) String = (unsigned char *)Remove_str;    /* some shorted */
  }
  else                   /* create short */
  {
    if (Test != 3) String = (unsigned char *)Create_str;    /* some unshorted */
  }

  /* if required tell user what to do */ 
  if (String)
  {
    lcd_clear();
    lcd_fix_string(String);             /* display: Remove/Create */
    lcd_line(2);
    lcd_fix_string(ShortCircuit_str);   /* display: short circuit! */
    Run = 1;                            /* enter loop */
  }
  
  /* wait until all probes are dis/connected */
  while (Run == 1)
  {
    Test = AllProbesShorted();     /* check for short circuits */

    if (Mode == 0)            /* remove short */
    {
      if (Test == 0) Run = 0;      /* end loop if all removed */
    }
    else                      /* create short */
    {
      if (Test == 3) Run = 0;      /* end loop if all shorted */
    }

    if (Run == 1)             /* if not done yet */
      MilliSleep(50);              /* wait a little bit */
    else                      /* if done */
      MilliSleep(200);             /* time to debounce */
  }
}



/*
 *  detect keypress of test push button
 *
 *  requires:
 *  - Timeout in ms 
 *    0 = no timeout, wait for key press
 *  - Mode:
 *    0 = no cursor
 *    1 = consider tester operation mode (Config.TesterMode)
 *    2 = steady cursor
 *    3 = blinking cursor (not implemented yet)
 *
 *  returns:
 *  - 0 if timeout was reached
 *  - 1 if key was pressed short
 *  - 2 if key was pressed long
 */

uint8_t TestKey(uint16_t Timeout, uint8_t Mode)
{
  uint8_t           Flag = 0;      /* return value */
  uint8_t           Run = 1;       /* loop control */
  uint8_t           Counter = 0;   /* time counter */


  /*
   *  init
   */

  if (Mode == 1)              /* consider operation mode */
  {
    if (Config.TesterMode == MODE_AUTOHOLD)  /* auto hold mode */
    {
      Timeout = 0;                           /* disable timeout */
      Mode = 2;                              /* enable steady cusrsor */
    }
  }

  if (Mode > 1)               /* cursor enabled */
  {
    /* set position: char 16 in line 2 */
    lcd_command(CMD_SET_DD_RAM_ADDR | 0x4F);

    /* enable cursor */
    lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON | FLAG_CURSOR_ON);
  }


  /*
   *  wait for key press or timeout
   */
 
  while (Run == 1)
  {
    /* take care about timeout */
    if (Timeout > 0)                    /* timeout enabled */
    {
      if (Timeout > 5) Timeout -= 5;    /* decrease timeout by 5ms */
      else Run = 0;                     /* end loop on timeout */
    }

    /* check for key press */
    /* test push button is low active */
    if (!(CONTROL_PIN & (1 << TEST_BUTTON)))      /* if key is pressed */
    {
      MilliSleep(30);         /* time to debounce */

      while (Run)             /* detect how long key is pressed */
      {
        if (!(CONTROL_PIN & (1 << TEST_BUTTON)))  /* key still pressed */
        {
          Counter++;                        /* increase counter */
          if (Counter > 26) Run = 0;        /* end loop if 300ms are reached */
          else MilliSleep(10);              /* otherweise wait 10ms */
        }
        else                                      /* key released */
        {
          Run = 0;                          /* end loop */
        }
      }

      /* determine key press type */
      if (Counter > 26) Flag = 2;       /* long (>= 300ms) */
      else Flag = 1;                    /* short (< 300ms) */
    }
    else                                          /* no key press */
    {
      MilliSleep(5);                      /* wait a little bit more (5ms) */
    }
  }


  /*
   *  clean up
   */

  if (Mode > 1)               /* cursor enabled */
  {
    /* disable cursor */
    lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON);
  }

  return Flag;
}



/*
 *  main menu
 *  - entered by short-circuiting all three probes
 */

void MainMenu(void)
{
  uint8_t           Flag = 1;           /* control flag */
  uint8_t           Run = 0;            /* loop control flag */
  uint8_t           Selected = 1;       /* ID of selected item */
  uint8_t           Top = 1;            /* ID of top item */
  uint8_t           n;                  /* counter */
  unsigned char     *String = NULL;     /* menu string */
  unsigned char     *String2 = NULL;    /* item string */

#define MAX_ITEMS   5         /* number of menu items */

  /*
   *  menu item selection
   */

  while (Run == 0)
  {
    lcd_clear();

    /* display two items */
    for (n = Top; n < (Top + 2); n++)
    {
      /* display marker for selected item, a space otherwise */
      if (n == Selected) lcd_data('*');
      else lcd_space();

      lcd_space();                      /* display space */

      /* display item */
      switch (n)
      {
        case 1:
          String = (unsigned char *)PWM_str;
          break;

        case 2:
          String = (unsigned char *)Selftest_str;
          break;

        case 3:
          String = (unsigned char *)Adjustment_str;
          break;

        case 4:
          String = (unsigned char *)Save_str;
          break;

        case 5:
          String = (unsigned char *)Show_str;
          break;
      }

      lcd_fix_string(String);
      lcd_line(2);

      if (n == Selected) String2 = String;   /* save string of selected item */
    }

    /* process user feedback */
    n = TestKey(0, 0);             /* wait for testkey */
    if (n == 1)                    /* short key press selects next item */
    {
      Selected++;                       /* move to next item */
      if (Selected > MAX_ITEMS)         /* max. number of items exceeded */
      {
        Selected = 1;                   /* roll over to first one */
        Top = 1;
      }
      else if (Selected < MAX_ITEMS)    /* some items are left */
      {
        Top = Selected;                 /* make selected item the top one */
      }
    }
    else if (n == 2)               /* long key press runs selected item */
    {
      Run = Selected;                   /* end loop */
    }
  }


  /*
   *  run selected item
   */

  lcd_clear();                 /* feedback for user */
  MilliSleep(500);             /* prevent test key trouble */

  switch (Run)
  {
    case 1:
      PWM_Menu();
      break;

    case 2:
      Flag = SelfTest();
      break;

    case 3:
      Flag = SelfAdjust();
      break;

    case 4:
      SafeAdjust();
      break;

    case 5:
      ShowAdjust();
      break;
  }

  /* display end of item */
  lcd_clear();
  lcd_fix_string(String2);              /* display: <item> */
  lcd_line(2);
  if (Flag == 1)
    lcd_fix_string(Done_str);           /* display: done! */
  else
    lcd_fix_string(Error_str);          /* display: error! */

#undef MAX_ITEMS
}



/* ************************************************************************
 *   extras
 * ************************************************************************ */


/*
 *  PWM tool
 *  - use probe #2 (PB2, OC1B) as PWM output
 *    and probe #1 + probe #3 as ground
 *
 *  requires:
 *  - Freqency in Hz
 */

void PWM_Tool(uint16_t Frequency)
{
  uint8_t           Test = 1;           /* loop control and user feedback */
  uint8_t           Ratio;              /* PWM ratio */
  uint8_t           Prescaler;          /* timer prescaler */
  uint16_t          Top;                /* top value */
  uint16_t          Toggle;             /* counter value to toggle output */
  uint32_t          Value;              /* temporary value */

  /*
      fast PWM:             f = f_MCU / (prescaler * depth)
      phase correct PWM:    f = f_MCU / (2 * prescaler * depth)
      available prescalers: 1, 8, 64, 256, 1024
      depth:                2^x (x is the bit depth)
  */

  ShortCircuit(0);                    /* make sure probes are not shorted */
  lcd_clear();
  lcd_fix_string(PWM_str);            /* display: PWM */
  lcd_data(' ');
  DisplayValue(Frequency, 0, 'H');    /* display frequency */
  lcd_data('z');                      /* make it Hz :-) */

  R_PORT = 0;                         /* make probe #1 and #3 ground */
  /* set all probes to output mode */
  R_DDR = (1 << (TP1 * 2)) | (1 << (TP2 * 2)) | (1 << (TP3 * 2));


  /*
   *  calculate required prescaler and top value based on MCU clock
   *
   *    depth = f_MCU / (2 * prescaler * f_PWM)
   */

  Value = CPU_FREQ / 2;
  Value /= Frequency;

  if (Value > 2000000)        /* low frequency */
  {
    Value /= 256;
    Prescaler = (1 << CS12);                 /* 256 */
  }
  else if (Value > 16000)     /* mid-range frequency */
  {
    Value /= 64;
    Prescaler = (1 << CS11) | (1 << CS10);   /* 64 */    
  }
  else                        /* high frequency */
  {
    Prescaler = (1 << CS10);                 /* 1 */    
  }

  Top = (uint16_t)Value;


  /*
   *  setup timer1 for PWM
   *  - PWM, phase correct, top value by OCR1A
   */

  Ratio = 50;                                /* default ratio is 50% */
  Toggle = (Top / 2) - 1;                    /* compare value for 50% */
  /* power save mode would disable timer1 */
  Config.SleepMode = SLEEP_MODE_IDLE;        /* change sleep mode to Idle */

  TCCR1B = 0;                                /* disable timer */
  /* enable OC1B pin and set timer mode */
  TCCR1A = (1 << WGM11) | (1 << WGM10) | (1 << COM1B1);
  TCCR1B = (1 << WGM13);
  TCNT1 = 0;                                 /* set counter to 0 */
  OCR1A = Top - 1;                           /* set top value (-1) */
  OCR1B = Toggle;                            /* set value to compare with */

  /* enable counter by setting clock prescaler */
  TCCR1B = (1 << WGM13) | Prescaler;


  /*
   *  ratio control
   */

  while (Test > 0)
  {
    /* show current ratio */
    lcd_clear_line(2);
    DisplayValue(Ratio, 0, '%');        /* show ratio in % */
    MilliSleep(500);                    /* prevent test key trouble */

    /*
        short key press -> increase ratio
        long key press -> decrease ratio
        if lower or upper limit is exceeded exit loop
     */

    Test = TestKey(0, 0);               /* wait for user feedback */
    if (Test == 1)                      /* short key press */
    {
      if (Ratio == 100) Test = 0;       /* end if maximum is exceeded */
      else Ratio += 5;
    }
    else                                /* long key press */
    {
      if (Ratio == 0) Test = 0;         /* end if minimum is exeeded */
      else Ratio -= 5;
    }

    /* calculate toggle value: (depth * (ratio / 100)) - 1 */
    Value = (uint32_t)Top * Ratio;
    Value /= 100;
    Toggle = (uint16_t)Value;
    Toggle--;

    OCR1B = Toggle;                     /* update compare value */
  }

  /* clean up */
  TCCR1B = 0;                 /* disable timer */
  TCCR1A = 0;                 /* reset flags (also frees PB2) */
  R_DDR = 0;                  /* set HiZ mode */
  Config.SleepMode = SLEEP_MODE_PWR_SAVE;    /* reset sleep mode to default */
}



/*
 *  submenu to select PWM frequency
 */

void PWM_Menu(void)
{
  uint8_t           Run = 0;            /* loop control flag */
  uint8_t           Selected = 1;       /* ID of selected item */
  uint8_t           Top = 1;            /* ID of top item */
  uint8_t           n;                  /* counter */
  uint16_t          Frequency = 0;      /* menu frequency */

#define MAX_ITEMS   8         /* number of menu items */

  /*
   *  menu item selection
   */

  while (Run == 0)
  {
    lcd_clear();

    /* display two items */
    for (n = Top; n < (Top + 2); n++)
    {
      /* display marker for selected item, a space otherwise */
      if (n == Selected) lcd_data('*');
      else lcd_space();

      lcd_space();                      /* display space */

      /* display item */
      /* get frequency based on the item number from a predefined table */
      Frequency = MEM_read_word(&PWM_Freq_table[n - 1]);
      DisplayValue(Frequency, 0, 'H');
      lcd_data('z');
      lcd_line(2);
    }

    /* process user feedback */
    n = TestKey(0, 0);             /* wait for testkey */
    if (n == 1)                    /* short key press selects next item */
    {
      Selected++;                       /* move to next item */
      if (Selected > MAX_ITEMS)         /* max. number of items exceeded */
      {
        Selected = 1;                   /* roll over to first one */
        Top = 1;
      }
      else if (Selected < MAX_ITEMS)    /* some items are left */
      {
        Top = Selected;                 /* make selected item the top one */
      }
    }
    else if (n == 2)               /* long key press runs selected item */
    {
      Run = Selected;                   /* end loop */
    }
  }


  /*
   *  run selected item
   */

  lcd_clear();                 /* feedback for user */
  MilliSleep(500);             /* prevent test key trouble */

  /* get selected frequency */
  Frequency = MEM_read_word(&PWM_Freq_table[Run - 1]);
  PWM_Tool(Frequency);                  /* run PWM tool */

#undef MAX_ITEMS

}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef USER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   user interface functions
 *
 *   (c) 2012-2013 by Markus Reschke
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
 *  requires:
 *  - first pair of value and scale
 *  - second pait of value and scale
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



#ifdef EXTRA

/*
 *  rescale value
 *
 *  requires:
 *  - value and scale
 *  - new scale
 */

unsigned long RescaleValue(unsigned long Value, int8_t Scale, int8_t NewScale)
{
  unsigned long     NewValue;

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

#endif



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
    lcd_fixed_string(String);             /* display: Remove/Create */
    lcd_line(2);
    lcd_fixed_string(ShortCircuit_str);   /* display: short circuit! */
    Run = 1;                              /* enter loop */
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
 *    1 = steady cursor
 *    2 = blinking cursor
 *    11 = steady cursor considering tester operation mode (Config.TesterMode)
 *    12 = blinking cursor considering tester operation mode (Config.TesterMode)
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

  if (Mode > 10)                   /* consider operation mode */
  {
    if (Config.TesterMode == MODE_AUTOHOLD)  /* auto hold mode */
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
    /* set position: char 16 in line 2 */
    lcd_command(CMD_SET_DD_RAM_ADDR | 0x4F);

    /* enable cursor */
    lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON | FLAG_CURSOR_ON);
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
      MilliSleep(5);                    /* wait a little bit more (5ms) */

      /* simulate blinking cursor
         The LCDs built in cursor blinking is ugly and slow */
      
      if (Mode == 2)                    /* blinking cursor */
      {
        Counter++;                        /* increase counter */

        if (Counter == 100)               /* every 500ms (2Hz) */
        {
          Counter = 0;                    /* reset counter */

          /* we misuse Run as toggle switch */
          if (Run == 1)                   /* turn off */
          {
            /* disable cursor */
            lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON | FLAG_CURSOR_OFF);
            Run = 2;                      /* toggle flag */
          }
          else                            /* turn on */
          {
            /* enable cursor */
            lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON | FLAG_CURSOR_ON);
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
    /* disable cursor */
    lcd_command(CMD_DISPLAY_CONTROL | FLAG_DISPLAY_ON);
  }

  return Flag;
}



/*
 *  menu tool
 *
 *  requires:
 *  - Items: number of menu items
 *  - Type: type of menu items
 *      1  pointer (in RAM) to fixed string stored in EEPROM
 *      2  uint16_t stored in Flash or EEPROM
 *  - Menu: address of array with menu items
 *  - Unit: optional fixed string stored in EEPROM
 *
 *  returns:
 *  - number of selected item
 */

uint8_t MenuTool(uint8_t Items, uint8_t Type, void *Menu[], unsigned char *Unit)
{
  uint8_t           Selected = 0;       /* return value / ID of selected item */
  uint8_t           Run = 1;            /* loop control flag */
  uint8_t           n;                  /* temp value */
  void              *Address;           /* address of menu element */
  uint16_t          Value;              /* temp. value */

  Items--;                    /* to match array counter */
  lcd_data(':');              /* whatever: */

  while (Run)
  {
    /*
     *  display item
     */

    lcd_clear_line(2);
    Address = &Menu[Selected];     /* get address of element */

    if (Type == 1)                 /* fixed string */
    {
      lcd_fixed_string(*(unsigned char **)Address);
    }
    else                           /* uint16_t in Flash or EEPROM */
    {
      Value = MEM_read_word(Address);   /* read value at flash/eeprom address */
      DisplayValue(Value, 0, 0);
    }

    if (Unit)                      /* optional fixed string */
    {
      lcd_fixed_string(Unit);
    }


    /*
     *  show navigation help
     */

    MilliSleep(100);               /* smooth UI */

    /* set position: char 16 in line 2 */
    lcd_command(CMD_SET_DD_RAM_ADDR | 0x4F);

    if (Selected < Items) n = '>';      /* another item follows */
    else n = '<';                       /* last item */

    lcd_data(n);


    /*
     *  process user feedback
     */

    n = TestKey(0, 0);             /* wait for testkey */
    if (n == 1)                    /* short key press: moves to next item */
    {
      Selected++;                       /* move to next item */
      if (Selected > Items)             /* max. number of items exceeded */
      {
        Selected = 0;                   /* roll over to first one */
      }
    }
    else if (n == 2)               /* long key press: select current item */
    {
      Run = 0;                          /* end loop */
    }
  }

  lcd_clear();                 /* feedback for user */
  MilliSleep(500);             /* smooth UI */

  return Selected;
}



/*
 *  main menu
 */

void MainMenu(void)
{
  #ifdef EXTRA
    #define MENU_ITEMS  7
  #else
    #define MENU_ITEMS  6  
  #endif

  uint8_t           Item = 0;           /* item number */
  uint8_t           Flag = 1;           /* control flag */
  uint8_t           ID;                 /* ID of selected item */
  uint16_t          Frequency;          /* PWM frequency */  
  void              *MenuItem[MENU_ITEMS];   /* menu item strings */
  uint8_t           MenuID[MENU_ITEMS];      /* menu item IDs */

  /* setup menu */
  MenuItem[Item] = (void *)PWM_str;
  MenuID[Item] = 5;
  #ifdef EXTRA
    #ifdef HW_ZENER
  Item++;
  MenuItem[Item] = (void *)Zener_str;
  MenuID[Item] = 6;
    #endif
  #endif
  Item++;
  MenuItem[Item] = (void *)Selftest_str;
  MenuID[Item] = 1;
  Item++;
  MenuItem[Item] = (void *)Show_str;
  MenuID[Item] = 4;
  Item++;
  MenuItem[Item] = (void *)Adjustment_str;
  MenuID[Item] = 2;
  Item++;  
  MenuItem[Item] = (void *)Save_str;
  MenuID[Item] = 3;
  Item++;
  MenuItem[Item] = (void *)Exit_str;
  MenuID[Item] = 0;

  /* run menu */
  lcd_clear();
  lcd_fixed_string(Select_str);              /* display "Select" */
  Item++;                                    /* add 1 for item #0 */
  ID = MenuTool(Item, 1, MenuItem, NULL);
  ID = MenuID[ID];

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

    case 3:              /* safe self-adjustment values */
      SafeAdjust();
      break;

    case 4:              /* show self-adjustment values */
      ShowAdjust();
      break;

    case 5:              /* PWM tool */
      /* run PWM menu */
      lcd_clear();
      lcd_fixed_string(PWM_str);
      ID = MenuTool(8, 2, (void *)PWM_Freq_table, (unsigned char *)Hertz_str);
      Frequency = MEM_read_word(&PWM_Freq_table[ID]);  /* get selected frequency */
      PWM_Tool(Frequency);                             /* and run PWM tool */
      break;

    #ifdef EXTRA
      #ifdef HW_ZENER
    case 6:              /* Zener tool */
      Zener_Tool();
      break;
      #endif
    #endif
  }

  /* display end of item */
  lcd_clear();
  if (Flag == 0)
    lcd_fixed_string(Error_str);          /* display: error! */
  else
    lcd_fixed_string(Done_str);           /* display: done! */

#undef MENU_ITEMS
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef USER_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

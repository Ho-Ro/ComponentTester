/* ************************************************************************
 *
 *   main part
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define MAIN_C


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
 *  local variables
 */

/* programm control */
uint8_t        TesterMode;              /* tester operation mode */
uint8_t        RunsPassed;              /* counter for successful measurements */
uint8_t        RunsMissed;              /* counter for failed/missed measurements */

/* misc */
uint8_t        TempByte1, TempByte2;    /* temporary values */
unsigned int   TempWord;                /* temporary value */
signed int     TempInt;                 /* temporary value */



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
    if (Index <= 6) Prefix = pgm_read_byte(&Prefix_table[Index]);
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

  /* and call display unsigned value */
  DisplayValue((signed long)Value, Exponent, Unit);
}



/* ************************************************************************
 *   user interface
 * ************************************************************************ */


/*
 *  detect keypress of test push button
 *
 *  requires:
 *  - timeout in ms (0 = disable timeout)
 *  - mode:
 *    0 = wait until key is pressed or timeout is reached
 *    1 = wait until key is pressed (unlimted timeout)
 *
 *  returns:
 *  - 0 if timeout was reached
 *  - 1 if key was pressed short
 *  - 2 if key was pressed long
 */

uint8_t TestKey(unsigned int Timeout, uint8_t Mode)
{
  uint8_t           Flag = 0;      /* return value */

  /* we re-use Mode as loop control mask */
  /* bit0: key flag / bit7: timeout flag */
  if (Mode == 0) Mode = 0b10000001;
  else Mode = 0b00000001;

  if (Timeout == 0) Timeout++;     /* prevent negative overrun */

  /* wait for key press or timeout */ 
  while (Mode > 0)
  {
    /* short and long key press */
    if (!(CONTROL_PIN & (1 << TEST_BUTTON)))      /* if test button is pressed */
    {
      wait300ms();                                /* wait to catch a long key press */

      if (!(CONTROL_PIN & (1 << TEST_BUTTON)))    /* if button is still pressed */
      {
        Flag = 2;                                   /* set "long" */
      }
      else                                        /* no, just a short button press */
      {
        Flag = 1;                                   /* set "short" */
      }      

      Mode = 0;                         /* reset all flags */
    }

    wdt_reset();                        /* reset watchdog */
    wait1ms();                          /* wait a little bit more */

    /* timeout */
    if (Mode & 0b10000000)              /* timeout enabled */
    {
      Timeout--;                        /* decrease timeout */
      if (Timeout == 0) Mode = 0;       /* reset all flags on timeout */
    }
  }

  return Flag;
}



/* ************************************************************************
 *   internal setup
 * ************************************************************************ */


/*
 *  Tell user to remove a short-circuit and wait until 
 *  the short-circuit is really removed.
 */

void RemoveShortCircuit(void)
{
  /* tell user to remove short circuit */
  lcd_fix_string(Remove_str);          /* display: Remove */
  lcd_line(2);
  lcd_fix_string(ShortCircuit_str);    /* display: short circuit! */
  
  /* wait until probes are disconnected */
  TempByte1 = 1;
  while (TempByte1 == 1)
  {
    /* check for short circuits */
    TempByte2 = AllProbesShorted();

    if (TempByte2 == 0)            /* if all removed */
    {
      TempByte1 = 0;                    /* end loop */
    }
    else                           /* otherwise wait */
    {
      wdt_reset();                      /* reset watchdog */
      wait1ms();                        /* wait a little bit */
    }
  }
}



/*
 *  selftest
 *  - display several internal values and measurements
 *  - self-calibration of RiL, RiH and C-Zero
 *
 *  returns:
 *  - 0 on error
 *  - 1 on success
 */

uint8_t SelfTest(void)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           Test = 1;           /* test counter */
  uint8_t           Counter;            /* loop counter */
  uint8_t           DisplayFlag;        /* display flag */
  signed int        Val1 = 0, Val2 = 0, Val3 = 0;   /* voltages/values */

  /* check for short-circuited probes */
  if (AllProbesShorted() != 3) return Flag;

  /* loop through all tests */
  while (Test <= 6)
  {
    Counter = 1;

    /* repeat each test 5 times */
    while (Counter <= 5)
    {
      /* display test number */
      lcd_clear();
      lcd_data('t');                    /* display: t */
      lcd_data('0' + Test);             /* display test number */
      lcd_space();

      DisplayFlag = 1;                  /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* reference voltage */
          TempWord = ReadU(0x0e);            /* dummy read for bandgap stabilization */
          TempWord = ReadU(0x0e);            /* read bandgap reference voltage */ 
          lcd_fix_string(URef_str);          /* display: Vref */

          lcd_line(2);
          DisplayValue(TempWord, -3, 'V');   /* display voltage in mV */

          DisplayFlag = 0;                   /* reset flag */
          break;

        case 2:     /* compare Rl resistors (probes still connected) */
          lcd_fix_string(Rl_str);            /* display: +Rl- */
          lcd_space();
          lcd_fix_string(ProbeComb_str);     /* display: 12 13 23 */

          /* set up a voltage divider with the Rl's */
          /* substract theoretical voltage of voltage divider */

          /* TP1: Gnd -- Rl -- probe-2 -- probe-1 -- Rl -- Vcc */
          R_PORT = 1 << (TP1 * 2);
          R_DDR = (1 << (TP1 * 2)) | (1 << (TP2 * 2));
          Val1 = ReadU_20ms(TP3);
          Val1 -= ((long)UREF_VCC * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          /* TP1: Gnd -- Rl -- probe-3 -- probe-1 -- Rl -- Vcc */
          R_DDR = (1 << (TP1 * 2)) | (1 << (TP3 * 2));
          Val2 = ReadU_20ms(TP2);
          Val2 -= ((long)UREF_VCC * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          /* TP1: Gnd -- Rl -- probe-3 -- probe-2 -- Rl -- Vcc */
          R_PORT = 1 << (TP2 * 2);
          R_DDR = (1 << (TP2 * 2)) | (1 << (TP3 * 2));
          Val3 = ReadU_20ms(TP2);
          Val3 -= ((long)UREF_VCC * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          break;

        case 3:     /* compare Rh resistors (probes still connected) */
          lcd_fix_string(Rh_str);            /* display: +Rh- */
          lcd_space();
          lcd_fix_string(ProbeComb_str);     /* display: 12 13 23 */

          /* set up a voltage divider with the Rh's */

          /* TP1: Gnd -- Rh -- probe-2 -- probe-1 -- Rh -- Vcc */
          R_PORT = 2 << (TP1 * 2);
          R_DDR = (2 << (TP1 * 2)) | (2 << (TP2 * 2));
          Val1 = ReadU_20ms(TP3);
          Val1 -= (UREF_VCC / 2);

          /* TP1: Gnd -- Rh -- probe-3 -- probe-1 -- Rh -- Vcc */
          R_DDR = (2 << (TP1 * 2)) | (2 << (TP3 * 2));
          Val2 = ReadU_20ms(TP2);
          Val2 -= (UREF_VCC / 2);

          /* TP1: Gnd -- Rh -- probe-3 -- probe-2 -- Rh -- Vcc */
          R_PORT = 2 << (TP2 * 2);
          R_DDR = (2 << (TP2 * 2)) | (2 << (TP3 * 2));
          Val3 = ReadU_20ms(TP1);
          Val3 -= (UREF_VCC / 2);

          break;

        case 4:     /* disconnect probes */
          RemoveShortCircuit();
          Counter = 100;                        /* skip test */
          DisplayFlag = 0;                      /* reset flag */
          break;

        case 5:     /* Rh resistors pulled down */
          lcd_fix_string(RhLow_str);         /* display: Rh- */

          /* TP1: Gnd -- Rh -- probe */
          R_PORT = 0;
          R_DDR = 2 << (TP1 * 2);
          Val1 = ReadU_20ms(TP1);

          /* TP1: Gnd -- Rh -- probe */
          R_DDR = 2 << (TP2 * 2);
          Val2 = ReadU_20ms(TP2);

          /* TP1: Gnd -- Rh -- probe */
          R_DDR = 2 << (TP3 * 2);
          Val3 = ReadU_20ms(TP3);

          break;

        case 6:     /* Rh resistors pulled up */
          lcd_fix_string(RhHigh_str);        /* display: Rh+ */

          /* TP1: probe -- Rh -- Vcc */
          R_DDR = 2 << (TP1 * 2);
          R_PORT = 2 << (TP1 * 2);
          Val1 = ReadU_20ms(TP1);

          /* TP1: probe -- Rh -- Vcc */
          R_DDR = 2 << (TP2 * 2);
          R_PORT = 2 << (TP2 * 2);
          Val2 = ReadU_20ms(TP2);

          /* TP1: probe -- Rh -- Vcc */
          R_DDR = 2 << (TP3 * 2);
          R_PORT = 2 << (TP3 * 2);
          Val3 = ReadU_20ms(TP3);

          break;
      }

      /* reset ports to defaults */
      R_DDR = 0;                             /* input mode */
      R_PORT = 0;                            /* all pins low */

      /* display voltages/values of all probes */
      if (DisplayFlag)
      {
        lcd_line(2);                         /* move to line #2 */
        DisplaySignedValue(Val1, 0 , 0);     /* display TP1 */
        lcd_space();
        DisplaySignedValue(Val2, 0 , 0);     /* display TP2 */
        lcd_space();
        DisplaySignedValue(Val3, 0 , 0);     /* display TP3 */
      }

      /* wait and check test push button */
      TempWord = 1000;                            /* timeout in ms */
      if (Counter > 99) TempWord = 0;             /* disable timeout if skipping test */
      TempByte1 = TestKey(TempWord, 0);           /* catch key press or timeout */

      /* short press -> next test / long press -> end selftest */
      if (TempByte1 > 0)
      {
        Counter = 100;                            /* skip current test */
        if (TempByte1 == 2) Test = 100;           /* skip selftest */
      } 
 
      Counter++;                        /* next run */
    }

    Test++;                             /* next one */
  }

  Flag = 1;         /* signal success */
  return Flag;
} 



/*
 *  show calibration values and offsets
 */

void ShowCal(void)
{
  /* display RiL and RiH */
  lcd_clear();
  lcd_fix_string(RiLow_str);            /* display: Ri- */
  lcd_space();
  DisplayValue(Config.RiL, -1, LCD_CHAR_OMEGA);

  lcd_line(2);
  lcd_fix_string(RiHigh_str);           /* display: Ri+ */
  lcd_space();
  DisplayValue(Config.RiH, -1, LCD_CHAR_OMEGA);

  TestKey(3000, TesterMode);            /* let the user read */

  /* display C-Zero */
  lcd_clear();
  lcd_fix_string(CapOffset_str);             /* display: C0 */
  lcd_space();
  DisplayValue(Config.CapZero, -12, 'F');    /* display C0 offset */

  /* display R-Zero */
  lcd_line(2);
  lcd_fix_string(ROffset_str);               /* display: R0 */
  lcd_space();
  DisplayValue(Config.RZero, -2, LCD_CHAR_OMEGA);  /* display R0 */

  TestKey(3000, TesterMode);            /* let the user read */

  /* display offset of bandgap reference */
  lcd_clear();
  lcd_fix_string(URef_str);             /* display: Vref */
  lcd_space();
  DisplaySignedValue(Config.RefOffset, -3, 'V');

  /* display offset of analog comparator */
  lcd_line(2);
  lcd_fix_string(CompOffset_str);
  lcd_space();
  DisplaySignedValue(Config.CompOffset, -3, 'V');

  TestKey(3000, TesterMode);            /* let the user read */
}



/*
 *  self calibration
 *
 *  returns:
 *  - 0 on error
 *  - 1 on success
 */

uint8_t SelfCal(void)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           Test = 1;           /* test counter */
  uint8_t           Counter;            /* loop counter */
  uint8_t           DisplayFlag;        /* display flag */
  unsigned int      Val1 = 0, Val2 = 0, Val3 = 0;   /* voltages */
  uint8_t           CapCounter = 0;     /* number of C_Zero measurements */
  unsigned int      CapSum = 0;         /* sum of C_Zero values */
  uint8_t           RCounter = 0;       /* number of R_Zero measurements */
  unsigned int      RSum = 0;           /* sum of R_Zero values */
  uint8_t           RiL_Counter = 0;    /* number of U_RiL measurements */
  unsigned int      U_RiL = 0;          /* sum of U_RiL values */
  uint8_t           RiH_Counter = 0;    /* number of U_RiH measurements */
  unsigned int      U_RiH = 0;          /* sum of U_RiL values */
  unsigned long     Val0;               /* temp. value */

  /* check for short-circuited probes */
  if (AllProbesShorted() != 3) return Flag;

  /*
   *  measurements
   */

  while (Test <= 5)
  {
    Counter = 1;

    /* repeat each measurement 5 times */
    while (Counter <= 5)
    {
      /* display test number */
      lcd_clear();
      lcd_data('c');                    /* display: c */
      lcd_data('0' + Test);             /* display cal number */
      lcd_space();

      DisplayFlag = 1;        /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* resistance of probe leads */
          lcd_fix_string(ROffset_str);       /* display: R0 */
          lcd_space();
          lcd_fix_string(ProbeComb_str);     /* display: 12 13 23 */          

          UpdateProbes(TP2, TP1, 0);
          Val1 = SmallResistor();
          if (Val1 < 100)                    /* < 1.00 Ohms */
          {
            RSum += Val1;
            RCounter++;
          }

          UpdateProbes(TP3, TP1, 0);
          Val2 = SmallResistor();
          if (Val2 < 100)                    /* < 1.00 Ohms */
          {
            RSum += Val2;
            RCounter++;
          }

          UpdateProbes(TP3, TP2, 0);
          Val3 = SmallResistor();
          if (Val3 < 100)                    /* < 1.00 Ohms */
          {
            RSum += Val3;
            RCounter++;
          }

          break;

        case 2:     /* disconnect probes */
          RemoveShortCircuit();
          Counter = 100;                        /* skip test */
          DisplayFlag = 0;                      /* reset display flag */
          break;

        case 3:     /* internal resistance of µC in pull-down mode */
          lcd_fix_string(RiLow_str);         /* display: Ri- */

          /* TP1:  Gnd -- Ri -- probe -- Rl -- Ri -- Vcc */
          ADC_PORT = 0;
          ADC_DDR = 1 << TP1;
          R_PORT = 1 << (TP1 * 2);
          R_DDR = 1 << (TP1 * 2);
          Val1 = ReadU_5ms(TP1);
          U_RiL += Val1;

          /* TP2: Gnd -- Ri -- probe -- Rl -- Ri -- Vcc */
          ADC_DDR = 1 << TP2;
          R_PORT =  1 << (TP2 * 2);
          R_DDR = 1 << (TP2 * 2);
          Val2 = ReadU_5ms(TP2);
          U_RiL += Val2;

          /* TP3: Gnd -- Ri -- probe -- Rl -- Ri -- Vcc */
          ADC_DDR = 1 << TP3;
          R_PORT =  1 << (TP3 * 2);
          R_DDR = 1 << (TP3 * 2);
          Val3 = ReadU_5ms(TP3);
          U_RiL += Val3;

          RiL_Counter += 3;
          break;

        case 4:     /* internal resistance of µC in pull-up mode */
          lcd_fix_string(RiHigh_str);        /* display: Ri+ */

          /* TP1: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          R_PORT = 0;
          ADC_PORT = 1 << TP1;
          ADC_DDR = 1 << TP1;
          R_DDR = 1 << (TP1 * 2);
          Val1 = UREF_VCC - ReadU_5ms(TP1);
          U_RiH += Val1;

          /* TP2: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          ADC_PORT = 1 << TP2;
          ADC_DDR = 1 << TP2;
          R_DDR = 1 << (TP2 * 2);
          Val2 = UREF_VCC - ReadU_5ms(TP2);
          U_RiH += Val2;

          /* TP3: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          ADC_PORT = 1 << TP3;
          ADC_DDR = 1 << TP3;
          R_DDR = 1 << (TP3 * 2);
          Val3 = UREF_VCC - ReadU_5ms(TP3);
          U_RiH += Val3;

          RiH_Counter += 3;
          break;

        case 5:     /* capacitance offset (PCB and probe leads) */
          lcd_fix_string(CapOffset_str);     /* display: C0 */
          lcd_space();
          lcd_fix_string(ProbeComb_str);     /* display: 12 13 23 */

          MeasureCap(TP2, TP1, 0);
          Val1 = (unsigned int)Caps[0].Raw;
          /* limit offset to 100pF */
          if ((Caps[0].Scale == -12) && (Caps[0].Raw <= 100))
          {
            CapSum += Val1;
            CapCounter++;            
          }

          MeasureCap(TP3, TP1, 1);
          Val2 = (unsigned int)Caps[1].Raw;
          /* limit offset to 100pF */
          if ((Caps[1].Scale == -12) && (Caps[1].Raw <= 100))
          {
            CapSum += Val2;
            CapCounter++;            
          }

          MeasureCap(TP3, TP2, 2);
          Val3 = (unsigned int)Caps[2].Raw;
          /* limit offset to 100pF */
          if ((Caps[2].Scale == -12) && (Caps[2].Raw <= 100))
          {
            CapSum += Val3;
            CapCounter++;            
          }

          break;
      }

      /* reset ports to defaults */
      ADC_DDR = 0;                      /* input mode */
      ADC_PORT = 0;                     /* all pins low */
      R_DDR = 0;                        /* input mode */
      R_PORT = 0;                       /* all pins low */

      /* display values */
      if (DisplayFlag)
      {
        lcd_line(2);                    /* move to line #2 */
        DisplayValue(Val1, 0 , 0);      /* display TP1 */
        lcd_space();
        DisplayValue(Val2, 0 , 0);      /* display TP2 */
        lcd_space();
        DisplayValue(Val3, 0 , 0);      /* display TP3 */
      }

      /* wait and check test push button */
      TempWord = 1000;                            /* timeout in ms */
      if (Counter > 99) TempWord = 0;             /* disable timeout if skipping test */
      TempByte1 = TestKey(TempWord, 0);           /* catch key press or timeout */

      /* short press -> next test / long press -> end cal */
      if (TempByte1 > 0)
      {
        Counter = 100;                            /* skip current test */
        if (TempByte1 == 2) Test = 100;           /* skip selftest */
      } 
 
      Counter++;                        /* next run */
    }

    Test++;                             /* next one */
  }


  /*
   *  calculate values and offsets
   */

  /* capacitance auto-zero */
  if (CapCounter == 15)
  {
    /* calculate average offset (pF) */
    Config.CapZero = CapSum / CapCounter;
    Flag++;
  }

  /* resistance auto-zero */
  if (RCounter == 15)
  { 
    /* calculate average offset (0.01 Ohms) */
    Config.RZero = RSum / RCounter;
    Flag++;
  }

  /* RiL & RiH */
  if ((RiL_Counter == 15) && (RiH_Counter == 15))
  {
    /*
     *  Calculate RiL and RiH using the voltage divider rule:
     *  Ri = Rl * (U_Ri / U_Rl)
     *  - scale up by 100, round up/down and scale down by 10
     */

    /* use values multiplied by 3 to increase accuracy */    
    U_RiL /= 5;                         /* average sum of 3 U_RiL */
    U_RiH /= 5;                         /* average sum of 3 U_RiH */
    Val1 = (UREF_VCC * 3) - U_RiL - U_RiH;  /* U_Rl * 3 */

    /* RiL */
    Val0 = ((unsigned long)R_LOW * 100 * U_RiL) / Val1; /* Rl * U_Ri / U_Rl in 0.01 Ohm */
    Val0 += 5;                                          /* for automagic rounding */
    Val0 /= 10;                                         /* scale down to 0.1 Ohm */
    if (Val0 < 250UL)         /* < 25 Ohms */
    {
      Config.RiL = (unsigned int)Val0;
      Flag++;
    }

    /* RiH */
    Val0 = ((unsigned long)R_LOW * 100 * U_RiH) / Val1; /* Rl * U_Ri / U_Rl in 0.01 Ohm */
    Val0 += 5;                                          /* for automagic rounding */
    Val0 /= 10;                                         /* scale down to 0.1 Ohm */
    if (Val0 < 280UL)         /* < 29 Ohms */
    {
      Config.RiH = (unsigned int)Val0;
      Flag++;
    }
  }

  /* show values and offsets */
  ShowCal();

  if (Flag == 4) Flag = 1;         /* all calibrations done -> success */
  else Flag = 0;                   /* signal error */
  return Flag;
}



/*
 *  calculate checksum for EEPROM stored values and offsets
 */

uint8_t CheckSum(void)
{
  uint8_t           Checksum;

  Checksum = (uint8_t)Config.RiL;
  Checksum += (uint8_t)Config.RiH;
  Checksum += (uint8_t)Config.RZero;
  Checksum += Config.CapZero;
  Checksum += (uint8_t)Config.RefOffset;
  Checksum += (uint8_t)Config.CompOffset;

  return Checksum;
}



/*
 *  save calibration values
 */

void SafeCal(void)
{
  uint8_t           Checksum;

  /*
   *  update values stored in EEPROM
   */

  /* Ri of µC in low mode */
  eeprom_write_word((uint16_t *)&NV_RiL, Config.RiL);

  /* Ri of µC in low mode */
  eeprom_write_word((uint16_t *)&NV_RiH, Config.RiH);

  /* resistance of probe leads */
  eeprom_write_word((uint16_t *)&NV_RZero, Config.RZero);

  /* capacitance offset: PCB + wiring + probe leads */
  eeprom_write_byte((uint8_t *)&NV_CapZero, Config.CapZero);

  /* voltage offset of bandgap reference */
  eeprom_write_byte((uint8_t *)&NV_RefOffset, (uint8_t)Config.RefOffset);

  /* voltage offset of analog comparator */
  eeprom_write_byte((uint8_t *)&NV_CompOffset, (uint8_t)Config.CompOffset);

  /* checksum */
  Checksum = CheckSum();
  eeprom_write_byte((uint8_t *)&NV_Checksum, Checksum);
}



/*
 *  load calibration values
 */

void LoadCal(void)
{
  uint8_t           Checksum;
  uint8_t           Test;

  /*
   *  read stored values from EEPROM
   */

  /* Ri of µC in low mode */ 
  Config.RiL = eeprom_read_word(&NV_RiL);

  /* Ri of µC in low mode */
  Config.RiH = eeprom_read_word(&NV_RiH);

  /* resitance of probe leads */
  Config.RZero = eeprom_read_word(&NV_RZero);

  /* capacitance offset: PCB + wiring + probe leads */
  Config.CapZero = eeprom_read_byte(&NV_CapZero);

  /* voltage offset of bandgap reference */
  Config.RefOffset = (int8_t)eeprom_read_byte((uint8_t *)&NV_RefOffset);

  /* voltage offset of analog comparator */
  Config.CompOffset = (int8_t)eeprom_read_byte((uint8_t *)&NV_CompOffset);

  /* checksum */
  Checksum = eeprom_read_byte(&NV_Checksum);


  /*
   *  check checksum
   */

  Test = CheckSum();

  if (Test != Checksum)
  {
    /* tell user */
    lcd_clear();
    lcd_fix_string(Checksum_str);
    lcd_space();
    lcd_fix_string(Error_str);
    lcd_data('!');
    wait2s();

    /* set default values */
    Config.RiL = R_MCU_LOW;
    Config.RiH = R_MCU_HIGH;
    Config.RZero = R_ZERO;
    Config.CapZero = C_ZERO;
    Config.RefOffset = UREF_OFFSET;
    Config.CompOffset = COMPARATOR_OFFSET;
  }
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

#define MAX_ITEMS   4         /* number of menu items */

  /* menu item selection */
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
          String = (unsigned char *)Selftest_str;
          break;

        case 2:
          String = (unsigned char *)Calibration_str;
          break;

        case 3:
          String = (unsigned char *)Save_str;
          break;

        case 4:
          String = (unsigned char *)Show_str;
          break;
      }

      if (n == Selected) String2 = String;
      lcd_fix_string(String);
      lcd_line(2);
    }

    /* process user feedback */
    n = TestKey(0, 1);             /* wait for testkey */
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
      Run = Selected; 
    }
  }

  /* display item choosen */
  lcd_clear();
  lcd_fix_string(String2);              /* display: <item> */  
  wait1s();

  /* run selected item */
  switch (Run)
  {
    case 1:
      Flag = SelfTest();
      break;

    case 2:
      Flag = SelfCal();
      break;

    case 3:
      SafeCal();
      break;

    case 4:
      ShowCal();
      break;
  }

  /* display end of item */
  lcd_clear();
  lcd_fix_string(String2);              /* display: <item> */
  lcd_line(2);
  if (Flag == 1)
    lcd_fix_string(Done_str);           /* display: done */
  else
    lcd_fix_string(Error_str);          /* display: error */
  lcd_data('!');

#undef MAX_ITEMS
}



/* ************************************************************************
 *   output found components
 * ************************************************************************ */


/*
 *  show failed test
 */

void ShowFail(void)
{
  /* display info */
  lcd_fix_string(Failed1_str);          /* display: No component */
  lcd_line(2);                          /* move to line #2 */
  lcd_fix_string(Failed2_str);          /* display: found!*/  

  /* display numbers of diodes found */
  if (DiodesFound > 0)                  /* diodes found */
  {
    lcd_space();                        /* display space */
    lcd_data(DiodesFound + '0');        /* display number of diodes found */
    lcd_fix_string(Diode_AC_str);       /* display: -|>|- */    
  }

  RunsMissed++;               /* increase counter */
  RunsPassed = 0;             /* reset counter */
}



/*
 *  show diode
 */

void ShowDiode(void)
{
  Diode_Type        *D1;           /* pointer to diode #1 */
  Diode_Type        *D2 = NULL;    /* pointer to diode #2 */
  uint8_t           CFlag = 1;     /* capacitance display flag */
  uint8_t           A = 5;         /* ID of common anode */
  uint8_t           C = 5;         /* ID of common cothode */

  D1 = &Diodes[0];                 /* pointer to first diode */

  if (DiodesFound == 1)            /* single diode */
  {
    C = D1->C;                     /* make anode first pin */
  }
  else if (DiodesFound == 2)       /* two diodes */
  {
    D2 = D1;
    D2++;                          /* pointer to second diode */

    if (D1->A == D2->A)            /* common anode */
    {
      A = D1->A;                   /* save common anode */ 
    }
    else if (D1->C == D2->C)       /* common cathode */
    {
      C = D1->C;                   /* save common cathode */
    }
    else if ((D1->A == D2->C) && (D1->C == D2->A))   /* anti-parallel */
    {
      A = D1->A;                   /* anode and cathode */
      C = A;                       /* are the same */
      CFlag = 0;                   /* disable display of capacitance */
    } 
  }
  else if (DiodesFound == 3)       /* three diodes */
  {
    uint8_t         n;
    uint8_t         m;

    /*
     *  Two diodes in series are additionally detected as third big diode:
     *  - Check for any possible way of 2 diodes be connected in series.
     *  - Only once the cathode of diode #1 matches the anode of diode #2.
     */

    for (n = 0; n <= 2; n++)       /* loop for first diode */
    {
      D1 = &Diodes[n];             /* get pointer of first diode */

      for (m = 0; m <= 2; m++)     /* loop for second diode */
      {
        D2 = &Diodes[m];           /* get pointer of second diode */

        if (n != m)                /* don't check same diode :-) */
        {
          if (D1->C == D2->A)      /* got match */
          {
            n = 5;                 /* end loops */
            m = 5;
          }
        }
      }
    }

    if (n < 5) D2 = NULL;          /* no match found */
    C = D1->C;                     /* cathode of first diode */
    A = 3;                         /* in series mode */
  }
  else                             /* to much diodes */
  {
    D1 = NULL;                     /* don't display any diode */
    ShowFail();                    /* and tell user */
  }


  /*
   *  display pins 
   */

  if (D1)           /* first Diode */
  {
    if (A < 3) lcd_testpin(D1->C);           /* common anode */
    else lcd_testpin(D1->A);                 /* common cathode */

    if (A < 3) lcd_fix_string(Diode_CA_str);   /* common anode */
    else lcd_fix_string(Diode_AC_str);         /* common cathode */

    if (A < 3) lcd_testpin(A);               /* common anode */
    else lcd_testpin(C);                     /* common cathode */
  }

  if (D2)           /* second diode */
  {
    if (A <= 3) lcd_fix_string(Diode_AC_str);   /* common anode or in series */
    else lcd_fix_string(Diode_CA_str);          /* common cathode */

    if (A == C) lcd_testpin(D2->A);          /* anti parallel */
    else if (A <= 3) lcd_testpin(D2->C);     /* common anode or in series */
    else lcd_testpin(D2->A);                 /* common cathode */
  }


  /*
   *  display Uf (forward voltage) and capacitance
   */

  if (D1)                                    /* first diode */
  {
    /* Uf */
    lcd_line(2);                             /* go to line #2 */
    lcd_fix_string(Vf_str);                  /* display: Vf= */
    DisplayValue(D1->V_f, -3, 'V');          /* display Vf */

    if (D2)                                  /* second diode */
    {
      lcd_space();
      DisplayValue(D2->V_f, -3, 'V');        /* display Vf */
    }

    /* capacitance */
    if (CFlag == 1)
    {
      TestKey(3000, TesterMode);             /* next page */
      lcd_clear_line(2);

      lcd_fix_string(DiodeCap_str);          /* display: C= */

      /* get capacitance (opposite of flow direction) */
      MeasureCap(D1->C, D1->A, 0);

      /* and show capacitance */
      DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');

      if (D2)                                /* second diode */
      {
        lcd_space();
        MeasureCap(D2->C, D2->A, 0);
        DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
      }
    }
  }
}



/*
 *  show BJT
 */

void ShowBJT(void)
{
  uint8_t           Counter;       /* counter */
  Diode_Type        *Diode;        /* pointer to diode */
  unsigned int      Vf;            /* forward voltage U_be */
  unsigned char     *String;       /* display string pointer */

  /* display type */
  if (CompType == TYPE_NPN)        /* NPN */
    String = (unsigned char *)NPN_str;
  else                             /* PNP */
    String = (unsigned char *)PNP_str;

  lcd_fix_string(String);          /* display: NPN / PNP */

  /* protections diodes */
  if (DiodesFound > 2)        /* transistor is a set of two diodes :-) */
  {
    lcd_space();
    if (CompType == TYPE_NPN)           /* NPN */
      String = (unsigned char *)Diode_AC_str;
    else                                /* PNP */
      String = (unsigned char *)Diode_CA_str;

    lcd_fix_string(String);     /* display: -|>|- / -|<|- */
  }

  /* display pins */
  lcd_space();
  lcd_fix_string(EBC_str);         /* display: EBC= */
  lcd_testpin(BJT.E);              /* display emitter pin */
  lcd_testpin(BJT.B);              /* display base pin */
  lcd_testpin(BJT.C);              /* display collector pin */

  /* display hfe */
  lcd_line(2);                     /* move to line #2 */ 
  lcd_fix_string(hfe_str);         /* display: B= */
  DisplayValue(BJT.hfe, 0, 0);

  /* display Uf (forward voltage) */
  Diode = &Diodes[0];                   /* get pointer of first diode */  
  Counter = 0;
  while (Counter < DiodesFound)         /* check all diodes */
  {
    /* if the diode matches the transistor */
    if (((Diode->A == BJT.B) &&
         (Diode->C == BJT.E) &&
         (CompType == TYPE_NPN)) ||
        ((Diode->A == BJT.E) &&
         (Diode->C == BJT.B) &&
         (CompType == TYPE_PNP)))
    {
      /* not enough space on LCD for large hfe and Vf */
      if (BJT.hfe < 1000)                    /* small hfe */
      {
        lcd_space();                         /* display space */
      }
      else                                   /* line to short */
      {
        TestKey(3000, TesterMode);           /* next page */
        lcd_clear_line(2);
      }

      lcd_fix_string(Vf_str);                /* display: Vf= */

      /*
       *  Vf is quite linear for a logarithmicly scaled I_b.
       *  So we may interpolate the Vf values of low and high test current
       *  measurements for a virtual test current. Low test current is 10µA
       *  and high test current is 7mA. That's a logarithmic scale of
       *  3 decades.
       */

      /* calculate slope for one decade */
      TempInt = Diode->V_f - Diode->V_f2;
      TempInt /= 3;

      /* select Vf based on hfe */
      if (BJT.hfe < 100)                /* low hfe */
      {
        /*
         *  BJTs with low hfe are power transistors and need a large I_b
         *  to drive the load. So we simply take Vf of the high test current
         *  measurement (7mA). 
         */

        Vf = Diode->V_f;
      }
      else if (BJT.hfe < 250)           /* mid-range hfe */
      {
        /*
         *  BJTs with a mid-range hfe are signal transistors and need
         *  a small I_b to drive the load. So we interpolate Vf for
         *  a virtual test current of about 1mA.
         */

        Vf = Diode->V_f - TempInt;
      }
      else                              /* high hfe */
      {
        /*
         *  BJTs with a high hfe are small signal transistors and need
         *  only a very small I_b to drive the load. So we interpolate Vf
         *  for a virtual test current of about 0.1mA.
         */

        Vf = Diode->V_f2 + TempInt;
      }

      DisplayValue(Vf, -3, 'V');
      Counter = DiodesFound;                 /* end loop */
    }
    else
    {
      Counter++;                        /* increase counter */
      Diode++;                          /* next one */
    }
  }
}



/*
 *  show FET
 */

void ShowFET(void)
{
  uint8_t           Data;          /* temp. data */

  /* display type */
  if (CompType & TYPE_MOSFET)      /* MOSFET */
    lcd_fix_string(MOS_str);         /* display: MOS */
  else                             /* JFET */
    lcd_data('J');                   /* display: J */
  lcd_fix_string(FET_str);         /* display: FET */ 

  /* display channel type */
  lcd_space();
  if (CompType & TYPE_N_CHANNEL)   /* n-channel */
    Data = 'N';
  else                             /* p-channel */
    Data = 'P';

  lcd_data(Data);                  /* display: N / P */
  lcd_fix_string(Channel_str);     /* display: -ch */
      
  /* display mode */
  if (CompType & TYPE_MOSFET)      /* MOSFET */
  {
    lcd_space();
    if (CompType & TYPE_ENHANCEMENT)    /* enhancement mode */
      lcd_fix_string(Enhancement_str);
    else                                /* depletion mode */
      lcd_fix_string(Depletion_str);
  }

  /* pins */
  lcd_line(2);                     /* move to line #2 */ 
  lcd_fix_string(GDS_str);         /* display: GDS= */
  lcd_testpin(FET.G);              /* display gate pin */
  lcd_testpin(FET.D);              /* display drain pin */
  lcd_testpin(FET.S);              /* display source pin */

  /* extra data for MOSFET in enhancement mode */
  if (CompType & (TYPE_ENHANCEMENT | TYPE_MOSFET))
  {
    /* protection diode */
    if (DiodesFound > 0)
    {
      lcd_space();                      /* display space */
      lcd_data(LCD_CHAR_DIODE1);        /* display diode symbol */
    }

    TestKey(3000, TesterMode);          /* next page */
    lcd_clear();

    /* gate threshold voltage */
    lcd_fix_string(Vth_str);            /* display: Vth */
    DisplayValue(FET.V_th, -3, 'V');    /* display V_th in mV */    

    lcd_line(2);

    /* display gate capacitance */
    lcd_fix_string(GateCap_str);        /* display: Cgs= */
    MeasureCap(FET.G, FET.S, 0);        /* measure capacitance */
    /* display value and unit */
    DisplayValue(Caps[0].Value, Caps[0].Scale, 'F');
  }
}



/*
 *   show special components like Thyristor and Triac
 */

void ShowSpecial(void)
{
  /* display component type */
  if (CompFound == COMP_THYRISTOR)
  {
    lcd_fix_string(Thyristor_str);      /* display: thyristor */
  }
  else if (CompFound == COMP_TRIAC)
  {
    lcd_fix_string(Triac_str);          /* display: triac */
  }

  /* display pins */
  lcd_line(2);                          /* move to line #2 */ 
  lcd_fix_string(GAK_str);              /* display: GAK */
  lcd_testpin(BJT.B);                   /* display gate pin */
  lcd_testpin(BJT.C);                   /* display anode pin */
  lcd_testpin(BJT.E);                   /* display cathode pin */
}



/*
 *  show resistor
 */

void ShowResistor(void)
{
  Resistor_Type     *R1;           /* pointer to resistor #1 */
  Resistor_Type     *R2;           /* pointer to resistor #2 */
  uint8_t           Pin;           /* ID of common pin */

  R1 = &Resistors[0];              /* pointer to first resistor */

  if (ResistorsFound == 1)         /* single resistor */
  {
    R2 = NULL;                     /* disable second resistor */
    Pin = R1->A;                   /* make B the first pin */
  }
  else                             /* multiple resistors */
  {
    R2 = R1;
    R2++;                          /* pointer to second resistor */

    if (ResistorsFound == 3)       /* three resistors */
    {
      Resistor_Type     *Rmax;     /* pointer to largest resistor */    

      /*
       *  3 resistors mean 2 single resistors and both resitors in series.
       *  So we have to single out that series resistor by finding the
       *  largest resistor.
       */

      Rmax = R1;                   /* starting point */
      for (Pin = 1; Pin <= 2; Pin++)
      {
        if (CmpValue(R2->Value, R2->Scale, Rmax->Value, Rmax->Scale) == 1)
        {
          Rmax = R2;          /* update largest one */
        }

        R2++;                 /* next one */
      }

      /* get the two smaller resistors */
      if (R1 == Rmax) R1++;
      R2 = R1;
      R2++;
      if (R2 == Rmax) R2++;
    }

    /* find common pin of both resistors */
    if ((R1->A == R2->A) || (R1->A == R2->B)) Pin = R1->A;
    else Pin = R1->B;
  }


  /*
   *  display the pins
   */

  /* first resistor */
  if (R1->A != Pin) lcd_testpin(R1->A);
  else lcd_testpin(R1->B);
  lcd_fix_string(Resistor_str);
  lcd_testpin(Pin);

  if (R2)           /* second resistor */
  {
    lcd_fix_string(Resistor_str);
    if (R2->A != Pin) lcd_testpin(R2->A);
    else lcd_testpin(R2->B);
  }


  /*
   *  display the values
   */

  /* first resistor */
  lcd_line(2);
  DisplayValue(R1->Value, R1->Scale, LCD_CHAR_OMEGA);

  if (R2)           /* second resistor */
  {
    lcd_space();
    DisplayValue(R2->Value, R2->Scale, LCD_CHAR_OMEGA);
  }
}



/*
 *  show capacitor
 */

void ShowCapacitor(void)
{
  Capacitor_Type    *MaxCap;       /* pointer to largest cap */
  Capacitor_Type    *Cap;          /* pointer to cap */

  /* find largest cap */
  MaxCap = &Caps[0];               /* pointer to first cap */
  Cap = MaxCap;

  for (TempByte1 = 1; TempByte1 <= 2; TempByte1++) 
  {
    Cap++;                              /* next cap */

    if (CmpValue(Cap->Value, Cap->Scale, MaxCap->Value, MaxCap->Scale) == 1)
    {
      MaxCap = Cap;
    }
  }

  /* display largest cap */
  lcd_testpin(MaxCap->A);               /* display pin #1 */
  lcd_fix_string(Cap_str);              /* display capacitor symbol */
  lcd_testpin(MaxCap->B);               /* display pin #2 */
  lcd_line(2);                          /* move to line #2 */
  /* and show capacitance */
  DisplayValue(MaxCap->Value, MaxCap->Scale, 'F');
}



/* ************************************************************************
 *   the one and only main()
 * ************************************************************************ */


/*
 *  main function
 */

int main(void)
{

  /*
   *  init
   */

  /* switch on power to keep me alive */
  CONTROL_DDR = (1 << POWER_CTRL);      /* set pin as output */
  CONTROL_PORT = (1 << POWER_CTRL);     /* set pin to drive power management transistor */

  /* setup µC */
  MCUCR = (1 << PUD);                        /* disable pull-up resistors globally */
  ADCSRA = (1 << ADEN) | ADC_CLOCK_DIV;      /* enable ADC and set clock divider */

  /* catch watchdog */  
  TempByte1 = (MCUSR & (1 << WDRF));    /* save watchdog flag */
  MCUSR &= ~(1 << WDRF);                /* reset watchdog flag */
  wdt_disable();                        /* disable watchdog */


  /*
   *  watchdog was triggered (timeout 2s)
   *  - This is after the µC done a reset driven by the watchdog.
   *  - Does only work if the capacitor at the base of the power management
   *    transistor is large enough to survive a µC reset. Otherwise the
   *    tester simply looses power.
   */

  if (TempByte1)
  {
    lcd_clear();                        /* display was initialized before */
    lcd_fix_string(Timeout_str);        /* display: timeout */
    lcd_line(2);
    lcd_fix_string(Error_str);          /* display: error */
    lcd_data('!');
    wait2s();                           /* give user some time to read */
    CONTROL_PORT = 0;                   /* power off myself */
    return 0;                           /* exit program */
  }


  /*
   *  init LCD module and load custom characters
   */

  lcd_init();                           /* initialize LCD */

  /* symbols for components */
  lcd_fix_customchar(DiodeIcon1, LCD_CHAR_DIODE1);     /* diode symbol '|>|' */
  lcd_fix_customchar(DiodeIcon2, LCD_CHAR_DIODE2);     /* diode symbol '|<|' */
  lcd_fix_customchar(CapIcon, LCD_CHAR_CAP);           /* capacitor symbol '||' */
  lcd_fix_customchar(ResIcon1, LCD_CHAR_RESIS1);       /* resistor symbol '[' */
  lcd_fix_customchar(ResIcon2, LCD_CHAR_RESIS2);       /* resistor symbol ']' */

  /* kyrillish LCD character set lacks omega and µ */ 
  #ifdef LCD_CYRILLIC
    lcd_fix_customchar(OmegaIcon, LCD_CHAR_OMEGA);     /* Omega */
    lcd_fix_customchar(MicroIcon, LCD_CHAR_MICRO);     /* µ / micro */
  #endif

  /* return to normal output */
  lcd_line(1);                               /* move to line #1 */


  /*
   *  operation mode selection
   */

  TesterMode = MODE_CONTINOUS;               /* set default mode: continous */

  /* catch long key press */
  if (!(CONTROL_PIN & (1 << TEST_BUTTON)))   /* if test button is pressed */
  {
    wait300ms();                             /* wait to catch a long key press */
    if (!(CONTROL_PIN & (1 << TEST_BUTTON))) /* if button is still pressed */
      TesterMode = MODE_AUTOHOLD;            /* set auto-hold mode */
  }

  /* output operation mode */
  lcd_fix_string(Mode_str);                  /* display: tester mode */
  lcd_line(2);                               /* move to line #2 */
  if (TesterMode == MODE_CONTINOUS)          /* if continous mode */
    lcd_fix_string(Continous_str);             /* display: continous */
  else if (TesterMode == MODE_AUTOHOLD)      /* if auto-hold mode */
    lcd_fix_string(AutoHold_str);              /* display: auto-hold */
  wait2s();                                  /* give user some time to read */


  /*
   *  init variables
   */

  /* cycling */
  RunsMissed = 0;
  RunsPassed = 0;

  /* default offsets and values */
  Config.Samples = ADC_SAMPLES;         /* number of ADC samples */
  Config.AutoScale = 1;                 /* enable ADC auto scaling */
  Config.RefFlag = 1;                   /* no ADC reference set yet */
  LoadCal();                            /* load calibration values */

  wdt_enable(WDTO_2S);		        /* enable watchdog (timeout 2s) */


  /*
   *  main processing cycle
   */

start:

  /* reset variabels */
  CompFound = COMP_NONE;
  CompType = 0;
  CompDone = 0;
  DiodesFound = 0;
  ResistorsFound = 0;
  BJT.hfe = 0;

  /* reset hardware */
  ADC_DDR = 0;                     /* set all pins of ADC port as input */ 
  lcd_clear();                     /* clear LCD */

  /* internal bandgap reference */
  Config.U_Bandgap = ReadU(0x0e);       /* dummy read for bandgap stabilization */
  Config.Samples = 200;                 /* do a lot of samples for high accuracy */
  Config.U_Bandgap = ReadU(0x0e);       /* get voltage of bandgap reference */
  Config.Samples = ADC_SAMPLES;         /* set samples back to default */
  Config.U_Bandgap += Config.RefOffset; /* add voltage offset */ 


  /*
   *  battery check
   */

  /* get current voltage */
  TempWord = ReadU(5);                  /* read voltage of ADC5 in mV */

  /*
   *  ADC pin is connected to a voltage divider Rh = 10k and Rl = 3k3.
   *  Ul = (Uin / (Rh + Rl)) * Rl  ->  Uin = (Ul * (Rh + Rl)) / Rl
   *  Uin = (Ul * (10k + 3k3)) / 3k3 = 4 * Ul  
   */

  TempWord *= 4;                        /* calculate U_bat (mV) */
  TempWord += BAT_OFFSET;               /* add offset for voltage drop */

  /* display battery voltage */
  lcd_fix_string(Battery_str);          /* display: Bat. */
  lcd_space();
  DisplayValue(TempWord / 10, -2, 'V'); /* display battery voltage */
  lcd_space();

  /* check limits */
  if (TempWord < BAT_POOR)              /* low level reached */
  {
    lcd_fix_string(Low_str);            /* display: low */
    wait2s();                           /* let user read info */
    goto power_off;                     /* power off */
  }
  else if (TempWord < BAT_POOR + 1000)  /* warning level reached */
  {
    lcd_fix_string(Weak_str);           /* display: weak */
  }
  else                                  /* ok */
  {
    lcd_fix_string(OK_str);             /* display: ok */
  }


  /*
   *  probing
   */

  /* display start of probing */
  lcd_line(2);                     /* move to line #2 */
  lcd_fix_string(Running_str);     /* display: probing... */

  /* try to discharge any connected component */
  DischargeProbes();
  if (CompFound == COMP_CELL)      /* detected a voltage supply */
  {
    goto end;                      /* new cycle */
  }

  /* enter main menu if requested by short-circuiting all probes */
  if (AllProbesShorted() == 3)
  {
    MainMenu();                    /* enter mainmenu */;
    goto end;                      /* new cycle after job is is done */
  }

  /* check all 6 combinations of the 3 probes */ 
  CheckProbes(TP1, TP2, TP3);
  CheckProbes(TP2, TP1, TP3);
  CheckProbes(TP1, TP3, TP2);
  CheckProbes(TP3, TP1, TP2);
  CheckProbes(TP2, TP3, TP1);
  CheckProbes(TP3, TP2, TP1);
  
  /* if component might be a capacitor */
  if ((CompFound == COMP_NONE) ||
      (CompFound == COMP_RESISTOR) ||
      (CompFound == COMP_DIODE))
  {
    /* check all possible combinations */
    MeasureCap(TP3, TP1, 0);
    MeasureCap(TP3, TP2, 1);
    MeasureCap(TP2, TP1, 2);
  }


  /*
   *  output test results
   */

  lcd_clear();                     /* clear LCD */

  /* call output function based on component type */
  switch (CompFound)
  {
    case COMP_DIODE:
      ShowDiode();
      break;

    case COMP_BJT:
      ShowBJT();
      break;

    case COMP_FET:
      ShowFET();
      break;

    case COMP_THYRISTOR:
      ShowSpecial();
      break;

    case COMP_TRIAC:
      ShowSpecial();
      break;

    case COMP_RESISTOR:
      ShowResistor();
      break;

    case COMP_CAPACITOR:
      ShowCapacitor();
      break;

    default:                  /* no component found */
      ShowFail();
      goto end;
  }

  /* component was found */
  RunsMissed = 0;             /* reset counter */
  RunsPassed++;               /* increase counter */


  /*
   *  take care about cycling and power-off
   */

end:

  /* get key press or timeout */
  TempByte1 = TestKey((unsigned int)CYCLE_DELAY, TesterMode);

  if (TempByte1 == 1) goto start;            /* short key press -> next round */
  else if (TempByte1 == 2) goto power_off;   /* long key press -> power off */

  /* check if we should go for another round */
  if ((RunsMissed < CYCLE_MAX) && (RunsPassed < CYCLE_MAX * 2))
  {
    goto start;               /* another round */
  }


power_off:

  /* display feedback (otherwise the user will wait ...) */
  lcd_clear();
  lcd_fix_string(Done_str);             /* display: done */
  lcd_line(2);
  lcd_fix_string(Version_str);          /* display firmware version */

  wdt_disable();                        /* disable watchdog */
  CONTROL_PORT &= ~(1 << POWER_CTRL);   /* power off myself */

  return 0;
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef MAIN_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

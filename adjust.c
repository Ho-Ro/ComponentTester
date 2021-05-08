/* ************************************************************************
 *
 *   self adjustment functions
 *
 *   (c) 2012-2015 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define ADJUST_C


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
 *   storage of adjustment values in EEPROM
 * ************************************************************************ */


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
 *  save adjustment values
 */

void SafeAdjust(void)
{
  uint8_t           Checksum;

  /*
   *  update values stored in EEPROM
   */

  /* Ri of MCU in low mode */
  eeprom_write_word((uint16_t *)&NV_RiL, Config.RiL);

  /* Ri of MCU in low mode */
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
 *  load adjustment values
 */

void LoadAdjust(void)
{
  uint8_t           Checksum;
  uint8_t           Test;

  /*
   *  read stored values from EEPROM
   */

  /* Ri of MCU in low mode */ 
  Config.RiL = eeprom_read_word(&NV_RiL);

  /* Ri of MCU in low mode */
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
    LCD_Clear();
    LCD_EEString2(Checksum_str);        /* display: Checksum */
    LCD_EEString(Error_str);            /* display: error! */
    MilliSleep(2000);                   /* give user some time to read */

    /* set default values */
    Config.RiL = R_MCU_LOW;
    Config.RiH = R_MCU_HIGH;
    Config.RZero = R_ZERO;
    Config.CapZero = C_ZERO;
    Config.RefOffset = UREF_OFFSET;
    Config.CompOffset = COMPARATOR_OFFSET;
  }
}



/* ************************************************************************
 *   self adjustment
 * ************************************************************************ */


/*
 *  show adjustment values and offsets
 */

void ShowAdjust(void)
{
  /* display RiL and RiH */
  LCD_Clear();
  LCD_EEString2(RiLow_str);             /* display: Ri- */
  DisplayValue(Config.RiL, -1, LCD_CHAR_OMEGA);

  LCD_Line2();
  LCD_EEString2(RiHigh_str);            /* display: Ri+ */
  DisplayValue(Config.RiH, -1, LCD_CHAR_OMEGA);

  WaitKey();                  /* let the user read */

  /* display C-Zero */
  LCD_Clear();
  LCD_EEString2(CapOffset_str);              /* display: C0 */
  DisplayValue(Config.CapZero, -12, 'F');    /* display C0 offset */

  /* display R-Zero */
  LCD_Line2();
  LCD_EEString2(ROffset_str);                      /* display: R0 */
  DisplayValue(Config.RZero, -2, LCD_CHAR_OMEGA);  /* display R0 */

  WaitKey();                  /* let the user read */

  /* display internal bandgap reference */
  LCD_Clear();
  LCD_EEString2(URef_str);                   /* display: Vref */
  DisplayValue(Config.Bandgap, -3, 'V');     /* display bandgap ref */

  /* display Vcc */
  LCD_Line2();
  LCD_EEString2(Vcc_str);                    /* display: Vcc */
  DisplayValue(Config.Vcc, -3, 'V');         /* display Vcc */

  WaitKey();                  /* let the user read */

  /* display offset of analog comparator */
  LCD_Clear();
  LCD_EEString2(CompOffset_str);             /* display: AComp */
  DisplaySignedValue(Config.CompOffset, -3, 'V');

  WaitKey();                  /* let the user read */
}



/*
 *  self adjustment
 *
 *  returns:
 *  - 0 on error
 *  - 1 on success
 */

uint8_t SelfAdjust(void)
{
  uint8_t           Flag = 0;           /* return value */
  uint8_t           Test = 1;           /* test counter */
  uint8_t           Counter;            /* loop counter */
  uint8_t           DisplayFlag;        /* display flag */
  uint16_t          Val1 = 0, Val2 = 0, Val3 = 0;   /* voltages */
  uint8_t           CapCounter = 0;     /* number of C_Zero measurements */
  uint16_t          CapSum = 0;         /* sum of C_Zero values */
  uint8_t           RCounter = 0;       /* number of R_Zero measurements */
  uint16_t          RSum = 0;           /* sum of R_Zero values */
  uint8_t           RiL_Counter = 0;    /* number of U_RiL measurements */
  uint16_t          U_RiL = 0;          /* sum of U_RiL values */
  uint8_t           RiH_Counter = 0;    /* number of U_RiH measurements */
  uint16_t          U_RiH = 0;          /* sum of U_RiL values */
  uint32_t          Val0;               /* temp. value */


  /*
   *  measurements
   */

  /* make sure all probes are shorted */
  Counter = ShortCircuit(1);
  if (Counter == 0) Test = 10;      /* skip adjustment on error */

  while (Test <= 5)      /* loop through tests */
  {
    Counter = 1;

    /* repeat each measurement 5 times */
    while (Counter <= 5)
    {
      /* display test number */
      LCD_Clear();
      LCD_Data('A');                    /* display: a */
      LCD_Data('0' + Test);             /* display number */
      LCD_Space();

      DisplayFlag = 1;        /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* resistance of probe leads (probes shorted) */
          LCD_EEString2(ROffset_str);   /* display: R0 */
          LCD_EEString(ProbeComb_str);  /* display: 12 13 23 */          

          /*
           *  The resistance is for two probes in series and we expect it to be
           *  smaller than 1.00 Ohms, i.e. 0.50 Ohms for a single probe
           */

          UpdateProbes(TP2, TP1, 0);
          Val1 = SmallResistor(0);
          if (Val1 < 100)                    /* within limit */
          {
            RSum += Val1;
            RCounter++;
          }

          UpdateProbes(TP3, TP1, 0);
          Val2 = SmallResistor(0);
          if (Val2 < 100)                    /* whithin limit */
          {
            RSum += Val2;
            RCounter++;
          }

          UpdateProbes(TP3, TP2, 0);
          Val3 = SmallResistor(0);
          if (Val3 < 100)                    /* within limit */
          {
            RSum += Val3;
            RCounter++;
          }

          break;

        case 2:     /* un-short probes */
          ShortCircuit(0);              /* make sure probes are not shorted */
          Counter = 100;                /* skip test */
          DisplayFlag = 0;              /* reset display flag */
          break;

        case 3:     /* internal resistance of MCU in pull-down mode */
          LCD_EEString(RiLow_str);      /* display: Ri- */

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

        case 4:     /* internal resistance of MCU in pull-up mode */
          LCD_EEString(RiHigh_str);     /* display: Ri+ */

          /* TP1: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          R_PORT = 0;
          ADC_PORT = 1 << TP1;
          ADC_DDR = 1 << TP1;
          R_DDR = 1 << (TP1 * 2);
          Val1 = Config.Vcc - ReadU_5ms(TP1);
          U_RiH += Val1;

          /* TP2: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          ADC_PORT = 1 << TP2;
          ADC_DDR = 1 << TP2;
          R_DDR = 1 << (TP2 * 2);
          Val2 = Config.Vcc - ReadU_5ms(TP2);
          U_RiH += Val2;

          /* TP3: Gnd -- Ri -- Rl -- probe -- Ri -- Vcc */
          ADC_PORT = 1 << TP3;
          ADC_DDR = 1 << TP3;
          R_DDR = 1 << (TP3 * 2);
          Val3 = Config.Vcc - ReadU_5ms(TP3);
          U_RiH += Val3;

          RiH_Counter += 3;
          break;

        case 5:     /* capacitance offset (PCB and probe leads) */
          LCD_EEString2(CapOffset_str);   /* display: C0 */
          LCD_EEString(ProbeComb_str);    /* display: 12 13 23 */

          /*
           *  The capacitance is for two probes and we expect it to be
           *  less than 100pF.
           */

          MeasureCap(TP2, TP1, 0);
          Val1 = (uint16_t)Caps[0].Raw;
          /* limit offset to 100pF */
          if ((Caps[0].Scale == -12) && (Caps[0].Raw <= 100))
          {
            CapSum += Val1;
            CapCounter++;            
          }

          MeasureCap(TP3, TP1, 1);
          Val2 = (uint16_t)Caps[1].Raw;
          /* limit offset to 100pF */
          if ((Caps[1].Scale == -12) && (Caps[1].Raw <= 100))
          {
            CapSum += Val2;
            CapCounter++;            
          }

          MeasureCap(TP3, TP2, 2);
          Val3 = (uint16_t)Caps[2].Raw;
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
        LCD_Line2();                    /* move to line #2 */
        DisplayValue(Val1, 0 , 0);      /* display TP1 */
        LCD_Space();
        DisplayValue(Val2, 0 , 0);      /* display TP2 */
        LCD_Space();
        DisplayValue(Val3, 0 , 0);      /* display TP3 */
      }

      /* wait and check test push button */
      if (Counter < 100)                     /* when we don't skip this test */
      {
        DisplayFlag = TestKey(1000, 0);      /* catch key press or timeout */

        /* short press -> next test / long press -> end selftest */
        if (DisplayFlag > 0)
        {
          Counter = 100;                       /* skip current test anyway */
          if (DisplayFlag == 2) Test = 100;    /* also skip selftest */
        } 
      }
 
      Counter++;                        /* next run */
    }

    Test++;                             /* next one */
  }


  /*
   *  calculate values and offsets
   */

  /* capacitance auto-zero: calculate average value for all probe pairs */
  if (CapCounter == 15)
  {
    /* calculate average offset (pF) */
    Config.CapZero = CapSum / CapCounter;
    Flag++;
  }

  /* resistance auto-zero: calculate average value for all probes pairs */
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
    Val1 = (Config.Vcc * 3) - U_RiL - U_RiH;  /* U_Rl * 3 */

    /* RiL */
    Val0 = ((uint32_t)R_LOW * 100 * U_RiL) / Val1;     /* Rl * U_Ri / U_Rl in 0.01 Ohm */
    Val0 += 5;                                         /* for automagic rounding */
    Val0 /= 10;                                        /* scale down to 0.1 Ohm */
    if (Val0 < 250UL)         /* < 25 Ohms */
    {
      Config.RiL = (uint16_t)Val0;
      Flag++;
    }

    /* RiH */
    Val0 = ((uint32_t)R_LOW * 100 * U_RiH) / Val1;     /* Rl * U_Ri / U_Rl in 0.01 Ohm */
    Val0 += 5;                                         /* for automagic rounding */
    Val0 /= 10;                                        /* scale down to 0.1 Ohm */
    if (Val0 < 280UL)         /* < 29 Ohms */
    {
      Config.RiH = (uint16_t)Val0;
      Flag++;
    }
  }

  /* show values and offsets */
  ShowAdjust();

  if (Flag == 4) Flag = 1;         /* all adjustments done -> success */
  else Flag = 0;                   /* signal error */

  return Flag;
}



/* ************************************************************************
 *   selftest
 * ************************************************************************ */


/*
 *  selftest
 *  - perform measurements on internal voltages and probe resistors
 *  - display results
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
  uint16_t          Val0;               /* voltage/value */
  int16_t           Val1 = 0, Val2 = 0, Val3 = 0;   /* voltages/values */

  /* make sure all probes are shorted */
  Counter = ShortCircuit(1);
  if (Counter == 0) Test = 10;     /* skip selftest */

  /* loop through all tests */
  while (Test <= 6)
  {
    Counter = 1;

    /* repeat each test 5 times */
    while (Counter <= 5)
    {
      /* display test number */
      LCD_Clear();
      LCD_Data('T');                    /* display: T */
      LCD_Data('0' + Test);             /* display test number */
      LCD_Space();

      DisplayFlag = 1;                  /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* reference voltage */
          Val0 = ReadU(0x0e);           /* dummy read for bandgap stabilization */
          Val0 = ReadU(0x0e);           /* read bandgap reference voltage */ 
          LCD_EEString(URef_str);       /* display: Vref */

          LCD_Line2();
          DisplayValue(Val0, -3, 'V');       /* display voltage in mV */

          DisplayFlag = 0;                   /* reset flag */
          break;

        case 2:     /* compare Rl resistors (probes still shorted) */
          LCD_EEString2(Rl_str);          /* display: +Rl- */
          LCD_EEString(ProbeComb_str);    /* display: 12 13 23 */

          /* set up a voltage divider with the Rl's */
          /* substract theoretical voltage of voltage divider */

          /* TP1: Gnd -- Rl -- probe-2 -- probe-1 -- Rl -- Vcc */
          R_PORT = 1 << (TP1 * 2);
          R_DDR = (1 << (TP1 * 2)) | (1 << (TP2 * 2));
          Val1 = ReadU_20ms(TP3);
          Val1 -= ((int32_t)Config.Vcc * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          /* TP1: Gnd -- Rl -- probe-3 -- probe-1 -- Rl -- Vcc */
          R_DDR = (1 << (TP1 * 2)) | (1 << (TP3 * 2));
          Val2 = ReadU_20ms(TP2);
          Val2 -= ((int32_t)Config.Vcc * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          /* TP1: Gnd -- Rl -- probe-3 -- probe-2 -- Rl -- Vcc */
          R_PORT = 1 << (TP2 * 2);
          R_DDR = (1 << (TP2 * 2)) | (1 << (TP3 * 2));
          Val3 = ReadU_20ms(TP2);
          Val3 -= ((int32_t)Config.Vcc * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          break;

        case 3:     /* compare Rh resistors (probes still shorted) */
          LCD_EEString2(Rh_str);          /* display: +Rh- */
          LCD_EEString(ProbeComb_str);    /* display: 12 13 23 */

          /* set up a voltage divider with the Rh's */

          /* TP1: Gnd -- Rh -- probe-2 -- probe-1 -- Rh -- Vcc */
          R_PORT = 2 << (TP1 * 2);
          R_DDR = (2 << (TP1 * 2)) | (2 << (TP2 * 2));
          Val1 = ReadU_20ms(TP3);
          Val1 -= (Config.Vcc / 2);

          /* TP1: Gnd -- Rh -- probe-3 -- probe-1 -- Rh -- Vcc */
          R_DDR = (2 << (TP1 * 2)) | (2 << (TP3 * 2));
          Val2 = ReadU_20ms(TP2);
          Val2 -= (Config.Vcc / 2);

          /* TP1: Gnd -- Rh -- probe-3 -- probe-2 -- Rh -- Vcc */
          R_PORT = 2 << (TP2 * 2);
          R_DDR = (2 << (TP2 * 2)) | (2 << (TP3 * 2));
          Val3 = ReadU_20ms(TP1);
          Val3 -= (Config.Vcc / 2);

          break;

        case 4:     /* un-short probes */
          ShortCircuit(0);         /* make sure probes are not shorted */
          Counter = 100;           /* skip test */
          DisplayFlag = 0;         /* reset flag */
          break;

        case 5:     /* Rh resistors pulled down */
          LCD_EEString(RhLow_str);      /* display: Rh- */

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
          LCD_EEString(RhHigh_str);     /* display: Rh+ */

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
        LCD_Line2();                         /* move to line #2 */
        DisplaySignedValue(Val1, 0 , 0);     /* display TP1 */
        LCD_Space();
        DisplaySignedValue(Val2, 0 , 0);     /* display TP2 */
        LCD_Space();
        DisplaySignedValue(Val3, 0 , 0);     /* display TP3 */
      }

      /* wait and check test push button */
      if (Counter < 100)                     /* when we don't skip this test */
      {
        DisplayFlag = TestKey(1000, 0);      /* catch key press or timeout */

        /* short press -> next test / long press -> end selftest */
        if (DisplayFlag > 0)
        {
          Counter = 100;                       /* skip current test anyway */
          if (DisplayFlag == 2) Test = 100;    /* also skip selftest */
        } 
      }
 
      Counter++;                        /* next run */
    }

    Test++;                             /* next one */
  }

  Flag = 1;         /* signal success */
  return Flag;
} 



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef ADJUST_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */

/* ************************************************************************
 *
 *   self adjustment functions
 *
 *   (c) 2012-2016 by Markus Reschke
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
#include "functions.h"        /* external functions */
#include "colors.h"           /* color definitions */



/* ************************************************************************
 *   storage of adjustment values in EEPROM
 * ************************************************************************ */


/*
 *  set default adjustment values
 */

void SetAdjustDefaults(void)
{
  /* set default values */
  NV.RiL = R_MCU_LOW;
  NV.RiH = R_MCU_HIGH;
  NV.RZero = R_ZERO;
  NV.CapZero = C_ZERO;
  NV.RefOffset = UREF_OFFSET;
  NV.CompOffset = COMPARATOR_OFFSET;
  NV.Contrast = LCD_CONTRAST;
}



/*
 *  calculate checksum for adjustment values
 */

uint8_t CheckSum(void)
{
  uint8_t      Checksum = 0;            /* checksum / return value */
  uint8_t      n;                       /* counter */
  uint8_t      *Data = (uint8_t *)&NV;  /* pointer to RAM */

  /* we simply add all bytes, besides the checksum */
  for (n = 0; n < (sizeof(NV_Type) - 1); n++)
  {  
    Checksum += *Data;
  }

  /* fix for zero (not updated yet) */
  if (Checksum == 0) Checksum++;

  return Checksum;
}



/*
 *  load/save adjustment values from/to EEPROM
 *
 *  requires:
 *  - mode: load/save
 *  - ID: profile ID
 */

void ManageAdjust(uint8_t Mode, uint8_t ID)
{
  uint8_t      n;                            /* counter */
  uint8_t      *Addr_RAM = (uint8_t *)&NV;   /* pointer to RAM */
  uint8_t      *Addr_EE;                     /* pointer to EEPROM */

  /* determine EEPROM address */
  if (ID == 2)                     /* profile #2 */
  {
    Addr_EE = (uint8_t *)&NV_EE2;
  }
  else                             /* profile #1 */
  {
    Addr_EE = (uint8_t *)&NV_EE;
  }

  NV.CheckSum = CheckSum();        /* update checksum */


  /*
   *  read/write EEPROM byte-wise to/from data structure 
   */

  for (n = 0; n < sizeof(NV_Type); n++)
  {
    if (Mode == MODE_SAVE)              /* write */
    {
      eeprom_write_byte(Addr_EE, *Addr_RAM);    /* write a byte */
    }
    else                                /* read */
    {
      *Addr_RAM = eeprom_read_byte(Addr_EE);    /* read a byte */
    }

    Addr_RAM++;               /* next byte */
    Addr_EE++;                /* next byte */
  }


  /*
   *  check checksum on read
   */

  if (Mode != MODE_SAVE)           /* read mode */
  {
    n = CheckSum();

    if (NV.CheckSum != 0)          /* EEPROM updated */
    {
      if (NV.CheckSum != n)        /* mismatch */
      {
        /* tell user */
        LCD_Clear();
        LCD_EEString(Checksum_str);          /* display: Checksum */
        LCD_NextLine_EEString(Error_str);    /* display: error! */
        MilliSleep(2000);                    /* give user some time to read */

        SetAdjustDefaults();                 /* set defaults */
      }
    }
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
  LCD_NextLine_Mode(MODE_KEY);          /* set next line mode */

  /* display RiL and RiH */
  LCD_Clear();
  LCD_EEString_Space(RiLow_str);             /* display: Ri- */
  DisplayValue(NV.RiL, -1, LCD_CHAR_OMEGA);
  LCD_NextLine_EEString_Space(RiHigh_str);   /* display: Ri+ */
  DisplayValue(NV.RiH, -1, LCD_CHAR_OMEGA);

  /* display C-Zero */
  LCD_NextLine_EEString_Space(CapOffset_str);     /* display: C0 */
  DisplayValue(NV.CapZero, -12, 'F');    /* display C0 offset */

  /* display R-Zero */
  LCD_NextLine_EEString_Space(ROffset_str);        /* display: R0 */
  DisplayValue(NV.RZero, -2, LCD_CHAR_OMEGA);  /* display R0 */

  /* display internal bandgap reference */
  LCD_NextLine_EEString_Space(URef_str);     /* display: Vref */
  DisplayValue(Config.Bandgap, -3, 'V');     /* display bandgap ref */

  /* display Vcc */
  LCD_NextLine_EEString_Space(Vcc_str);      /* display: Vcc */
  DisplayValue(Config.Vcc, -3, 'V');         /* display Vcc */

  /* display offset of analog comparator */
  LCD_NextLine_EEString_Space(CompOffset_str);    /* display: AComp */
  DisplaySignedValue(NV.CompOffset, -3, 'V');

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
  if (Counter == 0) Test = 10;     /* skip adjustment on error */

  while (Test <= 5)      /* loop through tests */
  {
    Counter = 1;

    /* repeat each measurement 5 times */
    while (Counter <= 5)
    {
      /* display test number */
      LCD_Clear();
      LCD_Char('A');                    /* display: a */
      LCD_Char('0' + Test);             /* display number */
      LCD_Space();

      DisplayFlag = 1;        /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* resistance of probe leads (probes shorted) */
          LCD_EEString_Space(ROffset_str);   /* display: R0 */
          LCD_EEString(ProbeComb_str);       /* display: 12 13 23 */          

          /*
           *  The resistance is for two probes in series and we expect it to be
           *  lower than 1.00 Ohms, i.e. 0.50 Ohms for a single probe.
           */

          UpdateProbes(PROBE_2, PROBE_1, 0);
          Val1 = SmallResistor(0);
          if (Val1 < 100)                    /* within limit */
          {
            RSum += Val1;
            RCounter++;
          }

          UpdateProbes(PROBE_3, PROBE_1, 0);
          Val2 = SmallResistor(0);
          if (Val2 < 100)                    /* whithin limit */
          {
            RSum += Val2;
            RCounter++;
          }

          UpdateProbes(PROBE_3, PROBE_2, 0);
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

          /* TP1:  Gnd -- RiL -- probe-1 -- Rl -- RiH -- Vcc */
          ADC_PORT = 0;
          ADC_DDR = (1 << TP1);
          R_PORT = (1 << R_RL_1);
          R_DDR = (1 << R_RL_1);
          Val1 = ReadU_5ms(TP1);        /* U across RiL */
          U_RiL += Val1;

          /* TP2: Gnd -- RiL -- probe-2 -- Rl -- RiH -- Vcc */
          ADC_DDR = (1 << TP2);
          R_PORT =  (1 << R_RL_2);
          R_DDR = (1 << R_RL_2);
          Val2 = ReadU_5ms(TP2);       /* U across RiL */
          U_RiL += Val2;

          /* TP3: Gnd -- RiL -- probe-3 -- Rl -- RiH -- Vcc */
          ADC_DDR = (1 << TP3);
          R_PORT =  (1 << R_RL_3);
          R_DDR = (1 << R_RL_3);
          Val3 = ReadU_5ms(TP3);       /* U across RiL */
          U_RiL += Val3;

          RiL_Counter += 3;
          break;

        case 4:     /* internal resistance of MCU in pull-up mode */
          LCD_EEString(RiHigh_str);     /* display: Ri+ */

          /* TP1: Gnd -- RiL -- Rl -- probe-1 -- RiH -- Vcc */
          R_PORT = 0;
          ADC_PORT = (1 << TP1);
          ADC_DDR = (1 << TP1);
          R_DDR = (1 << R_RL_1);
          Val1 = Config.Vcc - ReadU_5ms(TP1);     /* U across RiH */
          U_RiH += Val1;

          /* TP2: Gnd -- RiL -- Rl -- probe-2 -- RiH -- Vcc */
          ADC_PORT = (1 << TP2);
          ADC_DDR = (1 << TP2);
          R_DDR = (1 << R_RL_2);
          Val2 = Config.Vcc - ReadU_5ms(TP2);     /* U across RiH */
          U_RiH += Val2;

          /* TP3: Gnd -- RiL -- Rl -- probe-3 -- RiH -- Vcc */
          ADC_PORT = (1 << TP3);
          ADC_DDR = (1 << TP3);
          R_DDR = (1 << R_RL_3);
          Val3 = Config.Vcc - ReadU_5ms(TP3);     /* U across RiH */
          U_RiH += Val3;

          RiH_Counter += 3;
          break;

        case 5:     /* capacitance offset (PCB and probe leads) */
          LCD_EEString_Space(CapOffset_str);   /* display: C0 */
          LCD_EEString(ProbeComb_str);         /* display: 12 13 23 */

          /*
           *  The capacitance is for two probes and we expect it to be
           *  less than 100pF.
           */

          MeasureCap(PROBE_2, PROBE_1, 0);
          Val1 = (uint16_t)Caps[0].Raw;
          /* limit offset to 100pF */
          if ((Caps[0].Scale == -12) && (Caps[0].Raw <= 100))
          {
            CapSum += Val1;
            CapCounter++;            
          }

          MeasureCap(PROBE_3, PROBE_1, 1);
          Val2 = (uint16_t)Caps[1].Raw;
          /* limit offset to 100pF */
          if ((Caps[1].Scale == -12) && (Caps[1].Raw <= 100))
          {
            CapSum += Val2;
            CapCounter++;            
          }

          MeasureCap(PROBE_3, PROBE_2, 2);
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
        LCD_NextLine();                 /* move to line #2 */
        DisplayValue(Val1, 0 , 0);      /* display value probe-1 */
        LCD_Space();
        DisplayValue(Val2, 0 , 0);      /* display value probe-2 */
        LCD_Space();
        DisplayValue(Val3, 0 , 0);      /* display value probe-3 */
      }

      /* wait and check test push button */
      if (Counter < 100)                     /* when we don't skip this test */
      {
        DisplayFlag = TestKey(1000, 0);      /* catch key press or timeout */

        /* short press -> next test / long press -> end selftest */
        if (DisplayFlag > KEY_TIMEOUT)
        {
          Counter = 100;                       /* skip current test anyway */
          if (DisplayFlag == KEY_LONG) Test = 100;  /* also skip selftest */
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
    NV.CapZero = CapSum / CapCounter;
    Flag++;
  }

  /* resistance auto-zero: calculate average value for all probes pairs */
  if (RCounter == 15)
  { 
    /* calculate average offset (0.01 Ohms) */
    NV.RZero = RSum / RCounter;
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
      NV.RiL = (uint16_t)Val0;
      Flag++;
    }

    /* RiH */
    Val0 = ((uint32_t)R_LOW * 100 * U_RiH) / Val1;     /* Rl * U_Ri / U_Rl in 0.01 Ohm */
    Val0 += 5;                                         /* for automagic rounding */
    Val0 /= 10;                                        /* scale down to 0.1 Ohm */
    if (Val0 < 280UL)         /* < 29 Ohms */
    {
      NV.RiH = (uint16_t)Val0;
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
  int16_t           Temp;               /* value */

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
      LCD_Char('T');                    /* display: T */
      LCD_Char('0' + Test);             /* display test number */
      LCD_Space();

      DisplayFlag = 1;                  /* display values by default */

      /*
       *  tests
       */

      switch (Test)
      {
        case 1:     /* reference voltage */
          Val0 = ReadU(ADC_BANDGAP);    /* dummy read for bandgap stabilization */
          Val0 = ReadU(ADC_BANDGAP);    /* read bandgap reference voltage */ 
          LCD_EEString(URef_str);       /* display: Vref */

          LCD_NextLine();
          DisplayValue(Val0, -3, 'V');       /* display voltage in mV */

          DisplayFlag = 0;                   /* reset flag */
          break;

        case 2:     /* compare Rl resistors (probes still shorted) */
          LCD_EEString_Space(Rl_str);     /* display: +Rl- */
          LCD_EEString(ProbeComb_str);    /* display: 12 13 23 */

          /* set up a voltage divider with the Rl's */
          /* substract theoretical voltage of voltage divider */

          /* voltage of voltage divider */
          Temp = ((int32_t)Config.Vcc * (R_MCU_LOW + R_LOW)) / (R_MCU_LOW + R_LOW + R_LOW + R_MCU_HIGH);

          /* TP3: Gnd -- Rl -- probe-2 -- probe-1 -- Rl -- Vcc */
          R_PORT = (1 << R_RL_1);
          R_DDR = (1 << R_RL_1) | (1 << R_RL_2);
          Val3 = ReadU_20ms(TP3);
          Val3 -= Temp;

          /* TP2: Gnd -- Rl -- probe-3 -- probe-1 -- Rl -- Vcc */
          R_DDR = (1 << R_RL_1) | (1 << R_RL_3);
          Val2 = ReadU_20ms(TP2);
          Val2 -= Temp;

          /* TP1: Gnd -- Rl -- probe-3 -- probe-2 -- Rl -- Vcc */
          R_PORT = (1 << R_RL_2);
          R_DDR = (1 << R_RL_2) | (1 << R_RL_3);
          Val1 = ReadU_20ms(TP1);
          Val1 -= Temp;

          break;

        case 3:     /* compare Rh resistors (probes still shorted) */
          LCD_EEString_Space(Rh_str);     /* display: +Rh- */
          LCD_EEString(ProbeComb_str);    /* display: 12 13 23 */

          /* set up a voltage divider with the Rh's */

          /* voltage of voltage divider (ignore RiL and RiH) */
          Temp = Config.Vcc / 2;

          /* TP3: Gnd -- Rh -- probe-2 -- probe-1 -- Rh -- Vcc */
          R_PORT = (1 << R_RH_1);
          R_DDR = (1 << R_RH_1) | (1 << R_RH_2);
          Val3 = ReadU_20ms(TP3);
          Val3 -= Temp;

          /* TP2: Gnd -- Rh -- probe-3 -- probe-1 -- Rh -- Vcc */
          R_DDR = (1 << R_RH_1) | (1 << R_RH_3);
          Val2 = ReadU_20ms(TP2);
          Val2 -= Temp;

          /* TP1: Gnd -- Rh -- probe-3 -- probe-2 -- Rh -- Vcc */
          R_PORT = (1 << R_RH_2);
          R_DDR = (1 << R_RH_2) | (1 << R_RH_3);
          Val1 = ReadU_20ms(TP1);
          Val1 -= Temp;

          break;

        case 4:     /* un-short probes */
          ShortCircuit(0);         /* make sure probes are not shorted */
          Counter = 100;           /* skip test */
          DisplayFlag = 0;         /* reset flag */
          break;

        case 5:     /* Rh resistors pulled down */
          LCD_EEString(RhLow_str);      /* display: Rh- */

          /* TP1: Gnd -- Rh -- probe-1 */
          R_PORT = 0;
          R_DDR = (1 << R_RH_1);
          Val1 = ReadU_20ms(TP1);

          /* TP2: Gnd -- Rh -- probe-2 */
          R_DDR = (1 << R_RH_2);
          Val2 = ReadU_20ms(TP2);

          /* TP3: Gnd -- Rh -- probe-3 */
          R_DDR = (1 << R_RH_3);
          Val3 = ReadU_20ms(TP3);

          break;

        case 6:     /* Rh resistors pulled up */
          LCD_EEString(RhHigh_str);     /* display: Rh+ */

          /* TP1: probe-1 -- Rh -- Vcc */
          R_DDR = (1 << R_RH_1);
          R_PORT = (1 << R_RH_1);
          Val1 = ReadU_20ms(TP1);

          /* TP2: probe-2 -- Rh -- Vcc */
          R_DDR = (1 << R_RH_2);
          R_PORT = (1 << R_RH_2);
          Val2 = ReadU_20ms(TP2);

          /* TP3: probe-3 -- Rh -- Vcc */
          R_DDR = (1 << R_RH_3);
          R_PORT = (1 << R_RH_3);
          Val3 = ReadU_20ms(TP3);

          break;
      }

      /* reset ports to defaults */
      R_DDR = 0;                             /* input mode */
      R_PORT = 0;                            /* all pins low */

      /* display voltages/values of all probes */
      if (DisplayFlag)
      {
        LCD_NextLine();                      /* move to line #2 */
        DisplaySignedValue(Val1, 0 , 0);     /* display value probe-1 */
        LCD_Space();
        DisplaySignedValue(Val2, 0 , 0);     /* display value probe-2 */
        LCD_Space();
        DisplaySignedValue(Val3, 0 , 0);     /* display value probe-3 */
      }

      /* wait and check test push button */
      if (Counter < 100)                     /* when we don't skip this test */
      {
        DisplayFlag = TestKey(1000, 0);      /* catch key press or timeout */

        /* short press -> next test / long press -> end selftest */
        if (DisplayFlag > KEY_TIMEOUT)
        {
          Counter = 100;                       /* skip current test anyway */
          if (DisplayFlag == KEY_LONG) Test = 100;  /* also skip selftest */
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

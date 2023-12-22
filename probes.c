/* ************************************************************************
 *
 *   probing testpins
 *
 *   (c) 2012-2022 by Markus Reschke
 *   based on code from Markus Frejek and Karl-Heinz Kübbeler
 *
 * ************************************************************************ */


/*
 *  local constants
 */

/* source management */
#define PROBES_C


/*
 *  include header files
 */

/* local includes */
#include "config.h"           /* global configuration */
#include "common.h"           /* common header file */
#include "variables.h"        /* global variables */
#include "functions.h"        /* external functions */



/* ************************************************************************
 *   probe management
 * ************************************************************************ */


/*
 *  set up probes, register bits for probes and test resistors
 *
 *  requires:
 *  - Probe1: pin ID [0-2], mostly high level pin
 *  - Probe2: pin ID [0-2], mostly low level pin
 *  - Probe3: pin ID [0-2], mostly switch/gate pin
 */

void UpdateProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3)
{
  /* set probe IDs */
  Probes.ID_1 = Probe1;
  Probes.ID_2 = Probe2;
  Probes.ID_3 = Probe3;

  /* set register bits for probe resistors based on ID */
  Probes.Rl_1 = DATA_read_byte(&Rl_table[Probe1]);
  Probes.Rl_2 = DATA_read_byte(&Rl_table[Probe2]);
  Probes.Rl_3 = DATA_read_byte(&Rl_table[Probe3]);
  Probes.Rh_1 = DATA_read_byte(&Rh_table[Probe1]);
  Probes.Rh_2 = DATA_read_byte(&Rh_table[Probe2]);
  Probes.Rh_3 = DATA_read_byte(&Rh_table[Probe3]);

  /* set register bits for ADC port pins based on ID */ 
  Probes.Pin_1 = DATA_read_byte(&Pin_table[Probe1]);
  Probes.Pin_2 = DATA_read_byte(&Pin_table[Probe2]);
  Probes.Pin_3 = DATA_read_byte(&Pin_table[Probe3]);

  /* set register bits for ADC MUX input channels based on ID */
  Probes.Ch_1 = DATA_read_byte(&Channel_table[Probe1]);
  Probes.Ch_2 = DATA_read_byte(&Channel_table[Probe2]);
  Probes.Ch_3 = DATA_read_byte(&Channel_table[Probe3]);
}



/*
 *  convenience function for UpdateProbes()
 *  - called with just two probe IDs
 *  - third probe ID is derived from the other two
 *
 *  requires:
 *  - Probe1: ID of first probe (0-2)
 *  - Probe2: ID of second probe (0-2)
 */

void UpdateProbes2(uint8_t Probe1, uint8_t Probe2)
{
  uint8_t           Probe3;             /* ID of third probe */

  /*
   *  derive ID of third probe
   *  - use sum of probe IDs (order doesn't matter)
   *    probes #1 & #2: 0 + 1 = 1  -> probe #3: 2
   *    probes #1 & #3: 0 + 2 = 2  -> probe #2: 1
   *    probes #2 & #3: 1 + 2 = 3  -> probe #1: 0
   *  - third probe = 3 - (probe ID #1 + probe ID #2)
   */

  Probe3 = 3;                 /* = 3 */
  Probe3 -= Probe1;           /* - ID #1 */
  Probe3 -= Probe2;           /* - ID #2 */

  /* and now update probes */
  UpdateProbes(Probe1, Probe2, Probe3);
}



/*
 *  restore original probe IDs
 */

void RestoreProbes(void)
{
  /* call probe update for saved IDs */
  UpdateProbes(Probes.ID2_1, Probes.ID2_2, Probes.ID2_3);
}



/*
 *  backup current probe IDs
 */

void BackupProbes(void)
{
  /* copy current probe IDs to backup set */
  Probes.ID2_1 = Probes.ID_1;
  Probes.ID2_2 = Probes.ID_2;
  Probes.ID2_3 = Probes.ID_3;
}



/*
 *  check for a short circuit between two probes
 *  - changes probe settings
 *
 *  requires:
 *  - ID of first probe (0-2)
 *  - ID of second probe (0-2)
 *
 *  returns:
 *  - 0 if not shorted
 *  - 1 if shorted
 */

uint8_t ShortedPair(uint8_t Probe1, uint8_t Probe2)
{
  uint8_t           Flag = 0;      /* return value */
  uint16_t          U1;            /* voltage at probe #1 in mV */
  uint16_t          U2;            /* voltage at probe #2 in mV */
  uint16_t          Min;           /* lower threshold */
  uint16_t          Max;           /* upper threshold */

  UpdateProbes2(Probe1, Probe2);        /* update probes */

  /*
   *  Set up a voltage divider between the two probes:
   *  - Gnd -- Rl -- probe-2 / probe-1 -- Rl -- Vcc
   *  - third probe: HiZ
   */

  ADC_DDR = 0;                          /* set ADC port to HiZ */
  R_PORT = Probes.Rl_1;                 /* pull up probe-1 via Rl */
  R_DDR = Probes.Rl_1 | Probes.Rl_2;    /* and pull down probe-2 via Rl */

  /* read voltages */
  U1 = ReadU_5ms(Probes.Ch_1);
  U2 = ReadU(Probes.Ch_2);

  /*
   *  We expect both probe voltages to be about the same and
   *  to be half of Vcc (allowed difference +/- 30mV).
   */

  Min = (Cfg.Vcc / 2) - 30;        /* lower voltage */
  Max = (Cfg.Vcc / 2) + 30;        /* upper voltage */

  if ((U1 > Min) && (U1 < Max))    /* U1 within window */
  { 
    if ((U2 > Min) && (U2 < Max))  /* U2 within window */
    {
      Flag = 1;                    /* about the same */
    }    
  }

  R_DDR = 0;             /* reset port */

  return Flag;
}



/*
 *  check for a short circuit between all probes
 *
 *  returns:
 *  - number of short-circuited probe pairs
 *    (0 = none, 3 = all)
 */

uint8_t ShortedProbes(void)
{
  uint8_t           Flag = 0;      /* return value */

  /* check all possible combinations */
  Flag = ShortedPair(PROBE_1, PROBE_2);
  Flag += ShortedPair(PROBE_1, PROBE_3);
  Flag += ShortedPair(PROBE_2, PROBE_3);

  return Flag;  
}



#if defined (SW_ESR) || defined (SW_OLD_ESR)

/*
 *  discharge cap connected to two probes
 *
 *  requires:
 *  - Probe1: ID of probe-1
 *  - Probe2: ID of probe-2
 */

void DischargeCap(uint8_t Probe1, uint8_t Probe2)
{
  uint8_t           n;             /* counter */
  uint16_t          U_1;           /* voltage #1 */
  uint16_t          U_2;           /* voltage #2 */

  /* probes: set to safe mode */
  ADC_DDR = 0;
  ADC_PORT = 0;
  R_DDR = 0;
  R_PORT = 0;


  /*
   *  figure out the positive charged pin
   */

  UpdateProbes2(Probe1, Probe2);        /* update probes */

  /* try probe-1 */ 
  ADC_DDR = Probes.Pin_1;          /* pull down probe-1 directly */
  U_1 = ReadU(Probes.Ch_2);        /* get voltage at probe-2 */

  /* try probe-2 */
  ADC_DDR = Probes.Pin_2;          /* pull down probe-2 directly */
  U_2 = ReadU(Probes.Ch_1);        /* get voltage at probe-1 */

  if (U_2 > U_1)                   /* probe-1 is positive */
  {
    /* reverse probes */
    UpdateProbes2(Probe2, Probe1);      /* update probes */
  } 


  /*
   *  discharge cap to voltage below 40mV
   */

  /* discharge positive side via Rl */
  ADC_DDR = Probes.Pin_1;          /* pull down probe-1 directly */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */

  n = 1;

  while (n)                        /* processing loop */
  {
    U_1 = ReadU(Probes.Ch_2);      /* get voltage at probe-2 */

    if (U_1 < 400)                 /* below 400mV */
    {
      ADC_DDR |= Probes.Pin_2;     /* also pull down probe-2 directly */
    }

    if (U_1 < 40)                  /* discharged < 40mV */
    {
      n = 0;                       /* end loop */
    }
    else                           /* not discharged yet */
    {
      n++;                         /* another cycle */

      if (n > 50)                  /* timeout (5s) */
      {
        n = 0;                     /* end loop */
      }
      else                         /* keep discharging */
      {
        MilliSleep(100);           /* wait */
      }
    }
  }

  /* probes: reset to safe mode */
  ADC_DDR = 0;
  R_DDR = 0;
}

#endif



/*
 *  try to discharge any connected components, e.g. capacitors
 *  - detect batteries
 *  - sometimes large caps are detected as a battery
 */

void DischargeProbes(void)
{
  uint8_t           Counter;            /* loop control */
  uint8_t           Limit = 40;         /* sliding timeout (2s) */
  uint8_t           ID;                 /* test pin */
  uint8_t           Flags;              /* discharge state flags */
  uint8_t           Channel;            /* ADC MUX channel */
  uint16_t          U_c;                /* current voltage */
  uint16_t          U_old[3];           /* old voltages */


  /*
   *  set probes to a safe discharge mode (pull-down via Rh) 
   */

  /* set ADC port to HiZ input */
  ADC_DDR = 0;
  ADC_PORT = 0;

  /* all probe pins: Rh and Rl pull-down */
  R_PORT = 0;
  R_DDR = (1 << R_RH_1) | (1 << R_RH_2) | (1 << R_RH_3) |
          (1 << R_RL_1) | (1 << R_RL_2) | (1 << R_RL_3);

  /* get current voltages */
  U_old[0] = ReadU(TP1);
  U_old[1] = ReadU(TP2);
  U_old[2] = ReadU(TP3);

  /*
   *  try to discharge probes
   *  - We check if the voltage decreases over time.
   *  - A slow discharge rate will increase the timeout to support
   *    large caps.
   *  - A very large cap will discharge too slowly and an external voltage
   *    maybe never :)
   *  - The voltage measured is the voltage across the probe resistors Rh and
   *    Rl in parallel referenced to Gnd. At the same time the DUT's other side
   *    is also connected via its probe resistors to Gnd. In case of a battery
   *    it's not the battery's voltage. Therefore we measure the unloaded
   *    voltage later on when we encounter a discharge problem.
   *  - The protection relay option comes in two variants, i.e. probes shorted
   *    directly or via resistors. So we dont't use it here to keep things
   *    simple.
   */

  /* reset variables */
  Counter = 1;                          /* enter loop */
  ID = 2;                               /* probe #3 */
  Flags = 0;                            /* no probe discharged */

  /* processing loop */
  while (Counter > 0)
  {
    ID++;                               /* next probe */
    if (ID > 2) ID = 0;                 /* start with probe #1 again */

    if (Flags & (1 << ID))              /* skip discharged probe */
      continue;

    /* get voltage at probe */
    Channel = DATA_read_byte(&Channel_table[ID]);    /* update ADC channel */
    U_c = ReadU(Channel);                            /* get voltage */

    if (U_c < U_old[ID])                /* voltage decreased */
    {
      U_old[ID] = U_c;                  /* update old value */

      /* adapt timeout based on discharge rate */
      if ((Limit - Counter) < 20)
      {
        /* increase timeout while preventing overflow */
        if (Limit < (255 - 20)) Limit += 20;
      }

      Counter = 1;                      /* reset no-changes counter */
    }
    else                                /* voltage not decreased */
    {
      /* increase limit if we start at a low voltage */
      if ((U_c < 10) && (Limit <= 40)) Limit = 80;

      Counter++;                        /* increase no-changes counter */
    }

    if (U_c <= CAP_DISCHARGED)          /* seems to be discharged */
    {
      Flags |= (1 << ID);               /* set flag for probe */
    }
    else if (U_c < 800)                 /* extra pull-down */
    {
      /* it's safe now to pull down probe pin directly */
      ADC_DDR |= DATA_read_byte(&Pin_table[ID]);
    }

    if (Flags == 0b00000111)            /* all probes discharged */
    {
      Counter = 0;                      /* end loop */
    }
    else if (Counter > Limit)           /* no decrease for some time */
    {
      /* might be a battery or a super cap */
      Check.Found = COMP_ERROR;         /* report error */
      Check.Type = TYPE_DISCHARGE;      /* discharge problem */
      Check.Probe = ID;                 /* save probe */

      /* measure unloaded voltage */
      Flags = DATA_read_byte(&Pin_table[ID]);
      ADC_DDR &= ~Flags;                /* remove direct pull-down */
      Flags = DATA_read_byte(&Rh_table[ID]) | DATA_read_byte(&Rl_table[ID]);
      R_DDR &= ~Flags;                  /* disable load resistors */
      Check.U = ReadU(Channel);         /* get and save voltage */

      Counter = 0;                      /* end loop */
    }
    else                                /* go for another round */
    {
      wdt_reset();                      /* reset watchdog */
      MilliSleep(50);                   /* wait for 50ms */
    }
  }

  /* reset probes */
  R_DDR = 0;                  /* set resistor port to input mode */
  ADC_DDR = 0;                /* set ADC port to input mode */
}



/*
 *  pull probe up/down via probe resistor for 1 or 10 ms
 *
 *  requires:
 *  - mask for probe resistors
 *  - pull mode (bit flags):
 *    PULL_DOWN  pull down
 *    PULL_UP    pull up 
 *    PULL_1MS   for 1ms
 *    PULL_10MS  for 10ms
 */

void PullProbe(uint8_t Mask, uint8_t Mode)
{
  /* set pull mode */
  if (Mode & PULL_UP)         /* pull-up */
  {
    R_PORT |= Mask;                /* set bit */
  }
  else                        /* pull-down */
  {
    R_PORT &= ~Mask;               /* clear bit */
  }
  R_DDR |= Mask;              /* enable pulling */

  if (Mode & PULL_1MS)        /* wait 1ms */
  {
    wait1ms();
  }
  else                        /* wait 10ms */
  {
    wait10ms();
  }

  /* reset pulling */
  R_DDR &= ~Mask;             /* set to HiZ mode */
  R_PORT &= ~Mask;            /* set 0 */
}



/* ************************************************************************
 *   calculation support
 * ************************************************************************ */


/*
 *  lookup a voltage/ratio based factor in a table and interpolate it's value
 *  - value decreases with index position
 *
 *  requires:
 *  - voltage (in mV) or ratio
 *  - table ID
 *
 *  returns:
 *  - multiplicator/factor
 */

uint16_t GetFactor(uint16_t U_in, uint8_t ID)
{
  uint16_t          Factor;             /* return value */
  uint16_t          U_Diff;             /* voltage difference to table start */
  uint16_t          Fact1, Fact2;       /* table entries */
  uint16_t          TabStart;           /* table start voltage */
  uint16_t          TabStep;            /* table step voltage */
  uint16_t          TabIndex;           /* table entries (-2) */
  uint16_t          *Table;             /* pointer to table */
  uint8_t           Index;              /* table index */
  uint8_t           Diff;               /* difference to next entry */

  /*
   *  set up table specific stuff
   */

  if (ID == TABLE_SMALL_CAP)
  {
    TabStart = 1000;                         /* table starts at 1000mV */
    TabStep = 50;                            /* 50mV steps between entries */
    TabIndex = (NUM_SMALL_CAP - 2);          /* entries in table - 2 */
    Table = (uint16_t *)&SmallCap_table[0];  /* pointer to table */
  }
  else if (ID == TABLE_LARGE_CAP)
  {
    TabStart = 300;                          /* table starts at 1000mV */
    TabStep = 25;                            /* 25mV steps between entries */
    TabIndex = (NUM_LARGE_CAP - 2);          /* entries in table - 2 */
    Table = (uint16_t *)&LargeCap_table[0];  /* pointer to table */
  }
  #ifdef SW_INDUCTOR
  else if (ID == TABLE_INDUCTOR)
  {
    TabStart = 200;                          /* table starts at 200 */
    TabStep = 25;                            /* steps between entries */
    TabIndex = (NUM_INDUCTOR - 2);           /* entries in table - 2 */
    Table = (uint16_t *)&Inductor_table[0];  /* pointer to table */
  }
  #endif
  else
  {
    return 0;                 /* signal error */
  }

  /*
   *  We interpolate the table values corresponding to the given voltage/ratio.
   */

  /* difference to start of table */
  if (U_in >= TabStart) U_Diff = U_in - TabStart;  
  else U_Diff = 0;

  /* calculate table index */
  Index = U_Diff / TabStep;             /* index (position in table) */
  Diff = U_Diff % TabStep;              /* difference to index */
  Diff = TabStep - Diff;                /* difference to next entry */

  /* prevent index overflow */
  if (Index > TabIndex) Index = TabIndex;

  /* get values for index and next entry */
  Table += Index;                       /* advance to index */
  Fact1 = DATA_read_word(Table);
  Table++;                              /* next entry */
  Fact2 = DATA_read_word(Table);

  /* interpolate values based on the difference */
  Factor = Fact1 - Fact2;
  Factor *= Diff;
  Factor += TabStep / 2;
  Factor /= TabStep;
  Factor += Fact2;

  return Factor;
}



#if defined (FUNC_EVALUE) || defined (FUNC_COLORCODE) || defined (FUNC_EIA96)

/*
 *  get E series norm value(s)
 *  - sets:
 *    first value   second value   description
 *    --------------------------------------------------------
 *    Semi.I_value  Semi.C_value   norm value (10-99, 100-999)
 *    Semi.I_scale  Semi.C_scale   multiplicator (10^n)
 *    [Semi.A       Semi.B         index number (1-)] 
 *  - range 10-99 for E series <= E24
 *    range 100-999 for E series >= E48
 *
 *  requires:
 *  - Value: unsigned value
 *  - Scale: exponent/multiplier (* 10^n)
 *  - E_Series: E6 - E96
 *  - Tolerance: tolerance (in 0.1%)
 *
 *  returns:
 *  - 0 on any error or no matching norm values
 *  - 1 for one matching norm value
 *  - 2 for two matching norm values
 */

uint8_t GetENormValue(uint32_t Value, int8_t Scale, uint8_t E_Series, uint8_t Tolerance)
{
  uint8_t           Flag = 0;           /* return values */
  uint16_t          *Table;             /* pointer to table */
  uint8_t           Index;              /* table index */
  uint8_t           n;                  /* counter */
  uint16_t          Norm;               /* norm value */
  uint16_t          LowVal = 0;         /* lower norm value */
  uint16_t          HighVal = 0;        /* higher norm value */
  int8_t            HighScale;          /* multiplier for higher norm value */
  uint32_t          Value2;             /* normalized value */
  uint32_t          Offset;             /* offset */
  #ifdef FUNC_EIA96
  uint8_t           LowIndex = 0;       /* index of lower norm value */
  uint8_t           HighIndex = 0;      /* index of higher norm value */
  #endif

  /*
   *  E series implies maximum tolerance (may be tighter)
   *  E series:   E3   E6   E12   E24   E48   E96   E192
   *  tolerance:  40%  20%  10%   5%    2%    1%    0.5%
   *
   *  E series <= E24 rounded to 1 trailing digit
   *  E series >= E48 rounded to 2 trailing digits
   */


  /*
   *  set up table specific stuff
   *  - the norm values stored in the tables are scaled to 0.01
   */

  switch (E_Series)
  {
    #ifdef SW_E6
    case E6:                                /* E6 */
      Table = (uint16_t *)&E6_table[0];     /* pointer to table */
      Index = NUM_E6;                       /* 6 values */
      break;
    #endif

    #ifdef SW_E12
    case E12:                                /* E12 */
      Table = (uint16_t *)&E12_table[0];     /* pointer to table */
      Index = NUM_E12;                       /* 12 values */
      break;
    #endif

    #ifdef SW_E24
    case E24:                                /* E24 */
      Table = (uint16_t *)&E24_table[0];     /* pointer to table */
      Index = NUM_E24;                       /* 24 values */
      break;
    #endif

    #ifdef SW_E96
    case E96:                                /* E96 */
      Table = (uint16_t *)&E96_table[0];     /* pointer to table */
      Index = NUM_E96;                       /* 96 values */
      break;
    #endif

    default:                                 /* no matching E series */
      return Flag;                           /* signal error */
      break;
  }


  /*
   *  normalize component value: 10000 - 99999
   *  - for checking tolerance (2 decimal places)
   */

  while (Value >= 100000)     /* upper limit */
  {
    /* todo: round? */
    Value /= 10;              /* /10 */
    Scale++;                  /* +1 */
  }

  while (Value < 10000)       /* lower limit */
  {
    Value *= 10;              /* *10 */
    Scale--;                  /* decrease multiplier */
  }

  Value2 = Value;             /* save normalized value */


  /*
   *  normalize component value: 100 - 999
   *  - for finding norm values
   */

  while (Value >= 1000)       /* upper limit */
  {
    /* todo: round? */
    Value /= 10;              /* /10 */
    Scale++;                  /* increase multiplier */
  }

  HighScale = Scale;          /* save multiplier */


  /*
   *  get lower and higher norm value from table
   */

  n = 0;                      /* reset counter */

  while (n < Index)           /* loop through table */
  {
    Norm = DATA_read_word(Table);  /* read norm value */

    if (Norm < (uint16_t)Value)    /* norm value lower */
    {
      LowVal = Norm;               /* update lower norm value */
      #ifdef FUNC_EIA96
      LowIndex = n;                /* update index number */
      #endif
    }
    else                           /* norm value higher */
    {
      HighVal = Norm;              /* save higher norm value */
      #ifdef FUNC_EIA96
      HighIndex = n;               /* save index number */
      #endif
      break;                       /* end loop */
    }

    Table++;                       /* next element */
    n++;                           /* next one */
  }

  /* manage table index overflow */
  if (n == Index)                  /* reached last element */
  {
    /* higher norm value is 1000 (100 and multiplier + 1) */
    HighVal = 1000;
    #ifdef FUNC_EIA96
    HighIndex = 0;                 /* reset index number */
    #endif
  }

  #ifdef FUNC_EIA96
  /* adjust index number to start at 1 */
  LowIndex += 1;
  HighIndex += 1;
  #endif


  /*
   *  check for match with lower norm value
   */

  /* calculate top limit */
  Value = (uint32_t)LowVal * 100;  /* lower norm value plus two digits */
  Offset = Value * Tolerance;      /* * tolerance (in 0.1%) */
  Offset /= 1000;                  /* / (1000 * 0.1%) */
  Value += Offset;                 /* add tolerance offset */

  if (Value2 <= Value)             /* within tolerance */
  {
    if (Tolerance >= 50)           /* two digit value (>= 5%) */
    {
      LowVal /= 10;                /* scale to two digits */
      Scale++;                     /* increase multiplier */
    }

    /* save result as first value (misuse Semi) */
    Semi.I_value = LowVal;         /* norm value */
    Semi.I_scale = Scale;          /* multiplier (10^n) */
    #ifdef FUNC_EIA96
    Semi.A = LowIndex;             /* index number */
    #endif

    Flag++;                        /* got a match */
  }


  /*
   *  check for match with higher norm value
   */

  /* calculate bottom limit */
  Value = (uint32_t)HighVal * 100; /* higher norm value plus two digits */
  Offset = Value * Tolerance;      /* * tolerance (in 0.1%) */
  Offset /= 1000;                  /* / (1000 * 0.1%) */
  Value -= Offset;                 /* subtract tolerance offset */

  if (Value2 >= Value)             /* within tolerance */
  {
    /* rescale special case (norm value 1000) */
    if (HighVal == 1000)
    {
      HighVal = 100;               /* /10 */
      HighScale++;                 /* increase multiplier */
    }

    if (Tolerance >= 50)           /* two digit value (>= 5%) */
    {
      HighVal /= 10;               /* scale to two digits */
      HighScale++;                 /* increase multiplier */
    }

    if (Flag == 0)                 /* first match */
    {
      /* save result as first value (misuse Semi) */
      Semi.I_value = HighVal;      /* norm value */
      Semi.I_scale = HighScale;    /* multiplier (10^n) */
      #ifdef FUNC_EIA96
      Semi.A = HighIndex;          /* index number */
      #endif
    }
    else                           /* second match */
    {
      /* save result as second value (misuse Semi) */
      Semi.C_value = HighVal;      /* norm value */
      Semi.C_scale = HighScale;    /* multiplier (10^n) */
      #ifdef FUNC_EIA96
      Semi.B = HighIndex;          /* index number */
      #endif
    }

    Flag++;                        /* got a match */
  }

  return Flag;
}
#endif



/* ************************************************************************
 *   identify component
 * ************************************************************************ */


/*
 *  probe connected component and try to identify it
 *
 *  requires:
 *  - Probe1: ID of probe to be pulled up [0-2]
 *  - Probe2: ID of probe to be pulled down [0-2]
 *  - Probe3: ID of probe to be in HiZ mode [0-2]
 */

void CheckProbes(uint8_t Probe1, uint8_t Probe2, uint8_t Probe3)
{
  uint8_t           Flag;          /* temporary value */
  uint16_t          U_Rl;          /* voltage across Rl (load) */
  uint16_t          U_1;           /* voltage #1 */

  /* init */
  if (Check.Found == COMP_ERROR) return;   /* skip check on any error */
  wdt_reset();                             /* reset watchdog */
  UpdateProbes(Probe1, Probe2, Probe3);    /* update register bits */

  /*
   *  We measure the current from probe 2 to ground with probe 1 pulled up
   *  to 5V and probe 3 in HiZ mode to determine if we got a self-conducting
   *  part, i.e. diode, resistor or depletion-mode FET. Rl is used as current
   *  shunt.
   *
   *  In case of a FET we have to take care about the gate charge based on
   *  the channel type.
   */

  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_PORT = 0;                      /* set resistor port to Gnd */
  R_DDR = Probes.Rl_2;             /* pull down probe-2 via Rl */
  ADC_DDR = Probes.Pin_1;          /* set probe-1 to output */
  ADC_PORT = Probes.Pin_1;         /* pull-up probe-1 directly */

  /*
   *  For a possible n-channel FET we pull down the gate for a few ms.
   *  - assuming: probe-1 = D / probe-2 = S / probe-3 = G
   *
   *  Hint: The pull-down of the gate will trigger a possible PUT.
   */

  PullProbe(Probes.Rl_3, PULL_10MS | PULL_DOWN);  /* discharge gate via Rl */
  U_Rl = ReadU_5ms(Probes.Ch_2);                  /* get voltage at Rl */


  /*
   *  Additional check for Darlington NPN BJT plus EMI issues causing
   *  a high U_Rl. With base pulled down U_Rl should drop down to a few mV.
   *  - get emitter current with base pulled down
   *  - assuming: probe-1 = C / probe-2 = E / probe-3 = B
   */

  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc / Gnd -- Rl -- probe-3 */
  R_DDR = Probes.Rl_2 | Probes.Rl_3;    /* pull down probe-2 via Rl, probe-3 via Rl */
  U_1 = ReadU_5ms(Probes.Ch_2);         /* get voltage at emitter (Rl) */
  /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
  R_DDR = Probes.Rl_2;                  /* pull down probe-2 via Rl */

  if ((U_1 < U_Rl) && (U_1 < 5))        /* < 5mV (base pulled down) */
  {
    U_Rl = U_1;                         /* use U_1 instead */
  }


  /*
   *  If we got conduction we could have a p-channel FET. For any
   *  other part U_Rl will stay the same.
   */

  if (U_Rl >= 977)            /* > 1.4mA */
  {
    /*
     *  For a possible p-channel FET we pull up the gate for a few ms.
     *  - assuming: probe-1 = S / probe-2 = D / probe-3 = G
     */

    PullProbe(Probes.Rl_3, PULL_10MS | PULL_UP);  /* discharge gate via Rl */
    U_Rl = ReadU_5ms(Probes.Ch_2);                /* get voltage at Rl */


    /*
     *  Additional check for Darlington PNP BJT plus EMI issues causing
     *  a high U_Rl. With base pulled up U_Rl should drop down to a few mV.
     *  - get collector current with base pulled up
     *  - assuming: probe-1 = E / probe-2 = C / probe-3 = B
     */

    /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc / probe-3 -- Rl -- Vcc */
    R_PORT = Probes.Rl_3;                 /* pull up probe-3 via Rl */
    R_DDR = Probes.Rl_2 | Probes.Rl_3;    /* pull down probe-2 via Rl */
    U_1 = ReadU_5ms(Probes.Ch_2);         /* get voltage at collector (Rl) */
    /* set probes: Gnd -- Rl -- probe-2 / probe-1 -- Vcc */
    R_PORT = 0;                           /* set resistor port to Gnd */
    R_DDR = Probes.Rl_2;                  /* pull down probe-2 via Rl */

    if ((U_1 < U_Rl) && (U_1 < 5))        /* < 5mV (base pulled up) */
    {
      U_Rl = U_1;             /* use U_1 instead */
    }
  }


  /*
   *  If there's some current we could have a depletion-mode FET
   *  (self-conducting).
   *
   *  Other possibilities:
   *  - diode, resistor
   *  - Germanium BJT with high leakage current
   *  - high hFE Darlington plus long probe leads and/or noisy environment
   */

  if (U_Rl > 15)         /* > 21µA */
  {
    if (Check.Done == DONE_NONE)        /* not sure yet */
    {
      CheckDepletionModeFET(U_Rl);
    }
  }


  /*
   *  If there's only a low conduction (leakage current) between
   *  probe-1 and probe-2 we might have a semiconductor:
   *  - BJT
   *  - enhancement mode FET or IGBT
   *  - Thyristor or Triac
   *  - or a large resistor
   */

  if (U_Rl < 977)         /* load current < 1.4mA (resistance > 3k) */
  {
    /*
     *  check for:
     *  - PNP BJT (common emitter circuit)
     *  - p-channel MOSFET (low side switching circuit) or IGBT
     */

    if (Check.Done == DONE_NONE)        /* not sure yet */
    {
      /*
       *  we assume:
       *  - BJT: probe-1 = E / probe-2 = C / probe-3 = B
       *  - FET: probe-1 = S / probe-2 = D / probe-3 = G
       */

      /* set probes: Gnd -- Rl - probe-2 / probe-1 -- Vcc / probe-3 -- Rl -- Gnd */
      R_DDR = Probes.Rl_2;                /* enable Rl for probe-2 */
      R_PORT = 0;                         /* pull down collector via Rl */
      ADC_DDR = Probes.Pin_1;             /* set probe-1 to output */
      ADC_PORT = Probes.Pin_1;            /* pull up emitter directly */
      wait5ms();
      R_DDR = Probes.Rl_2 | Probes.Rl_3;  /* pull down base via Rl */
      U_1 = ReadU_5ms(Probes.Ch_2);       /* get voltage at collector */

      /*
       *  If DUT is conducting we might have a PNP BJT or p-channel FET.
       */

      if (U_1 > 3422)                   /* detected current > 4.8mA */
      {
        /* distinguish PNP BJT from p-channel MOSFET or IGBT */
        CheckTransistor(TYPE_PNP, U_Rl);
      }
    }


    /*
     *  check for
     *  - NPN BJT (common emitter circuit)
     *  - Thyristor and TRIAC
     *  - n-channel MOSFET (high side switching circuit) or IGBT
     */

    if (Check.Done == DONE_NONE)        /* not sure yet */
    {
      /*
       *  we assume:
       *  - BJT: probe-1 = C / probe-2 = E / probe-3 = B
       *  - FET: probe-1 = D / probe-2 = S / probe-3 = G
       *  - SCR: probe-1 = A / probe-2 = C / probe-3 = G
       *  - TRIAC: probe-1 = MT2 / probe-2 = MT1 / probe-3 = G
       */

      /* set probes: Gnd -- probe-2 / probe-1 -- Rl -- Vcc / probe-3 -- Rl -- Vcc */
      ADC_DDR = Probes.Pin_2;                /* set probe-2 to output mode */
      ADC_PORT = 0;                          /* pull down probe-2 directly */
      R_DDR = Probes.Rl_1 | Probes.Rl_3;     /* select Rl for probe-1 & Rl for probe-3 */
      R_PORT = Probes.Rl_1 | Probes.Rl_3;    /* pull up collector & base via Rl */
      U_1 = ReadU_5ms(Probes.Ch_1);          /* get voltage at collector */

      /*
       *  If DUT is conducting we might have an NPN BJT, something similar or
       *  an n-channel MOSFET.
       */

      if (U_1 < 1600)                   /* detected current > 4.8mA */
      {
        /* first check for Thyristor and TRIAC */
        Flag = CheckThyristorTriac();

        if (Flag == 0)                 /* no Thyristor or TRIAC */
        {
          /* If we've detected a TRIAC in a former run don't check for BJT. */
          if (Check.Found != COMP_TRIAC)
          {
            /* We might have an NPN BJT, an n-channel MOSFET or IGBT. */
            CheckTransistor(TYPE_NPN, U_Rl);
          }
        }
      }
    }

    #ifdef SW_UJT
    /*
     *  check for UJT
     */

    if (Check.Done == DONE_NONE)        /* not sure yet */
    {
      CheckUJT();
    }
    #endif
  }


  /*
   *  If there's conduction between probe-1 and probe-2 we might have a
   *  - diode (conducting)
   *  - small resistor (checked later on)
   */

  else              /* load current > 1.4mA  (resistance < 3k) */
  {
    /*
     *  check for a PUT
     */

    if (Check.Done == DONE_NONE)        /* not sure yet */
    {
      CheckPUT();
    }

    /*
     *  We check for a diode even if we already found a component to get Vf, 
     *  since there could be a body/protection diode of a transistor.
     */

    CheckDiode();
  }


  /*
   *  Check for a resistor (or another one)
   *  - if no other component is found yet
   *  - if we've got a resistor already 
   */

  if ((Check.Found == COMP_NONE) ||
      (Check.Found == COMP_RESISTOR))
  {
    CheckResistor();
  }


  /*
   *  ... otherwise run some final checks.
   */

  else
  {
    /* verify a MOSFET */
    if ((Check.Found == COMP_FET) && (Check.Type & TYPE_MOSFET))
    {
      VerifyMOSFET();
    }
  }


  /*
   *  clean up
   */

  ADC_DDR = 0;           /* set ADC port to HiZ mode */
  ADC_PORT = 0;          /* set ADC port low */
  R_DDR = 0;             /* set resistor port to HiZ mode */
  R_PORT = 0;            /* set resistor port low */
}



/*
 *  logic for alternative components which might be found
 */

void CheckAlternatives(void)
{
  uint8_t           Flag = 0;

  /*
   *  problematic components:
   *  - PNP with B-E resistor and flyback diode passes UJT test once
   *  - UJT might pass NPN test once
   *  - TRIAC passes PUT test twice, but PUT only once
   */

  if (Check.AltFound != COMP_NONE)           /* alternative found */
  {
    if (! (Check.Done & DONE_SEMI))          /* not sure about common transistor */
    {
      /* but sure about alternative or no common transistor found */
      if ((Check.Done & DONE_ALTSEMI) || (Check.Found < COMP_BJT))
      {
        Flag = 1;                       /* choose alternative component */
      }
    }
  }

  if (Flag)         /* take alternative component */
  {
    /* copy some data */
    Check.Found = Check.AltFound;

    Semi.A = AltSemi.A;
    Semi.B = AltSemi.B;
    Semi.C = AltSemi.C;

    #ifdef SW_SYMBOLS
    Check.Symbol = Check.AltSymbol;
    #endif
  }


  /*
   *  add other special checks here
   */
}



/* ************************************************************************
 *   clean-up of local constants
 * ************************************************************************ */


/* source management */
#undef PROBES_C



/* ************************************************************************
 *   EOF
 * ************************************************************************ */
